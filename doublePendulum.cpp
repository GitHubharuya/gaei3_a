#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip> // 出力フォーマット用
using namespace std;

int main() {
  // --- パラメータと定数 ---
  const double g = 9.8;
  // 接合部と先端の重さ
  const double m_1 = 1.0;
  const double m_2 = 2.0;
  // 振り子の長さ
  const double l_1 = 2.0;
  const double l_2 = 1.0;
  // 滑らかさ
  const double dt = 0.01; // 微小時間
  const double simulation_time = 10.0; // シミュレーション時間

  const double ballSize = 1.0;

  // --- 初期値 ---
  double theta_1 = 3.1415926535 / 2.0;
  double theta_2 = 0.0;
  double d_theta_dt_1 = 0.0;
  double d_theta_dt_2 = 0.0;

  // --- メインループ ---
  for (double t = 0; t < simulation_time; t += dt) {
    // --- 修正済みの物理計算 ---
    double c1 = cos(theta_1 - theta_2);
    double s1 = sin(theta_1 - theta_2);
    
    // 角加速度1 (dd_theta_dd_t_1)
    double num1_1 = -m_2 * l_1 * d_theta_dt_1 * d_theta_dt_1 * s1 * c1;
    double num1_2 = -g * (m_1 + m_2) * sin(theta_1);
    double num1_3 = -m_2 * l_2 * d_theta_dt_2 * d_theta_dt_2 * s1;
    double den1 = l_1 * (m_1 + m_2 * s1 * s1);
    double dd_theta_dd_t_1 = (num1_1 + num1_2 + num1_3) / den1;

    // 角加速度2 (dd_theta_dd_t_2)
    double num2_1 = (m_1 + m_2) * l_1 * d_theta_dt_1 * d_theta_dt_1 * s1;
    double num2_2 = -g * (m_1 + m_2) * sin(theta_2);
    double num2_3 = m_2 * l_2 * d_theta_dt_2 * d_theta_dt_2 * s1 * c1;
    double den2 = l_2 * (m_1 + m_2 * s1 * s1);
    double dd_theta_dd_t_2 = (num2_1 + num2_2 + num2_3) / den2;

    // --- オイラー法による数値積分 ---
    d_theta_dt_1 += dd_theta_dd_t_1 * dt;
    d_theta_dt_2 += dd_theta_dd_t_2 * dt;
    theta_1 += d_theta_dt_1 * dt;
    theta_2 += d_theta_dt_2 * dt;

    // --- デカルト座標に変換 ---
    double x1 = l_1 * sin(theta_1);
    double y1 = -l_1 * cos(theta_1);
    double x2 = x1 + l_2 * sin(theta_2);
    double y2 = y1 - l_2 * cos(theta_2);

    // --- 座標を出力 ---
    // fixed と setprecisionで小数点以下の桁数を固定
    cout << fixed << setprecision(2) << t << ", ";
    for(int i=0; i<360; i+=15){
      cout << x2+ballSize*cos(i) << ' ' << y2+ballSize*sin(i) << ' ';
    }
    cout << endl;
  }

  return 0;
}