#pragma once

#include "Geom.hpp"
#include "TraceObj3D.hpp"

struct TraceCenterObj3D : public TraceObj3D {

    TraceCenterObj3D(
        const std::vector<double>& xs,
        const std::vector<double>& ys,
        const std::vector<Slice>& _slices, // WARGING: will move
        double _total_length = 100,
        bool _is_same_slice = false
    ) {
        if (xs.size() != ys.size()) {
            std::cerr << "TraceCenterObj3D: not same size points\n";
            std::exit(1);
        }
        is_same_slice = _is_same_slice;
        TOTAL_LENGTH = _total_length;
        double length_per_time = TOTAL_LENGTH / xs.size();
        if (!_is_same_slice && _slices.size() < 3) {
            std::cerr << "slice is not enough\n";
            std::exit(1);
        }
        if (_is_same_slice && _slices.size() != 1) {
            std::cerr << "single slice with is_same_slice flag\n";
            std::exit(1);
        }

        slices = std::move(_slices);

        // これらは親クラスの変数
        step_size = xs.size();
        point_size_per_step = slices[0].points.size();
        first_slice = slices[0];
        last_slice = slices.back();

        center_points.clear();
        center_points.reserve(xs.size());
        double z = 0.0;
        for (PointSize i = 0; i < xs.size(); i++) {
            center_points.emplace_back(Geom::Point3{ xs[i], ys[i], z });
            z += length_per_time;
        }

        if (!from_slices()) {
            std::cerr << "Error: in TraceCenterObj3D constuctor\n";
            std::exit(1);
        }
    };

    double TOTAL_LENGTH = 100;
    std::vector<Slice> slices;
    std::vector<Geom::Point3> center_points;
    std::vector<Geom::Point3> norm_vecs;
    bool is_same_slice = false;
    bool make_points() override;
    int count_make_point_call = 0;
    void make_norm_vecs();
    Geom::Point3 make_points_from_norm(const Geom::Point3& n, const Geom::Point3& t, const Geom::Point3& a, const std::vector<Geom::Point2>& points2D);
    bool check_intersect(std::vector<PointIdx>& intercect_step_info) const;
    bool is_front_points(const Geom::Point3& n, const Geom::Point3& o, PointIdx start, PointIdx end) const;
    double add_first_end(const Geom::Point3& n);
    Geom::Point3 first_face_y_axis;
    double bend_first_edge(int turn_step);
};

double TraceCenterObj3D::bend_first_edge(int turn_step) {
    double pi = std::acos(-1);
    double d_theta = pi / (turn_step * 2.0);
    double cos_t = std::cos(d_theta);
    double sin_t = std::sin(d_theta);
    Geom::Point3 n {
        center_points[0].x - center_points[1].x,
        center_points[0].y - center_points[1].y,
        center_points[0].z - center_points[1].z
    };
    n.normalize();

    Geom::Point3 vec_u;
    double u, v, nextu, nextv;
    double d = 1;
    int ct = 0;
    double min_z = 0;
    while (ct++ < turn_step && 1 + n.z > 1e-4) {
        vec_u.x = n.x; vec_u.y = n.y; vec_u.z = 0;
        vec_u.normalize();
        u = Geom::dot_prod(vec_u, n);
        v = n.z;
        nextu = cos_t * u + sin_t * v;
        nextv = -sin_t * u + cos_t * v;
        n.x = nextu * vec_u.x;
        n.y = nextu * vec_u.y;
        n.z = nextu * vec_u.z + nextv;
        n.normalize();
        n.x *= d; n.y *= d; n.z *= d;
        min_z = std::min(min_z, add_first_end(n));
    }
    return min_z;
}

double TraceCenterObj3D::add_first_end(const Geom::Point3& n) {
    Geom::Point3 a = center_points[front_face_idx];
    a.x += n.x; a.y += n.y; a.z += n.z;
    Geom::Point3 n_ {-n.x, -n.y, -n.z};
    first_face_y_axis = make_points_from_norm(n_, first_face_y_axis, a, slices.begin()->points);
    double min_z = 0;
    for (PointIdx i = points.size() - point_size_per_step; i < points.size(); i++) {
        min_z = std::min(min_z, points[i].z);
    }
    add_faces(step_size, front_face_idx);
    front_face_idx = step_size;
    step_size++;
    center_points.push_back(a);
    return min_z;
}

