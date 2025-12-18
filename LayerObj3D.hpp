#pragma once

#include <iostream>
#include <vector>
#include "TraceObj3D.hpp"

// Catmull-Rom spline interpolation
inline Geom::Point2 spline_interpolate(const Geom::Point2& p0, const Geom::Point2& p1, const Geom::Point2& p2, const Geom::Point2& p3, double t) {
    double t2 = t * t;
    double t3 = t2 * t;

    double x = 0.5 * ((2 * p1.x) +
                      (-p0.x + p2.x) * t +
                      (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * t2 +
                      (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * t3);

    double y = 0.5 * ((2 * p1.y) +
                      (-p0.y + p2.y) * t +
                      (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * t2 +
                      (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * t3);

    return Geom::Point2{x, y};
}

enum class InterpolationType {
    Linear,
    Spline
};

struct LayerObj3D : public TraceObj3D {
    double TOTAL_SIZE = 100;
    double INTERPOLATE_SIZE = 0;
    InterpolationType interpolation_type = InterpolationType::Spline;
    LayerObj3D(double _total_size) : TOTAL_SIZE(_total_size) {};
    LayerObj3D(double _total_size, int _interpolate_size) : TOTAL_SIZE(_total_size), INTERPOLATE_SIZE(_interpolate_size) {};
    LayerObj3D(double _total_size, int _interpolate_size, InterpolationType _interp_type) : TOTAL_SIZE(_total_size), INTERPOLATE_SIZE(_interpolate_size), interpolation_type(_interp_type) {};
    LayerObj3D() {};
    std::vector<Slice> slices;
    bool make_points() override;
    bool make_points(int interpolate_size, InterpolationType interp_type);
    bool from_slices(int interpolate_size);
    bool check_slice_point_size() const;
};

bool LayerObj3D::make_points() {
    points.clear();
    points.reserve(slices.size() * slices[0].points.size());
    double z = 0;
    double length_per_time = TOTAL_SIZE / slices.size();
    for (PointSize i = 0; i < slices.size(); i++) {
        for (const auto& p : slices[i].points) {
            points.emplace_back(Geom::Point3{
                p.x, p.y, z
            });
        }
        z += length_per_time;
    }
    return true;
}

bool LayerObj3D::make_points(int interpolate_size, InterpolationType interp_type) {
    // 最後の層以外の間で interpolate_size 個の層が追加される
    PointSize exact_slice_size = (slices.size() - 1) * (interpolate_size + 1) + 1;
    PointSize total_point_size = exact_slice_size * slices[0].points.size();
    points.clear();
    points.reserve(total_point_size);
    double length_per_time = TOTAL_SIZE / exact_slice_size;

    double z = 0;
    // 最後の層を除き, 補間しながら点を追加する
    for (PointSize i = 0; i < slices.size() - 1; i++) {
        for (int polate_rate = 0; polate_rate < interpolate_size + 1; polate_rate++) {
            for (PointSize j = 0; j < slices[i].points.size(); j++) {
                double t = (double)polate_rate / (interpolate_size + 1);
                Geom::Point2 p;

                if (interp_type == InterpolationType::Linear) {
                    Geom::Point2 p1 = slices[i].points[j];
                    Geom::Point2 p2 = slices[i + 1].points[j];
                    double x = p1.x * (1 - t) + p2.x * t;
                    double y = p1.y * (1 - t) + p2.y * t;
                    p = Geom::Point2{x, y};
                } else { // Spline
                    Geom::Point2 p0 = (i > 0) ? slices[i - 1].points[j] : slices[i].points[j];
                    Geom::Point2 p1 = slices[i].points[j];
                    Geom::Point2 p2 = slices[i + 1].points[j];
                    Geom::Point2 p3 = (i < slices.size() - 2) ? slices[i + 2].points[j] : slices[i + 1].points[j];
                    p = spline_interpolate(p0, p1, p2, p3, t);
                }
                points.emplace_back(Geom::Point3{
                    p.x, p.y, z
                });
            }
            z += length_per_time;
        }
    }

    // 最後の層はそのまま追加
    for (const auto& p : slices.back().points) {
        points.emplace_back(Geom::Point3{
            p.x, p.y, z
        });
    }

    this->step_size = exact_slice_size;

    return true;
}

bool LayerObj3D::check_slice_point_size() const {
    for (const auto& slice : slices) {
        if (slice.points.size() != point_size_per_step) return false;
    }
    return true;
}

std::istream& operator>>(std::istream& ist, LayerObj3D& obj) {
    Slice slice;
    while (ist >> slice) {
        obj.slices.push_back(slice);
    }

    if (ist.fail() && !ist.eof()) {
        return ist;
    }

    bool is_ok = true;
    obj.step_size = obj.slices.size();
    if (obj.step_size == 0) {
        is_ok = false;
        std::cerr << "slice size is 0\n";
    } else {
        obj.point_size_per_step = obj.slices[0].points.size();
        obj.first_slice = obj.slices[0];
        obj.last_slice = obj.slices.back();
    }

    is_ok &= obj.check_slice_point_size();
    is_ok &= obj.from_slices(obj.INTERPOLATE_SIZE);

    if (!is_ok) {
        // スライスからの生成に失敗
        ist.setstate(std::ios_base::failbit);
    };
    return ist;
}

bool LayerObj3D::from_slices(int interpolate_size) {
    if (interpolate_size == 0) {
        return TraceObj3D::from_slices();
    }

    bool is_ok = true;
    // 線形補間が指定されている場合は, 線形補間付き make_points を呼び出す
    is_ok &= make_points(interpolate_size, interpolation_type);
    is_ok &= make_faces_from_slices();
    return is_ok;
}
