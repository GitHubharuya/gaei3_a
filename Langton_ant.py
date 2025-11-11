import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# ===== パラメータ =====
grid_size = 101
steps = 1000
output_interval = 1
scale = 0.5

# ===== 初期化 =====
grid = np.zeros((grid_size, grid_size), dtype=int)
x, y = grid_size // 2, grid_size // 2
direction = 0  # 0:上, 1:右, 2:下, 3:左

# ===== 三角形の形（相対座標） =====
# 中心を原点として、向き0(上)を基準にした三角形
# 前方1.0、後方0.5、幅0.6程度の大きさ
triangle_shape = np.array([
    [0,  1.0],    # 頂点（前方）
    [-0.5, -0.5], # 左後方
    [0.5, -0.5]   # 右後方
]) * 0.5  # サイズスケーリング

# ===== 出力用リスト =====
triangle_positions = []

# ===== アリのルール関数 =====
def step():
    global x, y, direction, grid

    if grid[y, x] == 0:
        direction = (direction + 1) % 4
        grid[y, x] = 1
    else:
        direction = (direction - 1) % 4
        grid[y, x] = 0

    if direction == 0:   # 上
        y -= 1
    elif direction == 1: # 右
        x += 1
    elif direction == 2: # 下
        y += 1
    elif direction == 3: # 左
        x -= 1

    x %= grid_size
    y %= grid_size

# ===== 三角形の座標を求める関数 =====
def get_triangle_points(x, y, direction):
    # 向きに応じた回転角（ラジアン）
    angle = direction * np.pi / 2
    R = np.array([[np.cos(angle), -np.sin(angle)],
                  [np.sin(angle),  np.cos(angle)]])
    rotated = triangle_shape @ R.T
    translated = rotated + np.array([x, y])
    return translated

for i in range(9600):
    step()

# ===== シミュレーション＋出力 =====
for i in range(steps):
    step()
    if i % output_interval == 0:
        tri = get_triangle_points(x, y, direction)
        flat = " ".join(f"{p[0]*scale:.3f} {p[1]*scale:.3f}" for p in tri)
        triangle_positions.append(f"{i}, {flat}")


# ===== 標準出力 =====
for line in triangle_positions:
        print(line)

# ===== アニメーション描画 =====
# ★★★ アニメーションのために状態をリセット ★★★
grid = np.zeros((grid_size, grid_size), dtype=int)
x, y = grid_size // 2, grid_size // 2
direction = 0  # 0:上, 1:右, 2:下, 3:左

for i in range(9600):
    step()

fig, ax = plt.subplots()
im = ax.imshow(grid, cmap='binary', origin='upper', vmin=0, vmax=1)
tri_patch, = ax.plot([], [], 'r-', lw=1.5)

ax.set_title("Langton's Ant (Triangle Representation)")
ax.set_xlim(0, grid_size)
ax.set_ylim(grid_size, 0)  # y軸を上方向に反転

def init():
    im.set_data(grid)
    tri_patch.set_data([], [])
    return im, tri_patch

def update(frame):
    step()
    im.set_data(grid)
    tri = get_triangle_points(x, y, direction)
    xs, ys = np.append(tri[:,0], tri[0,0]), np.append(tri[:,1], tri[0,1])
    tri_patch.set_data(xs, ys)
    ax.set_xlabel(f"Step: {frame}")
    return im, tri_patch

ani = FuncAnimation(fig, update, frames=steps, init_func=init,
                    interval=1, blit=True, repeat=False)

plt.show()