import numpy as np
from copent import copent
from scipy.spatial import cKDTree

def _make_embedding(ts, E, tau):
    """构造时间延迟嵌入向量"""
    N = len(ts) - (E - 1) * tau
    return np.array([ts[i:(i + E * tau):tau] for i in range(N)]), ts[(E - 1) * tau:]

def _crossmap(src_emb, tgt, E):
    """用 src_emb 预测 tgt"""
    tree = cKDTree(src_emb)
    dists, idxs = tree.query(src_emb, k=E + 1)
    dists = dists[:, 1:]  # 去掉自己
    idxs = idxs[:, 1:]
    weights = np.exp(-dists / (dists[:, 0][:, None] + 1e-12))
    weights /= weights.sum(axis=1, keepdims=True)
    pred = np.sum(weights * tgt[idxs], axis=1)
    return pred

def _mi_skill(pred, true, k=5, metric="chebyshev"):
    """互信息 (基于 copula 熵估计)"""
    arr = np.column_stack((pred, true))
    mi = copent(arr, k=k, dtype=metric)
    return max(0.0, float(mi))

def ce_ccm(X, Y, E=2, tau=1, lib_sizes=None,
           n_samples=100, replace=False, random_seed=None,
           k=5, metric="chebyshev"):
    """
    Copula Entropy based Convergent Cross Mapping (CE-CCM)

    参数
    ----
    X, Y : array-like
        两个时间序列
    E : int
        embedding dimension
    tau : int
        time delay
    lib_sizes : list[int]
        library sizes to evaluate
    n_samples : int
        每个库长度下的重复采样数
    replace : bool
        是否有放回采样
    random_seed : int
        随机数种子
    k : int
        copent 中 kNN 参数
    metric : str
        copent 距离度量 ('chebyshev' 或 'euclidean')

    返回
    ----
    skills : dict[int -> list[float]]
        每个库长度对应的互信息 skill 数组
    """
    if random_seed is not None:
        np.random.seed(random_seed)

    X = np.asarray(X)
    Y = np.asarray(Y)

    X_emb, X_target = _make_embedding(X, E, tau)
    Y_emb, Y_target = _make_embedding(Y, E, tau)
    N = len(X_target)

    if lib_sizes is None:
        lib_sizes = np.arange(E + 1, N, max(1, N // 20))

    results = {}

    for L in lib_sizes:
        skills = []
        for _ in range(n_samples):
            idx = np.random.choice(N, L, replace=replace)
            pred_X = _crossmap(Y_emb[idx], X_target[idx], E)
            true_X = X_target[idx[:len(pred_X)]]
            skill = _mi_skill(pred_X, true_X, k=k, metric=metric)
            skills.append(skill)
        results[L] = skills

    return results

if __name__ == '__main__':
    import matplotlib.pyplot as plt
    # ====== 构造一个简单的例子 ======
    # Y 驱动 X (非对称因果关系)
    T = 500
    time = np.arange(T)
    Y = np.sin(0.05 * time) + 0.1 * np.random.randn(T)
    X = np.roll(Y, 3) + 0.2 * np.random.randn(T)  # X 依赖于滞后 Y

    # ====== 参数设置 ======
    E = 3
    tau = 1
    lib_sizes = np.arange(50, 300, 50)
    n_samples = 20

    # ====== 计算 CE-CCM ======
    ce_results = ce_ccm(X, Y, E=E, tau=tau,
                        lib_sizes=lib_sizes, n_samples=n_samples,
                        k=5, metric="chebyshev")

    # ====== 可视化对比 ======
    # pearson_means = [np.mean(pearson_results[L]) for L in lib_sizes]
    # pearson_stds = [np.std(pearson_results[L]) for L in lib_sizes]
    ce_means = [np.mean(ce_results[L]) for L in lib_sizes]
    ce_stds = [np.std(ce_results[L]) for L in lib_sizes]

    plt.figure(figsize=(8, 5))
    # plt.errorbar(lib_sizes, pearson_means, yerr=pearson_stds, label="Pearson-CCM", fmt='-o')
    plt.errorbar(lib_sizes, ce_means, yerr=ce_stds, label="CE-CCM (Copula Entropy)", fmt='-s')
    plt.xlabel("Library Size")
    plt.ylabel("Skill")
    plt.title("CCM vs CE-CCM")
    plt.legend()
    plt.grid(True)
    plt.show()