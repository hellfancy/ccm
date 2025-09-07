# Python 实现的收敛交叉映射 (CCM) 算法

本项目是收敛交叉映射（Convergent Cross Mapping, CCM）算法的纯 Python 实现，主要依赖于 NumPy 库。

CCM 是一种基于状态空间重构的强大方法，用于检测动态系统中变量之间的因果关系。其核心思想是：如果变量 X 对变量 Y 有因果影响，那么 Y 的历史信息中必然会包含 X 的痕迹。因此，我们可以通过 Y 的时间序列重构出的“影子流形”来预测 X 的值。

## 依赖要求

- Python 3.x
- NumPy

您可以使用 pip 安装 NumPy：
```bash
pip install numpy
```

## 如何使用

将本目录下的 `ccm.py` 文件复制到您的项目中，然后导入 `ccm` 函数。

### 函数签名

```python
ccm(x, y, E, tau, lib_size=None)
```

#### 参数:
- `x` (`np.ndarray`): 假定的 **原因** 变量的时间序列 (1D array)。
- `y` (`np.ndarray`): 假定的 **结果** 变量的时间序列 (1D array)。
- `E` (`int`): 嵌入维度，即用于重构状态空间向量的坐标数量。
- `tau` (`int`): 时间延迟。
- `lib_size` (`int`, optional): 用于预测的库的大小。如果为 `None`，则使用所有可用的数据点。

#### 返回值:
- `float`: 返回一个相关系数 (rho)。这个值衡量了基于 `y` 的流形预测出的 `x` 与真实的 `x` 之间的吻合程度。值越接近 1，表明 `x` 对 `y` 的因果关系越强。

### 示例代码

```python
import numpy as np
from ccm import ccm

# 1. 创建一个 x 驱动 y 的耦合系统（逻辑斯蒂映射）
n = 500
x = np.zeros(n)
y = np.zeros(n)
x[0], y[0] = 0.4, 0.2

for t in range(n - 1):
    x[t+1] = x[t] * (3.8 - 3.8 * x[t])
    y[t+1] = y[t] * (3.5 - 3.5 * y[t] - 0.1 * x[t])

# 2. 定义嵌入参数
E = 3
tau = 1

# 3. 计算 x 对 y 的因果性
#    根据 CCM 原理，我们用 y 的流形来预测 x
rho_x_to_y = ccm(x, y, E, tau)

# 4. 作为对比，计算 y 对 x 的因果性 (预期较弱)
#    我们用 x 的流形来预测 y
rho_y_to_x = ccm(y, x, E, tau)

print(f"检测到 x 对 y 的因果性 (rho): {rho_x_to_y:.4f}")
print(f"检测到 y 对 x 的因果性 (rho): {rho_y_to_x:.4f}")

# 预期输出:
# 检测到 x 对 y 的因果性 (rho): 0.9278
# 检测到 y 对 x 的因果性 (rho): 0.1920
```

## 运行测试

本项目包含一套单元测试 (`test_ccm.py`)，以验证算法的正确性。要运行测试，请在项目的根目录 (`ccm`) 下执行以下命令：

```bash
python -m unittest py_src/test_ccm.py
```

如果实现正确，所有测试都应该通过。
