#pragma once
#include "TraceObj3D.hpp"
#include <tuple>

struct MultiObj3D : TraceObj3D {
    MultiObj3D(const std::vector<TraceObj3D*>& _objs) {
        if (_objs.size() == 0) return;
        objs = _objs;
        points.clear(); faces.clear();
        double stand_z = attach_stand_objs();
        add_obj_points();

        add_shifted_face(0);
        add_stand(stand_z);

        // 親クラスの初期化
    }

    std::vector<TraceObj3D*> objs;
    PointIdx stand_point_idx_begin = 0;
    PointSize stand_point_size = 0;

    // スタンドの追加は点と面を同一の場所で追加すべきなので定義しない
    bool make_points() override { return false; }
    bool make_faces_from_slices() override { return false; }
    bool from_slices() override { return false; }

    PointSize add_obj_points();
    PointSize add_stand(double stand_z);
    void add_shifted_face(PointSize offset);
    double attach_stand_objs();
};

PointSize MultiObj3D::add_stand(double stand_z) {
    double max_x = objs[0]->points[0].x;
    double min_x = objs[0]->points[0].x;
    double max_y = objs[0]->points[0].y;
    double min_y = objs[0]->points[0].y;
    for (const auto& obj : objs) {
        for (const auto& p : obj->points) {
            max_x = std::max(max_x, p.x);
            min_x = std::min(min_x, p.x);
            max_y = std::max(max_y, p.y);
            min_y = std::min(min_y, p.y);
        }
    }

    double xmargin = (max_x - min_x) * 0.2;
    double ymargin = (max_y - min_y) * 0.2;
    double left = min_x - xmargin;
    double right = max_x + xmargin;
    double bottom = min_y - ymargin;
    double top = max_y + ymargin;
    double stand_height = 10;

    stand_point_idx_begin = points.size();
    points.emplace_back(Geom::Point3{
        left, bottom, stand_z
    });
    points.emplace_back(Geom::Point3{
        right, bottom, stand_z
    });
    points.emplace_back(Geom::Point3{
        right, top, stand_z
    });
    points.emplace_back(Geom::Point3{
        left, top, stand_z
    });
    points.emplace_back(Geom::Point3{
        left, bottom, stand_z - stand_height
    });
    points.emplace_back(Geom::Point3{
        right, bottom, stand_z - stand_height
    });
    points.emplace_back(Geom::Point3{
        right, top, stand_z - stand_height
    });
    points.emplace_back(Geom::Point3{
        left, top, stand_z - stand_height
    });

    // 四角形を三角形2つにする
    auto insert_rect_face = [&](std::array<PointIdx, 4> idxs) {
        faces.emplace_back(std::array<PointIdx, 3>{idxs[0], idxs[1], idxs[2]});
        faces.emplace_back(std::array<PointIdx, 3>{idxs[2], idxs[3], idxs[0]});
    };

    PointIdx offset = stand_point_idx_begin;
    insert_rect_face({ offset + 0, offset + 4, offset + 5, offset + 1, });
    insert_rect_face({ offset + 1, offset + 5, offset + 6, offset + 2, });
    insert_rect_face({ offset + 2, offset + 6, offset + 7, offset + 3, });
    insert_rect_face({ offset + 3, offset + 7, offset + 4, offset + 0, });
    insert_rect_face({ offset + 7, offset + 6, offset + 5, offset + 4, });

    stand_point_size = 8;
    return 8; // stand point size
}

PointSize MultiObj3D::add_obj_points() {
    PointSize obj_point_size_all = 0;
    for (const auto& obj : objs) {
        obj_point_size_all += obj->points.size();
    }
    points.reserve(obj_point_size_all);
    for (const auto& obj : objs) {
        points.insert(points.end(), obj->points.begin(), obj->points.end());
    }
    return obj_point_size_all;
}

void MultiObj3D::add_shifted_face(PointSize offset) {
    PointSize obj_face_size = 0;
    for (const auto& obj : objs) {
        obj_face_size += obj->faces.size();
    }
    faces.reserve(obj_face_size);
    for (const auto& obj : objs) {
        for (const auto& face : obj->faces) {
            if (
                face[0] >= 0 && face[0] < obj->point_size_per_step
                && face[1] >= 0 && face[1] < obj->point_size_per_step
                && face[2] >= 0 && face[2] < obj->point_size_per_step
            ) {
                continue;
            }
            faces.emplace_back(std::array<PointIdx, 3>{
                face[0] + offset,
                face[1] + offset,
                face[2] + offset
            });
        }
        offset += obj->points.size();
    }
}

// @return: stand z coordinate
double MultiObj3D::attach_stand_objs() {
    int turn_step = 16;
    double min_z = 0;
    PointIdx min_z_idx = 0;
    for (PointIdx i = 0; i < objs.size(); i++) {
        if (auto curve_obj = dynamic_cast<TraceCenterObj3D*>(objs[i])) {
            double tmp = curve_obj->bend_first_edge(turn_step);
            if (tmp < min_z) {
                min_z = tmp;
                min_z_idx = i;
            }
        }
    }

    // そろえる
    for (PointIdx i = 0; i < objs.size(); i++) {
        if (i == min_z_idx) continue;
        auto obj = objs[i];
        PointSize psize = obj->point_size_per_step;
        std::vector<Geom::Point3> add_ps(psize);
        for (PointIdx j = 0; j < psize; j++) {
            add_ps[j] = obj->points[obj->front_face_idx * psize + j];
            add_ps[j].z = min_z;
        }
        objs[i]->push_front_step(add_ps);
    }

    return min_z; // @return: stand z coordinate
}
