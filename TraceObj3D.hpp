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
    PointSize step_size;
    PointSize point_size_per_step;
    Slice first_slice;
    Slice last_slice;

    // 出力
    std::vector<std::array<PointIdx, 3>> faces;
    std::vector<Geom::Point3> points;

    bool make_side_faces();
    bool make_top_bottom_face();
    virtual bool make_points() = 0; // 仮想関数
    virtual bool from_slices();
    virtual bool make_faces_from_slices();

    void push_front_step(const std::vector<Geom::Point3>& add_points);
    void add_faces(PointIdx s1, PointIdx s2);
    PointIdx front_face_idx = 0;
};

bool TraceObj3D::make_side_faces() {
    if (step_size == 0) {
        return false;
    }
    unsigned long sideface_num = (step_size-1) * point_size_per_step;
    faces.reserve(faces.size() + sideface_num * 2); // 各側面につき三角形2枚
    
    for (PointSize t = 0; t < step_size - 1; t++) {
        add_faces(t, t + 1);
    }
    return true;
}

bool TraceObj3D::make_top_bottom_face() {
    if (step_size == 0) { return false; }

    // 三角形分割
    std::vector<double> begin_face_xy(point_size_per_step * 2);
    std::vector<double> end_face_xy(point_size_per_step * 2);

    // Delaunator に渡すために { x0,  y0, x1, y1, ... } の vector に格納
    // https://github.com/delfrrr/delaunator-cpp/blob/master/examples/basic.cpp
    for (PointSize i = 0; i < point_size_per_step; i++) {
        begin_face_xy[2*i] = first_slice.points[i].x;
        begin_face_xy[2*i+1] = first_slice.points[i].y;
        end_face_xy[2*i] = last_slice.points[i].x;
        end_face_xy[2*i+1] = last_slice.points[i].y;
    }
    delaunator::Delaunator d1(begin_face_xy);
    delaunator::Delaunator d2(end_face_xy);
    PointIdx end_fece_offset = (step_size - 1) * point_size_per_step;
    decltype(faces) begin_face;
    decltype(faces) end_face;
    begin_face.reserve(d1.triangles.size()/3);
    end_face.reserve(d2.triangles.size()/3);
    for (PointSize i = 0; i < d1.triangles.size(); i+=3) {
        begin_face.push_back({
            static_cast<unsigned long>(d1.triangles[i]),
            static_cast<unsigned long>(d1.triangles[i+1]),
            static_cast<unsigned long>(d1.triangles[i+2])
        });
    }
    for (PointSize i = 0; i < d2.triangles.size(); i+=3) {
        end_face.push_back({
            static_cast<unsigned long>(d2.triangles[i] + end_fece_offset),
            static_cast<unsigned long>(d2.triangles[i+1] + end_fece_offset),
            static_cast<unsigned long>(d2.triangles[i+2] + end_fece_offset)
        });
    }

    // check direction
    for (auto& tri : begin_face) {
        // 手前から奥向きにする
        if (Geom::is_ccw(first_slice.points[tri[0]], first_slice.points[tri[1]], first_slice.points[tri[2]])) {
            std::swap(tri[1], tri[2]);
        }
    }
    faces.insert(faces.end(),
        begin_face.begin(), begin_face.end()
    );

    for (auto& tri : end_face) {
        // 奥から手前向きにする
        if (!Geom::is_ccw(last_slice.points[tri[0] - end_fece_offset], last_slice.points[tri[1] - end_fece_offset], last_slice.points[tri[2] - end_fece_offset])) {
            std::swap(tri[1], tri[2]);
        }
    }
    faces.insert(faces.end(),
        end_face.begin(), end_face.end()
    );

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

// s1 より s2 が z座標が大きい
void TraceObj3D::add_faces(PointIdx s1, PointIdx s2) {
    // 四角形を三角形2つにする
    auto rect_face = [&](std::array<PointIdx, 4> idxs) -> std::vector<std::array<PointIdx, 3>> {
        return {
            {idxs[0], idxs[1], idxs[2]},
            {idxs[2], idxs[3], idxs[0]}
        };
    };

    PointIdx s1_p_idx = s1 * point_size_per_step;
    PointIdx s2_p_idx = s2 * point_size_per_step;
    for (PointIdx i = 0; i < point_size_per_step; i++) {
        PointIdx next = (i + 1) % point_size_per_step;
        // auto face = rect_face({s1_p_idx + i, s2_p_idx + i, s2_p_idx + next, s1_p_idx + next});
        auto face = rect_face({
            s1_p_idx + i, s1_p_idx + next,
            s2_p_idx + next, s2_p_idx + i
        });
        faces.insert(faces.end(), face.begin(), face.end());
    }
}

void TraceObj3D::push_front_step(const std::vector<Geom::Point3>& add_points) {
    if (add_points.size() != point_size_per_step) {
        std::cerr << "attempt to push front diffrent size points\n";
        std::exit(1);
    }

    points.insert(points.end(), add_points.begin(), add_points.end());
    add_faces(step_size, front_face_idx);
    front_face_idx = step_size;
    step_size++;
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

