import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# ===== パラメータ =====
grid_size = 101   # グリッドのサイズ（奇数が中心にアリを置きやすい）
steps = 11000     # シミュレーションステップ数
output_interval = 1  # 出力間隔（1で毎ステップ）

# ===== 初期化 =====
grid = np.zeros((grid_size, grid_size), dtype=int)  # 0=白, 1=黒
x, y = grid_size // 2, grid_size // 2  # アリの初期位置
direction = 0  # 0:上, 1:右, 2:下, 3:左
scale=0.5

# ===== 出力用リスト =====
positions = []

# ===== アリのルール関数 =====
def step():
    global x, y, direction, grid

    # 現在のマスの色に応じて回転
    if grid[y, x] == 0:  # 白なら右回転
        direction = (direction + 1) % 4
        grid[y, x] = 1
    else:  # 黒なら左回転
        direction = (direction - 1) % 4
        grid[y, x] = 0

    # 方向に応じて1歩進む
    if direction == 0:   # 上
        y -= 1
    elif direction == 1: # 右
        x += 1
    elif direction == 2: # 下
        y += 1
    elif direction == 3: # 左
        x -= 1

    # 枠外に出ないようにする（トーラス境界）
    x %= grid_size
    y %= grid_size

# ===== まず全ステップを計算して記録 =====
for i in range(steps):
    step()
    if i % output_interval == 0:
        positions.append((i, x, y))

# ===== ファイル出力 =====
with open("ant_path.txt", "w", encoding="utf-8") as f:
    for step_idx, px, py in positions:
        f.write(f"{step_idx}, {px*scale} {py*scale}\n")

print("✅ 'ant_path.txt' にアリの座標を出力しました。")

# ===== アニメーション =====
fig, ax = plt.subplots()
im = ax.imshow(grid, cmap='binary', origin='upper')
ant_dot, = ax.plot([], [], 'ro', markersize=4)
ax.set_title("Langton's Ant")

def init():
    im.set_data(grid)
    ant_dot.set_data([], [])
    return im, ant_dot

def update(frame):
    step()
    im.set_data(grid)
    ant_dot.set_data(x, y)
    ax.set_xlabel(f"Step: {frame}")
    return im, ant_dot

ani = FuncAnimation(fig, update, frames=steps, init_func=init,
                    interval=1, blit=True, repeat=False)

plt.show()
