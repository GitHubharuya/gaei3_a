import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import japanize_matplotlib
import math

# --- 準備 ---
fig = plt.figure(figsize=(6, 6)) # 少し大きめのウィンドウ
ax = fig.add_subplot(111, autoscale_on=False, xlim=(-3.5, 3.5), ylim=(-3.5, 3.5))
ax.set_aspect('equal')
ax.grid()

line, = ax.plot([], [], 'o-', lw=2) # 描画するラインオブジェクト
time_template = 'time = %.1fs'
time_text = ax.text(0.05, 0.9, '', transform=ax.transAxes)

# --- パラメータと定数 ---
g = 9.8
m_1 = 1.0
m_2 = 5.0
# 振り子の長さ
l_1 = 1.0
l_2 = 2.0
# 滑らかさ
dt = 0.1

# --- 初期値 ---
theta_1 = math.pi / 2.0
theta_2 = 0.0
d_theta_dt_1 = 0.0
d_theta_dt_2 = 0.0

# --- シミュレーション結果を保存するリスト ---
history_x, history_y = [], []

def init():
    """アニメーションの初期化関数"""
    line.set_data([], [])
    time_text.set_text('')
    return line, time_text

def animate(i):
    """各フレームを計算・描画する関数"""
    global theta_1, theta_2, d_theta_dt_1, d_theta_dt_2

    # --- ▼▼▼ 修正済みの物理計算 ▼▼▼ ---
    c1 = np.cos(theta_1 - theta_2)
    s1 = np.sin(theta_1 - theta_2)
    
    # 角加速度1 (dd_theta_dd_t_1)
    num1_1 = -m_2 * l_1 * (d_theta_dt_1**2) * s1 * c1
    num1_2 = -g * (m_1 + m_2) * np.sin(theta_1)
    num1_3 = -m_2 * l_2 * (d_theta_dt_2**2) * s1
    den1 = l_1 * (m_1 + m_2 * s1**2)
    dd_theta_dd_t_1 = (num1_1 + num1_2 + num1_3) / den1

    # 角加速度2 (dd_theta_dd_t_2)
    num2_1 = (m_1 + m_2) * l_1 * (d_theta_dt_1**2) * s1
    num2_2 = -g * (m_1 + m_2) * np.sin(theta_2)
    num2_3 = m_2 * l_2 * (d_theta_dt_2**2) * s1 * c1
    den2 = l_2 * (m_1 + m_2 * s1**2)
    dd_theta_dd_t_2 = (num2_1 + num2_2 + num2_3) / den2
    # --- ▲▲▲ 計算ここまで ▲▲▲ ---

    # オイラー法による数値積分
    d_theta_dt_1 += dd_theta_dd_t_1 * dt
    d_theta_dt_2 += dd_theta_dd_t_2 * dt
    theta_1 += d_theta_dt_1 * dt
    theta_2 += d_theta_dt_2 * dt

    # デカルト座標に変換
    x1 = l_1 * np.sin(theta_1)
    y1 = -l_1 * np.cos(theta_1)
    x2 = x1 + l_2 * np.sin(theta_2)
    y2 = y1 - l_2 * np.cos(theta_2)

    # 描画データを更新
    thisx = [0, x1, x2]
    thisy = [0, y1, y2]
    line.set_data(thisx, thisy)
    time_text.set_text(time_template % (i * dt))
    
    return line, time_text

# アニメーションオブジェクトを生成 (10秒分 = 1000フレーム)
ani = animation.FuncAnimation(fig, animate, range(1, int(30/dt)),
                              interval=dt*1000, blit=True, init_func=init)

# 保存
print("アニメーションをGIFとして保存中です... 少し時間がかかります。")
ani.save("furiko_2tai_simple.gif", writer="pillow", fps=30)
print("保存が完了しました。")

plt.show()