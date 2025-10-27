#pragma once
#include "TraceObj3D.hpp"
#include "delaunator.hpp"
#include <tuple>
#include <limits>

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

    double stand_height = 2.0;
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
    std::vector<PointSize> objs_point_offset;
    std::vector<PointSize> first_point_face_offset;
};

PointSize MultiObj3D::add_stand(double stand_z) {
    double max_x = objs[0]->points[0].x;
    double min_x = objs[0]->points[0].x;
    double max_y = objs[0]->points[0].y;
    double min_y = objs[0]->points[0].y;
    double min_z = objs[0]->points[0].z;
    double max_z = objs[0]->points[0].z;
    for (const auto& obj : objs) {
        for (const auto& p : obj->points) {
            max_x = std::max(max_x, p.x);
            min_x = std::min(min_x, p.x);
            max_y = std::max(max_y, p.y);
            min_y = std::min(min_y, p.y);
            max_z = std::max(max_z, p.z);
            min_z = std::min(min_z, p.z);
        }
    }

    Geom::Point3 exact_size { max_x - min_x, max_y - min_y, max_z - min_z };
    Geom::Point3 center { min_x + exact_size.x / 2.0, min_y + exact_size.y / 2.0, min_z + exact_size.z / 2.0, };
    double xmargin = exact_size.x * 0.2;
    double ymargin = exact_size.y * 0.2;
    double min_length = exact_size.z / 3.0;
    Geom::Point3 stand_size {
        std::max(exact_size.x + 2.0 * xmargin, min_length),
        std::max(exact_size.y + 2.0 * ymargin, min_length),
        stand_height
    };
    double left = center.x - stand_size.x / 2.0;
    double right = center.x + stand_size.x / 2.0;
    double bottom = center.y - stand_size.y / 2.0;
    double top = center.y + stand_size.y / 2.0;

    stand_point_idx_begin = points.size();
    std::vector<Geom::Point3> stand_points = {
        Geom::Point3{ left, bottom, stand_z },
        Geom::Point3{ right, bottom, stand_z },
        Geom::Point3{ right, top, stand_z },
        Geom::Point3{ left, top, stand_z },
        Geom::Point3{ left, bottom, stand_z - stand_height },
        Geom::Point3{ right, bottom, stand_z - stand_height },
        Geom::Point3{ right, top, stand_z - stand_height },
        Geom::Point3{ left, top, stand_z - stand_height },
    };

    points.insert(points.end(), stand_points.begin(), stand_points.end());

    // 四角形を三角形2つにする
    auto insert_rect_face = [&](std::array<PointIdx, 4> idxs) {
        faces.emplace_back(std::array<PointIdx, 3>{idxs[0], idxs[1], idxs[2]});
        faces.emplace_back(std::array<PointIdx, 3>{idxs[2], idxs[3], idxs[0]});
    };

    PointIdx stand_offset = stand_point_idx_begin;
    insert_rect_face({ stand_offset + 0, stand_offset + 4, stand_offset + 5, stand_offset + 1, });
    insert_rect_face({ stand_offset + 1, stand_offset + 5, stand_offset + 6, stand_offset + 2, });
    insert_rect_face({ stand_offset + 2, stand_offset + 6, stand_offset + 7, stand_offset + 3, });
    insert_rect_face({ stand_offset + 3, stand_offset + 7, stand_offset + 4, stand_offset + 0, });
    insert_rect_face({ stand_offset + 7, stand_offset + 6, stand_offset + 5, stand_offset + 4, });

    // attach face
    std::vector<double> cap_xys;
    std::vector<int> cap_obj_idx; // 点からそれが属するobjの添え字を得るテーブル. objs.size() の時は土台の点
    std::vector<int> cap_obj_offset; // その点が属するobjの点がcap_xysで最初に現れる添え字
    for (auto p : stand_points) {
        if (p.z != stand_z) continue;
        cap_xys.push_back(p.x);
        cap_xys.push_back(p.y);
        cap_obj_idx.push_back(objs.size());
        cap_obj_offset.push_back(0);
    }
    for (PointIdx i = 0; i < objs.size(); i++) {
        auto& obj = objs[i];
        PointIdx end = first_point_face_offset[i] + obj->point_size_per_step;
        int offset = cap_xys.size() / 2;
        for (PointIdx j = first_point_face_offset[i]; j < end; j++) {
            cap_xys.push_back(obj->points[j].x);
            cap_xys.push_back(obj->points[j].y);
            cap_obj_idx.push_back(i);
            cap_obj_offset.push_back(offset);
        }
    }

    delaunator::Delaunator d(cap_xys);
    auto get_pidx = [&](PointIdx idx) -> PointIdx {
        PointIdx obj_idx = cap_obj_idx[idx];
        if (obj_idx == objs.size()) {
            return idx + stand_offset;
        }
        PointIdx offset_in_obj = first_point_face_offset[obj_idx];
        PointIdx relative_idx = idx - cap_obj_offset[idx];
        return objs_point_offset[obj_idx] + offset_in_obj + relative_idx;
    };

    for (PointSize i = 0; i < d.triangles.size(); i+=3) {
        PointIdx tri1 = d.triangles[i];
        PointIdx tri2 = d.triangles[i + 1];
        PointIdx tri3 = d.triangles[i + 2];
        if (cap_obj_idx[tri1] == cap_obj_idx[tri2] && cap_obj_idx[tri2] == cap_obj_idx[tri3]) {
            continue;
        }
        PointIdx p1 = get_pidx(d.triangles[i]);
        PointIdx p2 = get_pidx(d.triangles[i + 1]);
        PointIdx p3 = get_pidx(d.triangles[i + 2]);
        // TODO: 反時計回りか
        std::array<PointIdx, 3> tri = {p1, p3, p2};
        faces.emplace_back(tri);
    }

    stand_point_size = 8;
    return 8; // stand point size
}

