import numpy as np

def create_time_delayed_manifold(series, E, tau):
    """
    通过时间延迟嵌入法，根据一维时间序列创建其“影子流形”。
    
    参数:
        series (np.ndarray): 输入的一维时间序列。
        E (int): 嵌入维度。
        tau (int): 时间延迟。
        
    返回:
        np.ndarray: 重建出的二维流形矩阵。
    """
    n = len(series)
    n_vectors = n - (E - 1) * tau
    if n_vectors <= 0:
        raise ValueError("时间序列对于给定的 E 和 tau 来说太短了。")
    
    manifold = np.zeros((n_vectors, E))
    for i in range(E):
        manifold[:, i] = series[i * tau : i * tau + n_vectors]
        
    return manifold

def ccm(x, y, E, tau, lib_size=None):
    """
    执行收敛交叉映射（CCM）以检验因果关系。
    此函数通过 y 的流形来预测 x (检验 x 是否“导致”y)。
    
    参数:
        x (np.ndarray): 假定的“原因”变量的时间序列。
        y (np.ndarray): 假定的“结果”变量的时间序列。
        E (int): 嵌入维度。
        tau (int): 时间延迟。
        lib_size (int, optional): 用于构建流形和预测的库的大小。如果为 None，则使用所有可能的点。
        
    返回:
        float: 预测的 x 与真实的 x 之间的相关系数 (rho)。
    """
    if len(x) != len(y):
        raise ValueError("时间序列 x 和 y 必须有相同的长度。")

    # CCM 原理: 如果 x 导致 y，那么 y 的动态中包含了 x 的信息，因此我们可以从 y 的流形中预测出 x。
    # 所以，我们用 y 构建流形，并用它来预测 x。
    
    # 1. 根据 y 创建影子流形
    manifold_y = create_time_delayed_manifold(y, E, tau)
    
    # 相应地，x 的值需要向前平移 (E-1)*tau 个单位以对齐
    offset = (E - 1) * tau
    x_target = x[offset:]
    
    if lib_size is None:
        lib_size = len(manifold_y)
        
    if lib_size > len(manifold_y):
        raise ValueError("lib_size 不能大于状态向量的总数。")

    # 使用前 `lib_size` 个点作为“库”
    library_y = manifold_y[:lib_size]
    
    # 在本实现中，待预测的点与库中的点相同
    x_actual = x_target[:lib_size]
    x_predicted = np.zeros(lib_size)

    # 2. 对库中的每个点，寻找其最近邻
    for i in range(lib_size):
        target_point = library_y[i]
        
        # 计算到库中所有其他点的欧氏距离
        distances = np.linalg.norm(library_y - target_point, axis=1)
        
        # 通过将自身距离设为无穷大来排除该点本身
        distances[i] = np.inf
        
        # 找到 E+1 个最近邻的索引
        neighbor_indices = np.argsort(distances)[:E + 1]
        
        # 3. 根据距离计算权重
        neighbor_distances = distances[neighbor_indices]
        min_dist = neighbor_distances[0]

        # 特殊处理：如果最近邻距离为0，则给予所有距离为0的邻居相同的权重
        if min_dist == 0:
            weights = np.zeros(E + 1)
            zero_dist_indices = np.where(neighbor_distances == 0)[0]
            weights[zero_dist_indices] = 1 / len(zero_dist_indices)
        else:
            # 标准权重计算方法
            weights = np.exp(-neighbor_distances / min_dist)
        
        sum_weights = np.sum(weights)
        weights = weights / sum_weights
        
        # 4. 预测 x 的值
        # 获取邻居们对应的 x 值
        x_neighbors = x_target[neighbor_indices]
        x_predicted[i] = np.sum(weights * x_neighbors)
        
    # 5. 计算预测值与真实值之间的皮尔逊相关系数
    rho = np.corrcoef(x_actual, x_predicted)[0, 1]
    
    return rho

def ccm_matrix(data, E, tau, lib_size=None):
    """
    为一组时间序列计算 CCM 因果关系矩阵。

    参数:
        data (np.ndarray): 一个二维数组，每列代表一个时间序列。
                           形状为: (样本数, 序列数)。
        E (int): 嵌入维度。
        tau (int): 时间延迟。
        lib_size (int, optional): 库的大小。默认为 None。

    返回:
        np.ndarray: 一个二维的因果关系矩阵。矩阵中位于 (j, i) 的元素代表
                    从序列 i 到序列 j 的因果关系强度。
    """
    n_samples, n_series = data.shape
    causality_matrix = np.ones((n_series, n_series))

    # i 是潜在“原因”序列的索引
    for i in range(n_series):
        # j 是潜在“结果”序列的索引
        for j in range(n_series):
            if i == j:
                continue
            
            # 测试从 i 到 j 的因果关系
            x = data[:, i]
            y = data[:, j]
            
            causality_matrix[j, i] = ccm(x, y, E, tau, lib_size)
            
    return causality_matrix
