#円周上の点の座標を時間ごとにCircle_points.txtに出力

from numpy import sin, cos, pi
from scipy.integrate import solve_ivp
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle
from matplotlib.animation import FuncAnimation

# ===== 定数 =====
G = 9.8
L1 = 10
L2 = 10
M1 = 2.0
M2 = 1.0
ball_radius = 0.7  # 円の半径

# ===== 運動方程式 =====
def derivs(t, state):
    dydx = np.zeros_like(state)
    dydx[0] = state[1]
    delta = state[2] - state[0]
    den1 = (M1 + M2) * L1 - M2 * L1 * cos(delta)**2
    dydx[1] = ((M2 * L1 * state[1]**2 * sin(delta) * cos(delta)
                + M2 * G * sin(state[2]) * cos(delta)
                + M2 * L2 * state[3]**2 * sin(delta)
                - (M1 + M2) * G * sin(state[0])) / den1)
    dydx[2] = state[3]
    den2 = (L2 / L1) * den1
    dydx[3] = ((-M2 * L2 * state[3]**2 * sin(delta) * cos(delta)
                + (M1 + M2) * G * sin(state[0]) * cos(delta)
                - (M1 + M2) * L1 * state[1]**2 * sin(delta)
                - (M1 + M2) * G * sin(state[2])) / den2)
    return dydx

# ===== 時間・初期条件 =====
t_span = [0, 15]
dt = 0.02
t = np.arange(t_span[0], t_span[1], dt)
th1, w1, th2, w2 = np.radians([120.0, 0.0, -10.0, 0.0])
state = [th1, w1, th2, w2]
sol = solve_ivp(derivs, t_span, state, t_eval=t)
y = sol.y

# ===== 円周上の点を出力 =====
n = 16         # 円周上の点の数
T = 15         # 記録時間（秒）
record_times = np.arange(0, T + 1, 0.01)
angles = np.linspace(0, 2*pi, n, endpoint=False)

# 出力用リスト
lines = []

for tt in record_times:
    # 近い時間のインデックスを取得
    idx = np.argmin(np.abs(t - tt))
    th1 = y[0, idx]
    th2 = y[2, idx]
    # 振り子先端（円の中心）
    x1 = L1 * sin(th1)
    y1 = -L1 * cos(th1)
    x2 = L2 * sin(th2) + x1
    y2 = -L2 * cos(th2) + y1

    # 円周上の n 点を計算
    coords = []
    for a in angles:
        px = x2 + ball_radius * cos(a+th2)
        py = y2 + ball_radius * sin(a+th2)
        coords += [px, py]

    # 出力行（t, p0x p0y ...）
    line = f"{float(tt)}, " + " ".join(f"{v:.6f}" for v in coords)
    lines.append(line)

# ===== テキストファイルに保存 =====
with open("circle_points.txt", "w", encoding="utf-8") as f:
    for line in lines:
        f.write(line + "\n")

print("✅ 'circle_points.txt' に円周上の点座標を出力しました。")

# ===== アニメーション（任意、確認用） =====
fig, ax = plt.subplots()
ax.set_xlim(-(L1+L2+0.5), L1+L2+0.5)
ax.set_ylim(-(L1+L2+0.5), L1+L2+0.5)
ax.set_aspect('equal')
ax.grid()

line, = ax.plot([], [], 'o-', lw=2)
circle_points, = ax.plot([], [], 'go', ms=3)  # 円周上の点
trail, = ax.plot([], [], 'r-', lw=1, alpha=0.5)

trail_x, trail_y = [], []

def animate(i):
    th1, th2 = y[0, i], y[2, i]
    x1, y1 = L1*sin(th1), -L1*cos(th1)
    x2, y2 = x1 + L2*sin(th2), y1 - L2*cos(th2)

    # 円周上の点の計算
    xs = x2 + ball_radius * np.cos(angles+th2)
    ys = y2 + ball_radius * np.sin(angles+th2)

    line.set_data([0, x1, x2], [0, y1, y2])
    circle_points.set_data(xs, ys)

    # 先端の軌跡
    trail_x.append(x2)
    trail_y.append(y2)
    trail.set_data(trail_x, trail_y)
    

    return line, circle_points, trail

ani = FuncAnimation(fig, animate, frames=len(t), interval=20, blit=True)
plt.show()
