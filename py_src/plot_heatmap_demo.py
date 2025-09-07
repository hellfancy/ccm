import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from ccm import ccm_matrix

# --- 1. Generate Synthetic Data with a Known Causal Chain ---
# We create a system where X drives Y, and Y drives Z (X -> Y -> Z).

print("Generating synthetic data for the causal chain X -> Y -> Z...")
n = 1000  # Length of time series

# Logistic map parameter
r = 3.8
beta_xy = 0.2  # Coupling strength from X to Y
beta_yz = 0.2  # Coupling strength from Y to Z

x = np.zeros(n)
y = np.zeros(n)
z = np.zeros(n)
x[0], y[0], z[0] = 0.4, 0.2, 0.3

for t in range(n - 1):
    fx = x[t] * (r - r * x[t])
    fy = y[t] * (r - r * y[t])
    fz = z[t] * (r - r * z[t])
    x[t+1] = fx
    y[t+1] = (1 - beta_xy) * fy + beta_xy * fx
    z[t+1] = (1 - beta_yz) * fz + beta_yz * fy

# Combine into a single data matrix (each column is a time series)
data = np.vstack([x, y, z]).T
series_names = ['X', 'Y', 'Z']

# --- 2. Compute the CCM Causality Matrix ---
E = 3    # Embedding dimension
tau = 1  # Time delay

print("Computing the CCM causality matrix...")
# This can take a moment
causality_matrix = ccm_matrix(data, E, tau)

print("Causality Matrix (row=effect, col=cause):")
print(causality_matrix)

# --- 3. Plot the Heatmap ---
print("Plotting the heatmap...")
fig, ax = plt.subplots(figsize=(8, 6))

sns.heatmap(causality_matrix, 
            ax=ax,
            annot=True, 
            fmt=".2f", 
            cmap="viridis",
            xticklabels=series_names, 
            yticklabels=series_names)

ax.set_title("CCM Causality Analysis", fontsize=16)
ax.set_xlabel("Cause Variable", fontsize=12)
ax.set_ylabel("Effect Variable", fontsize=12)

# Adjust tick labels for clarity
ax.tick_params(axis='x', labelbottom=True, labeltop=False)
ax.tick_params(axis='y', labelleft=True, labelright=False)
plt.tight_layout()

# Save the figure
output_filename = 'ccm_heatmap.png'
plt.savefig(output_filename)
print(f"Heatmap saved as {output_filename}")

# To display the plot in an interactive session
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from ccm import ccm_matrix

# --- 1. 生成具有已知因果链的合成数据 ---
# 我们创建一个 X 驱动 Y，Y 驱动 Z (X -> Y -> Z) 的系统。

print("正在生成合成数据，因果链为 X -> Y -> Z...")
n = 1000  # 时间序列的长度

# 逻辑斯蒂映射参数
r = 3.8
beta_xy = 0.2  # 从 X 到 Y 的耦合强度
beta_yz = 0.2  # 从 Y 到 Z 的耦合强度

x = np.zeros(n)
y = np.zeros(n)
z = np.zeros(n)
x[0], y[0], z[0] = 0.4, 0.2, 0.3

# 使用稳定的加权平均方法进行耦合
for t in range(n - 1):
    fx = x[t] * (r - r * x[t])
    fy = y[t] * (r - r * y[t])
    fz = z[t] * (r - r * z[t])
    x[t+1] = fx
    y[t+1] = (1 - beta_xy) * fy + beta_xy * fx
    z[t+1] = (1 - beta_yz) * fz + beta_yz * fy

# 将时间序列合并成一个数据矩阵（每列是一个时间序列）
data = np.vstack([x, y, z]).T
series_names = ['X', 'Y', 'Z']

# --- 2. 计算 CCM 因果关系矩阵 ---
E = 3    # 嵌入维度
tau = 1  # 时间延迟

print("正在计算 CCM 因果关系矩阵...")
# 这一步计算可能需要一些时间
causality_matrix = ccm_matrix(data, E, tau)

print("因果关系矩阵 (行=结果, 列=原因):")
print(causality_matrix)

# --- 3. 绘制热力图 ---
print("正在绘制热力图...")
fig, ax = plt.subplots(figsize=(8, 6))

sns.heatmap(causality_matrix, 
            ax=ax,
            annot=True,      # 在格子里显示数字
            fmt=".2f",       # 数字格式为两位小数
            cmap="viridis",  # 设置颜色映射
            xticklabels=series_names, 
            yticklabels=series_names)

ax.set_title("CCM 因果关系分析", fontsize=16)
ax.set_xlabel("原因变量", fontsize=12)
ax.set_ylabel("结果变量", fontsize=12)

# 调整刻度标签以提高清晰度
ax.tick_params(axis='x', labelbottom=True, labeltop=False)
ax.tick_params(axis='y', labelleft=True, labelright=False)
plt.tight_layout()

# 保存图像
output_filename = 'ccm_heatmap.png'
plt.savefig(output_filename)
print(f"热力图已保存为 {output_filename}")

# 如果在交互式会话中 (例如 Jupyter Notebook)，取消下一行注释以直接显示图像
# plt.show()

