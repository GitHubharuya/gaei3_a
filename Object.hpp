#pragma once

#include "Slice.hpp"

using PointIdx = unsigned long;
using PointSize = unsigned long;

struct Object {
    struct Point3 {
        double x, y, z;
    };
    std::vector<Object::Point3> points;
    std::vector<std::array<PointIdx, 3>> faces;
    std::vector<Slice> slices;
};

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

