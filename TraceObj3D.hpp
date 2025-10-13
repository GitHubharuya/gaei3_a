#pragma once

#include "Geom.hpp"
#include "Slice.hpp"
#include "delaunator.hpp"

#include <array>
#include <vector>

using PointIdx = unsigned long;
using PointSize = unsigned long;

struct TraceObj3D {

    // 入力
    std::vector<Slice> slices;

    // 出力
    std::vector<std::array<PointIdx, 3>> faces;
    std::vector<Geom::Point3> points;

    bool make_side_faces();
    bool make_top_bottom_face();
    virtual bool make_points() = 0; // 仮想関数
    bool from_slices();
    bool check_slice_point_size() const ;
    bool make_faces_from_slices();
};

bool TraceObj3D::make_side_faces() {
    PointSize slice_size = slices.size();
    if (slice_size == 0 || !check_slice_point_size()) {
        return false;
    }
    PointSize slice_point_size = slices[0].points.size();
    unsigned long sideface_num = (slice_size-1) * slice_point_size;
    faces.reserve(faces.size() + sideface_num * 4); // 各側面につき三角形4枚
    
    // 四角形を三角形2つにする
    auto rect_face = [&](std::array<PointIdx, 4> idxs) -> std::vector<std::array<PointIdx, 3>> {
        return {
            {idxs[0], idxs[1], idxs[2]},
            {idxs[2], idxs[3], idxs[0]}
        };
    };

    for (PointSize t = 1; t < slice_size; t++) {
        PointIdx cur_offset = t * slice_point_size;
        PointIdx pre_offset = (t-1) * slice_point_size;
        for (PointSize i = 1; i < slice_point_size; i++) {
            auto face = rect_face({
                 cur_offset + i, cur_offset + i-1, // 今の点
                 pre_offset + i-1, pre_offset + i // 一つ前の点
            });
            faces.insert(faces.end(), face.begin(), face.end());
        }
        auto face = rect_face({
             cur_offset + 0, cur_offset + slice_point_size-1, // 今の点
             pre_offset + slice_point_size-1, pre_offset + 0 // 一つ前の点
        });
        faces.insert(faces.end(), face.begin(), face.end());
    }
    return true;
}

bool TraceObj3D::make_top_bottom_face() {
    PointSize slice_size = slices.size();
    if (slice_size == 0) { false; }
    PointSize slice_point_size = slices[0].points.size();

    // 三角形分割
    std::vector<double> begin_face_xy(slice_point_size * 2);
    std::vector<double> end_face_xy(slice_point_size * 2);

    // slices の最初と最後を同じループで参照するためにmove
    auto begin_face_p = std::move(slices[0].points);
    auto end_face_p = std::move(slices[slice_size - 1].points);

    // Delaunator に渡すために { x0,  y0, x1, y1, ... } の vector に格納
    // https://github.com/delfrrr/delaunator-cpp/blob/master/examples/basic.cpp
    for (PointSize i = 0; i < slice_point_size; i++) {
        begin_face_xy[2*i] = begin_face_p[i].x;
        begin_face_xy[2*i+1] = begin_face_p[i].y;
        end_face_xy[2*i] = end_face_p[i].x;
        end_face_xy[2*i+1] = end_face_p[i].y;
    }
    delaunator::Delaunator d1(begin_face_xy);
    delaunator::Delaunator d2(end_face_xy);
    PointIdx end_fece_offset = (slice_size - 1) * slice_point_size;
    decltype(faces) begin_face;
    decltype(faces) end_face;
    begin_face.reserve(d1.triangles.size()/3);
    end_face.reserve(d2.triangles.size()/3);
    for (PointSize i = 0; i < d1.triangles.size(); i+=3) {
        begin_face.push_back({
            d1.triangles[i], d1.triangles[i+1], d1.triangles[i+2]
        });
    }
    for (PointSize i = 0; i < d2.triangles.size(); i+=3) {
        end_face.push_back({
            d2.triangles[i] + end_fece_offset,
            d2.triangles[i+1] + end_fece_offset,
            d2.triangles[i+2] + end_fece_offset
        });
    }

    // check direction
    for (auto& tri : begin_face) {
        // 手前から奥向きにする
        if (Geom::is_ccw(begin_face_p[tri[0]], begin_face_p[tri[1]], begin_face_p[tri[2]])) {
            std::swap(tri[1], tri[2]);
        }
    }
    faces.insert(faces.end(),
        begin_face.begin(), begin_face.end()
    );

    for (auto& tri : end_face) {
        // 奥から手前向きにする
        if (!Geom::is_ccw(end_face_p[tri[0] - end_fece_offset], end_face_p[tri[1] - end_fece_offset], end_face_p[tri[2] - end_fece_offset])) {
            std::swap(tri[1], tri[2]);
        }
    }
    faces.insert(faces.end(),
        end_face.begin(), end_face.end()
    );

    slices[0].points = std::move(begin_face_p);
    slices[slice_size - 1].points = std::move(end_face_p);

    return true;
}

bool TraceObj3D::make_faces_from_slices() {
    faces.clear();
    return make_side_faces() && make_top_bottom_face();
}

bool TraceObj3D::from_slices() {
    return make_points() &&
        make_faces_from_slices();
}

bool TraceObj3D::check_slice_point_size() const {
    PointSize n = slices.size();
    if (n == 0) return false;
    PointSize slice_points_size = slices[0].points.size();
    for (const auto& slice : slices) {
        if (slice.points.size() != slice_points_size) return false;
    }
    return true;
}

std::ostream& operator<<(std::ostream& ost, const TraceObj3D& obj) {
    for (const auto& p : obj.points) {
        ost << "v " << p.x << " " << p.y << " " << p.z << "\n";
    }
    for (const auto& f : obj.faces) {
        // WARNING: 1-based indexing for OBJ format
        ost << "f " << f[0] + 1 << " " << f[1] + 1 << " " << f[2] + 1 << "\n";
    }
    return ost;
}