PointSize MultiObj3D::add_obj_points() {
    PointSize obj_point_size_all = 0;
    for (const auto& obj : objs) {
        obj_point_size_all += obj->points.size();
    }
    points.reserve(obj_point_size_all);
    objs_point_offset.resize(objs.size());
    for (PointIdx i = 0; i < objs.size(); i++) {
        objs_point_offset[i] = points.size();
        points.insert(points.end(), objs[i]->points.begin(), objs[i]->points.end());
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
    first_point_face_offset.clear();
    first_point_face_offset.resize(objs.size(), 0);

    int turn_step = 16;
    for (PointIdx i = 0; i < objs.size(); i++) {
        if (auto curve_obj = dynamic_cast<TraceCenterObj3D*>(objs[i])) {
            PointIdx added_offset = curve_obj->bend_first_edge(turn_step);
            if (added_offset != 0) {
                first_point_face_offset[i] = added_offset;
            }
        }
    }

    double min_z = std::numeric_limits<double>::max();
    PointIdx min_z_idx = 0;

    bool first_point_found = false;
    for (PointIdx i = 0; i < objs.size(); i++) {
        auto& obj = objs[i];
        PointIdx offset = first_point_face_offset[i];
        for (PointIdx j = 0; j < obj->point_size_per_step; j++) {
            double current_z = obj->points[offset + j].z;
            if (!first_point_found || current_z < min_z) {
                min_z = current_z;
                min_z_idx = i;
                first_point_found = true;
            }
        }
    }
    if (!first_point_found) {
        min_z = 0;
        min_z_idx = 0;
    }

    // そろえる
    for (PointIdx i = 0; i < objs.size(); i++) {
        if (i == min_z_idx) continue;

        auto obj = objs[i];
        PointSize psize = obj->point_size_per_step;
        std::vector<Geom::Point3> add_ps(psize);
        for (PointIdx j = 0; j < psize; j++) {
            add_ps[j] = obj->points[first_point_face_offset[i] + j];
            add_ps[j].z = min_z;
        }
        first_point_face_offset[i] = objs[i]->push_front_step(add_ps);
    }

    return min_z;
}
