#include "Geom.hpp"
#include "TraceObj3D.hpp"

struct TraceCenterObj3D : public TraceObj3D {

    TraceCenterObj3D(
        const std::vector<double>& xs,
        const std::vector<double>& ys,
        const std::vector<Slice>& _slices, // WARGING: will move
        bool _is_same_slice = false
    ) : is_same_slice(_is_same_slice) {
        if (!_is_same_slice && _slices.size() < 3) {
            std::cerr << "slice is not enough\n";
            std::exit(1);
        }
        this->slices = std::move(_slices);
        center_points.clear();
        center_points.reserve(xs.size());
        double z = 0.0;
        for (PointSize i = 0; i < xs.size(); i++, z++) {
            center_points.emplace_back(Geom::Point3{ xs[i], ys[i], z });
        }

        from_slices();
    };

    std::vector<Geom::Point3> center_points;
    std::vector<Geom::Point3> norm_vecs;
    bool is_same_slice = false;
    bool make_points() override;
    void make_norm_vecs();
    void make_points_from_norm(const Geom::Point3 n, const Geom::Point3& a, const std::vector<Geom::Point2>& points2D);
};

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

void TraceCenterObj3D::make_points_from_norm(const Geom::Point3 n, const Geom::Point3& a, const std::vector<Geom::Point2>& points2D) {
    Geom::Point3 t { 0.0, 1.0, 0.0 };
    Geom::Point3 t_sub { 0.0, 0.0, 1.0 };
    Geom::Point3 u, v;
    double cos_theta = Geom::dot_prod(n, t);
    if (cos_theta > 0.99) {
        u = Geom::cross_prod(t_sub, n);
        u.normalize();
    } else {
        u = Geom::cross_prod(t, n);
        u.normalize();
    }
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
}

bool TraceCenterObj3D::make_points() {
    PointSize slice_size = slices.size();
    if (slice_size == 0) { return false; }
    PointSize slice_point_size = slices[0].points.size();

    make_points_from_norm(
        Geom::Point3{0, 0, 1}, 
        center_points[0],
        slices[0].points
    );

    make_norm_vecs();
    for (PointSize i = 0; i < norm_vecs.size(); i++) {
        PointIdx si = is_same_slice ? 0 : i+1;
        make_points_from_norm(
            norm_vecs[i],
            center_points[i + 1],
            slices[si].points
        );
    }

    make_points_from_norm(
        Geom::Point3{0, 0, 1}, 
        center_points.back(),
        slices.back().points
    );

    return true;
}
