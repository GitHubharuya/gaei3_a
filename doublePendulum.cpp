#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>
#include <numeric>
#include "TraceCurveObj3D.hpp"
using namespace std;
using ll = long long;
using ld = long double;
#define M_PI 3.1415926535

// ===== 定数 =====
const ld G = 9.8;
// 棒の長さ
const ld L1 = 10.0;
const ld L2 = 10.0;
// 重りの重さ
const ld M1 = 2.0;
const ld M2 = 1.0;
// 最下部半径
const ld BALL_RADIUS = 0.7;

// 状態ベクトル（[th1, w1, th2, w2]）の型を定義

// linspace関数の実装
vector<ld> linspace(ld start, ld end, int num, bool endpoint = true) {
    vector<ld> result;
    if (num<=0) return result;
    if (num==1) {
        result.push_back(start);
        return result;
    }
    ld step = (end - start) / (endpoint ? (num - 1) : num);
    for (int i = 0; i < num; ++i) {
        result.push_back(start + i * step);
    }
    return result;
}

// ===== 運動方程式 =====
// state(状態)を与えると、その瞬間の変化率(dydx)を計算する関数
vector<ld> derivs(const vector<ld> &state) {
    vector<ld> dydx((int)state.size());
    dydx[0] = state[1];
    ld delta = state[2] - state[0];
    ld cos_delta = cos(delta);
    ld sin_delta = sin(delta);

    ld den1 = (M1 + M2) * L1 - M2 * L1 * cos_delta * cos_delta;
    dydx[1] = ((M2 * L1 * state[1] * state[1] * sin_delta * cos_delta
                + M2 * G * sin(state[2]) * cos_delta
                + M2 * L2 * state[3] * state[3] * sin_delta
                - (M1 + M2) * G * sin(state[0])) / den1);

    dydx[2] = state[3];
    ld den2 = (L2 / L1) * den1;
    dydx[3] = ((-M2 * L2 * state[3] * state[3] * sin_delta * cos_delta
                + (M1 + M2) * G * sin(state[0]) * cos_delta
                - (M1 + M2) * L1 * state[1] * state[1] * sin_delta
                - (M1 + M2) * G * sin(state[2])) / den2);
    return dydx;
}


int main() {
    // ===== 時間・初期条件 =====
    ld t_start = 0.0;
    ld t_end = 15.0;
    ld dt = 0.01;

    ld th1_rad = 120.0 * M_PI / 180.0;
    ld w1_rad = 0.0 * M_PI / 180.0;
    ld th2_rad = -10.0 * M_PI / 180.0;
    ld w2_rad = 0.0 * M_PI / 180.0;
    vector<ld> state = {th1_rad, w1_rad, th2_rad, w2_rad};

    // ===== ルンゲ＝クッタ法による数値積分 =====
    vector<vector<ld>> y_history;
    vector<ld> t_history;
    
    // 最初の状態を記録
    y_history.push_back(state);
    t_history.push_back(t_start);

    for (ld t = t_start; t < t_end; t += dt) {
        // --- 4次のルンゲ＝クッタ法 (RK4) ---
        vector<ld> k1(4), k2(4), k3(4), k4(4);
        vector<ld> temp_state(4);

        // k1
        k1 = derivs(state);

        // k2
        for(int i=0; i<4; ++i) temp_state[i] = state[i] + (k1[i] * dt / 2.0);
        k2 = derivs(temp_state);

        // k3
        for(int i=0; i<4; ++i) temp_state[i] = state[i] + (k2[i] * dt / 2.0);
        k3 = derivs(temp_state);
        
        // k4
        for(int i=0; i<4; ++i) temp_state[i] = state[i] + (k3[i] * dt);
        k4 = derivs(temp_state);

        // 状態を更新
        for(int i=0; i<4; ++i) {
            state[i] = state[i] + (dt / 6.0) * (k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]);
        }
        
        // 結果を保存
        y_history.push_back(state);
        t_history.push_back(t + dt);
    }

    // ===== 円周上の点を出力 =====
    const int n_circle_points = 16;
    const ld T_record = 15.0;
    const vector<ld> angles = linspace(0, 2 * M_PI, n_circle_points, false);
    
    ofstream outfile("circle_points.txt");
    if (!outfile) {
        cerr << "エラー: ファイルを開けませんでした。" << endl;
        return 1;
    }

    vector<double> xs,ys;


    for (ld tt = 0; tt <= T_record; tt += 0.01) {
        auto it = min_element(t_history.begin(), t_history.end(), 
            [tt](ld a, ld b) { return abs(a - tt) < abs(b - tt); });
        int idx = distance(t_history.begin(), it);

        ld current_th1 = y_history[idx][0];
        ld current_th2 = y_history[idx][2];

        ld x1 = L1 * sin(current_th1);
        ld y1 = -L1 * cos(current_th1);
        ld x2_center = L2 * sin(current_th2) + x1;
        ld y2_center = -L2 * cos(current_th2) + y1;

        xs.push_back(x2_center), ys.push_back(y2_center);

        //stringstream ss;
        //ss << fixed << setprecision(6);

        // 先端の座標出力
        //ss << x2_center << " " << y2_center << " ";

        // z軸垂直の円周上の座標出力
        /*for(ld a : angles) {
            ld px = x2_center + BALL_RADIUS * cos(a + current_th2);
            ld py = y2_center + BALL_RADIUS * sin(a + current_th2);
            ss << px << " " << py << " ";
        }*/

        //string coords_str = ss.str();
        //coords_str.pop_back();
        //outfile << fixed << setprecision(2) << tt << ", " << coords_str << endl;
    }

    double total_length = 130;
    TraceCurveObj3D obj{xs, ys, total_length};
    cout << obj;

    //cout << "✅ 'circle_points.txt' に円周上の点座標を出力しました。" << endl;

    return 0;
}