void TraceCenterObj3D::make_norm_vecs() {
    norm_vecs.clear();
    norm_vecs.reserve(center_points.size() - 2);
    for (PointIdx i = 0; i < center_points.size() - 2; i++) {
        norm_vecs.emplace_back(Geom::Point3{
            center_points[i + 2].x - center_points[i].x,
            center_points[i + 2].y - center_points[i].y,
            center_points[i + 2].z - center_points[i].z,
        });
        norm_vecs[i].normalize();
    }
}

Geom::Point3 TraceCenterObj3D::make_points_from_norm(const Geom::Point3& n, const Geom::Point3& t_main, const Geom::Point3& a, const std::vector<Geom::Point2>& points2D) {
    Geom::Point3 t_sub { 0.0, 0.0, -1.0 }; // fix
    Geom::Point3 t, u, v;
    double cos_theta = Geom::dot_prod(n, t_main);
    if (cos_theta > 0.99) {
        t = t_sub;
    } else {
        t = t_main;
    }
    u = Geom::cross_prod(t, n);
    u.normalize();
    v = Geom::cross_prod(n, u);
    v.normalize();
    points.reserve(points2D.size());
    for (const auto& p : points2D) {
        points.emplace_back(Geom::Point3{
            p.x * u.x + p.y * v.x + a.x,
            p.x * u.y + p.y * v.y + a.y,
            p.x * u.z + p.y * v.z + a.z,
        });
    }
    return v;
}

bool TraceCenterObj3D::make_points() {

    count_make_point_call++;
    points.clear();
    norm_vecs.clear();

    Geom::Point3 first_n {
        center_points[1].x - center_points[0].x,
        center_points[1].y - center_points[0].y,
        center_points[1].z - center_points[0].z
    };
    Geom::Point3 t { 0.0, 1.0, 0.0 };
    t = make_points_from_norm(
        first_n,
        t,
        center_points[0],
        slices[0].points
    );
    first_face_y_axis = t;

    make_norm_vecs();
    for (PointSize i = 0; i < norm_vecs.size(); i++) {
        PointIdx si = is_same_slice ? 0 : i+1;
        t = make_points_from_norm(
            norm_vecs[i],
            t,
            center_points[i + 1],
            slices[si].points
        );
    }

    Geom::Point3 last_n {
        center_points.back().x - center_points[center_points.size() - 2].x,
        center_points.back().y - center_points[center_points.size() - 2].y,
        center_points.back().z - center_points[center_points.size() - 2].z
    };
    make_points_from_norm(
        last_n,
        t,
        center_points.back(),
        slices.back().points
    );

    std::vector<PointIdx> intercect_step_info;
    if (!check_intersect(intercect_step_info)) {
        std::cerr << count_make_point_call << "回目: " << intercect_step_info.size() << " ステップの交差を検出\n";
        double offset = 0.0;
        double eps = 1e-4;
        PointIdx isi = 0;
        for (PointIdx i = intercect_step_info[isi]; i < center_points.size(); i++) {
            if (isi < intercect_step_info.size() && i == intercect_step_info[isi]) {
                offset += eps;
                isi++;
            }
            center_points[i].z += offset;
        }
        make_points();
    }

    return true;
}

bool TraceCenterObj3D::is_front_points(const Geom::Point3& n, const Geom::Point3& o, PointIdx start, PointIdx end) const {
    for (PointIdx i = start; i < end; i++) {
        double dot = Geom::dot_prod(n,
            Geom::Point3{
                points[i].x - o.x,
                points[i].y - o.y,
                points[i].z - o.z,
            }
        );
        if (dot <= 0) { return false; }
    }
    return true;
}

bool TraceCenterObj3D::check_intersect(std::vector<PointIdx>& intercect_step_info) const {
    // 最初の面をチェック
    PointSize origin_size = intercect_step_info.size();
    bool is_front = is_front_points(
        Geom::Point3{ -norm_vecs[0].x, -norm_vecs[0].y, -norm_vecs[0].z },
        center_points[1],
        0, point_size_per_step
    );
    if (!is_front) {
        intercect_step_info.push_back(1); // オフセットを追加する面
    }

    for (PointSize i = 0; i < norm_vecs.size(); i++) {
        PointIdx start = point_size_per_step * (i + 2);
        PointIdx end = point_size_per_step * (i + 3);
        is_front = is_front_points(
            norm_vecs[i],
            center_points[i + 1], // 最初の面を除く
            start, end
        );
        if (!is_front) {
            intercect_step_info.push_back(i+2); // オフセットを追加する面
        }
    }
    return intercect_step_info.size() == origin_size;
}
