#pragma once

#include "Slice.hpp"

using PointIdx = unsigned long;
using PointSize = unsigned long;

struct Object {
    struct Point3 {
        double x, y, z;
    };
    double LENGTH_PER_TIME = 1;
    std::vector<Object::Point3> points;
    std::vector<std::array<PointIdx, 3>> faces;
    std::vector<Slice> slices;
    bool check_slice_point_size() const ;
    bool from_slices();
    bool make_points_from_slices();
    bool make_faces_from_slices();
};

std::vector<std::array<PointIdx, 3>> rect_face(PointIdx a, PointIdx b, PointIdx c, PointIdx d) {
    return {std::array{a, b, c}, std::array{c, d, a}};
}

bool Object::make_faces_from_slices() {
    int slice_size = slices.size();
    if (slice_size == 0) {
        return false;
    }
    int slice_point_size = slices[0].points.size();
    faces.clear();
    faces.reserve((slice_size-1) * slice_point_size * 2); // 各側面につき三角形2枚
    for (int t = 1; t < slice_size; t++) {
        PointIdx cur_offset = t * slice_point_size;
        PointIdx pre_offset = (t-1) * slice_point_size;
        for (int i = 1; i < slice_point_size; i++) {
            auto face = rect_face(
                 cur_offset + i, cur_offset + i-1, // 今の点
                 pre_offset + i-1, pre_offset + i // 一つ前の点
            );
            faces.insert(faces.end(), face.begin(), face.end());
        }
        auto face = rect_face(
             cur_offset + 0, cur_offset + slice_point_size-1, // 今の点
             pre_offset + slice_point_size-1, pre_offset + 0 // 一つ前の点
        );
        faces.insert(faces.end(), face.begin(), face.end());
    }

    // t = 0 と t = slice_size の面をふさぐ
    // make_begin_and_end_face();

    return true;
}

bool Object::make_points_from_slices() {
    points.clear();
    points.reserve(slices.size());
    double z = 0;
    for (PointSize i = 0; i < slices.size(); i++) {
        for (const auto& p : slices[i].points) {
            points.emplace_back(Object::Point3{
                p.x, p.y, z
            });
        }
        z += LENGTH_PER_TIME;
    }
    return true;
}

bool Object::check_slice_point_size() const {
    PointSize n = slices.size();
    if (n == 0) return false;
    PointSize slice_points_size = slices[0].points.size();
    for (const auto& slice : slices) {
        if (slice.points.size() != slice_points_size) return false;
    }
    return true;
}

bool Object::from_slices() {
    return make_points_from_slices() &&
        make_faces_from_slices();
}

std::istream& operator>>(std::istream& ist, Object& obj) {
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

std::ostream& operator<<(std::ostream& ost, const Object& obj) {
    for (const auto& p : obj.points) {
        ost << "v " << p.x << " " << p.y << " " << p.z << "\n";
    }
    for (const auto& f : obj.faces) {
        // WARNING: 1-based indexing for OBJ format
        ost << "f " << f[0] + 1 << " " << f[1] + 1 << " " << f[2] + 1 << "\n";
    }
    return ost;
}

