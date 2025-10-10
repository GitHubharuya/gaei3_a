#pragma once

#include <iostream>
#include <vector>
#include <limits>

struct Slice {
    double t;
    struct Point2 {
        double x, y;
    };
    std::vector<Slice::Point2> points;
    void sort_by_direction();
};

 // 形式: t, x0 y0 x1 y1 ... xn yn'\n'
 // WARNING: 行末に空白があると失敗することがある.
std::istream& operator>>(std::istream& ist, Slice& slice) {
    if (!(ist >> slice.t) || ist.get() != ',') {
        // t とカンマの読み込みに失敗
        ist.setstate(std::ios_base::failbit);
        return ist;
    }

    slice.points.clear();

    double x, y;
    while (ist.peek() != '\n' && ist.peek() != EOF) {
        if (ist >> x >> y) {
            slice.points.emplace_back(Slice::Point2{x, y});
        } else {
            // 点の座標の読み込みに失敗
            ist.setstate(std::ios_base::failbit);
            break;
        }
    }

    // ストリームが正常な場合、行の残りを読み飛ばす
    if (ist) {
        ist.ignore(
            std::numeric_limits<std::streamsize>::max(),
            '\n'
        );
    }

    return ist;
}

