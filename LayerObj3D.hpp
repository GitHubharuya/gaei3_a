#pragma once

#include <array>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

#include "Slice.hpp"
#include "delaunator.hpp"
#include "Geom.hpp"

using PointIdx = unsigned long;
using PointSize = unsigned long;

struct LayerObj3D {
    double LENGTH_PER_TIME = 1;
    LayerObj3D(double lpt) : LENGTH_PER_TIME(lpt) {};
    LayerObj3D() {};
    std::vector<Geom::Point3> points;
    std::vector<std::array<PointIdx, 3>> faces;
    std::vector<Slice> slices;
    bool check_slice_point_size() const ;
    bool from_slices();
    bool make_points_from_slices();
    bool make_faces_from_slices();
    bool make_begin_and_end_face();
};

std::vector<std::array<PointIdx, 3>> rect_face(PointIdx a, PointIdx b, PointIdx c, PointIdx d) {
    return {std::array{a, b, c}, std::array{c, d, a}};
}

std::vector<std::array<PointIdx, 3>> rect_face_with_cent(const std::array<PointIdx, 4> pidxs, std::vector<Geom::Point3>& points) {
    double centx = 0, centy = 0, centz = 0;
    for (auto i : pidxs) {
        centx += points[i].x; centy += points[i].y; centz += points[i].z;
    }
    centx /= 4; centy /= 4; centz /= 4;
    points.emplace_back(Geom::Point3{centx, centy, centz});
    PointIdx cidx = points.size() - 1;
    std::vector<std::array<PointIdx, 3>> res;
    res.reserve(4);
    for (int i = 0; i < 4; i++) {
        int nei = i == 3 ? 0 : i + 1;
        res.emplace_back(std::array<PointIdx, 3>{pidxs[i], pidxs[nei], cidx});
    }
    return res;
}

bool LayerObj3D::make_faces_from_slices() {
    PointSize slice_size = slices.size();
    if (slice_size == 0) {
        return false;
    }
    PointSize slice_point_size = slices[0].points.size();
    unsigned long sideface_num = (slice_size-1) * slice_point_size;
    faces.clear();
    faces.reserve(sideface_num * 4); // 各側面につき三角形4枚
    points.reserve(sideface_num + points.size()); // 各側面の重心を追加
    for (PointSize t = 1; t < slice_size; t++) {
        PointIdx cur_offset = t * slice_point_size;
        PointIdx pre_offset = (t-1) * slice_point_size;
        for (PointSize i = 1; i < slice_point_size; i++) {
            auto face = rect_face_with_cent({
                 cur_offset + i, cur_offset + i-1, // 今の点
                 pre_offset + i-1, pre_offset + i // 一つ前の点
            }, points);
            faces.insert(faces.end(), face.begin(), face.end());
        }
        auto face = rect_face_with_cent({
             cur_offset + 0, cur_offset + slice_point_size-1, // 今の点
             pre_offset + slice_point_size-1, pre_offset + 0 // 一つ前の点
        }, points);
        faces.insert(faces.end(), face.begin(), face.end());
    }

    // t = 0 と t = slice_size の面をふさぐ
    make_begin_and_end_face();

    return true;
}

bool LayerObj3D::make_begin_and_end_face() {
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
    slices[0].points = std::move(begin_face_p);
    slices[slice_size - 1].points = std::move(end_face_p);

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
    auto check_ccw = [&](PointIdx a, PointIdx b, PointIdx c) {
        auto p0 = points[a], p1 = points[b], p2 = points[c];
        double A = (p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x);
        return A > 0;
    };

    for (auto& tri : begin_face) {
        if (check_ccw(tri[0], tri[1], tri[2])) {
            // 手前から奥向き
            std::swap(tri[1], tri[2]);
        }
    }
    faces.insert(faces.end(),
        begin_face.begin(), begin_face.end()
    );

    for (auto& tri : end_face) {
        if (!check_ccw(tri[0], tri[1], tri[2])) {
            // 奥から手前向き
            std::swap(tri[1], tri[2]);
        }
    }
    faces.insert(faces.end(),
        end_face.begin(), end_face.end()
    );

    return true;
}

bool LayerObj3D::make_points_from_slices() {
    points.clear();
    points.reserve(slices.size());
    double z = 0;
    for (PointSize i = 0; i < slices.size(); i++) {
        for (const auto& p : slices[i].points) {
            points.emplace_back(Geom::Point3{
                p.x, p.y, z
            });
        }
        z += LENGTH_PER_TIME;
    }
    return true;
}

bool LayerObj3D::check_slice_point_size() const {
    PointSize n = slices.size();
    if (n == 0) return false;
    PointSize slice_points_size = slices[0].points.size();
    for (const auto& slice : slices) {
        if (slice.points.size() != slice_points_size) return false;
    }
    return true;
}

bool LayerObj3D::from_slices() {
    return make_points_from_slices() &&
        make_faces_from_slices();
}

std::istream& operator>>(std::istream& ist, LayerObj3D& obj) {
    Slice slice;
    while (ist >> slice) {
        // slice.sort_by_direction();
        obj.slices.push_back(slice);
    }

    if (ist.fail() && !ist.eof()) {
        return ist;
    }
    if (!obj.check_slice_point_size() || !obj.from_slices()) {
        // スライスからの生成に失敗
        ist.setstate(std::ios_base::failbit);
    };
    return ist;
}

std::ostream& operator<<(std::ostream& ost, const LayerObj3D& obj) {
    for (const auto& p : obj.points) {
        ost << "v " << p.x << " " << p.y << " " << p.z << "\n";
    }
    for (const auto& f : obj.faces) {
        // WARNING: 1-based indexing for OBJ format
        ost << "f " << f[0] + 1 << " " << f[1] + 1 << " " << f[2] + 1 << "\n";
    }
    return ost;
}

