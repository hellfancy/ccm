# 收敛交叉映射（CCM）算法的北太天元插件

本项目是收敛交叉映射（Convergent Cross Mapping, CCM）算法的 C++ 实现，并封装为北太天元（Baltamatica）的插件。CCM 是一种用于检测时间序列之间因果关系的方法。

## 功能

- 实现了 CCM 算法的核心逻辑。
- 作为北太天元插件，可以直接在北太天元环境中调用，利用 C++ 的高性能进行计算。

## 环境要求

在编译和使用本插件之前，请确保您的系统满足以下要求：

1.  **C++ 编译器**: 支持 C++17 标准的编译器 (例如 GCC 8.0 或更高版本)。
2.  **CMake**: 版本 3.20 或更高。
3.  **北太天元 (Baltamatica)**: 需要预先安装，并且 **必须** 安装在 `/opt/Baltamatica` 目录下，因为项目配置写定了该路径。
4.  **openBLAS**: 项目依赖于 openBLAS 库，请确保已正确安装。

## 编译指南

您可以使用 CMake 来编译生成插件。

1.  **克隆或下载项目**

2.  **创建 build 目录**:
    ```bash
    mkdir build
    cd build
    ```

3.  **运行 CMake 和 Make**:
    ```bash
    # 生成 Makefile
    cmake ..

    # 编译项目
    make
    ```

4.  **编译产物**:
    编译成功后，会在 `build` 目录下生成插件文件（例如，在 Linux 上是 `main.so`，在 Windows 上是 `main.dll`）。

## 使用方法

1.  **启动北太天元**。

2.  **添加路径**:
    将本项目 `build` 目录的绝对路径添加到北太天元的搜索路径中。
    ```
    addpath('/path/to/your/project/ccm/build')
    ```

3.  **调用插件**:
    根据 `src/main.cpp` 的定义，该插件注册了一个名为 `ccm` 的函数。它需要两个输入参数和一个输出参数。

    **函数签名**:
    ```
    [rho] = ccm(timeseries1, timeseries2)
    ```

    - `timeseries1`: 第一个时间序列（行向量）。
    - `timeseries2`: 第二个时间序列（行向量）。
    - `rho`: 返回的交叉映射技巧（correlation coefficient），一个数值。

    **示例 (假设)**:
    ```
    % 假设 x 和 y 是两个长度相同的时间序列行向量
    x = 1:100;
    y = sin(x/10);

    % 调用 ccm 函数
    rho = ccm(x, y);

    % 显示结果
    disp(rho);
    ```
    **注意**: 上述示例是基于对 CCM 算法的通用理解，具体输入输出的数据结构和含义请参考 `src/ccmth.cpp` 的实现。

## 注意事项

- 当前项目的 `CMakeLists.txt` 中硬编码了北太天元的头文件和库文件路径 (`/opt/Baltamatica`)。如果您的安装路径不同，请相应修改 `CMakeLists.txt` 文件。
- 插件的帮助文档尚未完善。