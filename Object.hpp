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
};

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

