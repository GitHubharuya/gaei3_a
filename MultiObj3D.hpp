#pragma once
#include "TraceObj3D.hpp"

struct MultiObj3D : TraceObj3D {
    MultiObj3D(const std::vector<TraceObj3D*>& _objs) {
        if (_objs.size() == 0) return;
        objs = _objs;
        points.clear(); faces.clear();
        PointSize obj_point_size = add_obj_points();

        add_stand(obj_point_size);
        add_shifted_face(0);
        // attach_stand_objs();

        // 親クラスの初期化
    }
    std::vector<TraceObj3D*> objs;

    bool make_points() override { return false; } // スタンドの追加は点と面を同一の場所で追加すべきなので make_points は定義しない

    PointSize add_obj_points();
    PointSize add_stand(PointSize offset);
    void add_shifted_face(PointSize offset);
    void attach_stand_objs();
};

PointSize MultiObj3D::add_stand(PointSize offset) {
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

    points.emplace_back(Geom::Point3{
        left, bottom, 0
    });
    points.emplace_back(Geom::Point3{
        right, bottom, 0
    });
    points.emplace_back(Geom::Point3{
        right, top, 0
    });
    points.emplace_back(Geom::Point3{
        left, top, 0
    });
    points.emplace_back(Geom::Point3{
        left, bottom, -stand_height
    });
    points.emplace_back(Geom::Point3{
        right, bottom, -stand_height
    });
    points.emplace_back(Geom::Point3{
        right, top, -stand_height
    });
    points.emplace_back(Geom::Point3{
        left, top, -stand_height
    });

    // 四角形を三角形2つにする
    auto insert_rect_face = [&](std::array<PointIdx, 4> idxs) {
        faces.emplace_back(std::array<PointIdx, 3>{idxs[0], idxs[1], idxs[2]});
        faces.emplace_back(std::array<PointIdx, 3>{idxs[2], idxs[3], idxs[0]});
    };

    insert_rect_face({ offset + 0, offset + 4, offset + 5, offset + 1, });
    insert_rect_face({ offset + 1, offset + 5, offset + 6, offset + 2, });
    insert_rect_face({ offset + 2, offset + 6, offset + 7, offset + 3, });
    insert_rect_face({ offset + 3, offset + 7, offset + 4, offset + 0, });
    insert_rect_face({ offset + 7, offset + 6, offset + 5, offset + 4, });

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
            // TODO: remove bottom faces
            faces.emplace_back(std::array<PointIdx, 3>{
                face[0] + offset,
                face[1] + offset,
                face[2] + offset
            });
        }
        offset += obj->points.size();
    }
}

void MultiObj3D::attach_stand_objs() {
}
