import numpy as np
from copent import copent
from scipy.spatial import cKDTree

class CE_CCM:
    def __init__(self, E=2, tau=1, lib_sizes=None, n_samples=5, replace=False, random_seed=None, k=5, metric="chebyshev"):
        self.E = E
        self.tau = tau
        self.lib_sizes = lib_sizes
        self.n_samples = n_samples
        self.replace = replace
        self.random_seed = random_seed
        self.k = k
        self.metric = metric

    def _make_embedding(self, ts):
        """Construct time-delayed embedding vectors"""
        N = len(ts) - (self.E - 1) * self.tau
        if N <= 0:
            raise ValueError("Time series is too short for the given E and tau.")
        return np.array([ts[i:(i + self.E * self.tau):self.tau] for i in range(N)]), ts[(self.E - 1) * self.tau:]

    def _crossmap(self, src_emb, tgt):
        """Predict tgt using src_emb"""
        tree = cKDTree(src_emb)
        dists, idxs = tree.query(src_emb, k=self.E + 1)
        dists = dists[:, 1:]
        idxs = idxs[:, 1:]
        weights = np.exp(-dists / (dists[:, 0][:, None] + 1e-12))
        weights /= weights.sum(axis=1, keepdims=True)
        pred = np.sum(weights * tgt[idxs], axis=1)
        return pred

    def _mi_skill(self, pred, true):
        """Mutual information (estimated via copula entropy)"""
        arr = np.column_stack((pred, true))
        k = min(self.k, len(pred) - 1)
        if k <= 0:
            return 0.0
        mi = copent(arr, k=k, dtype=self.metric)
        return max(0.0, float(mi))

    def causality(self, X, Y, lib_size=None):
        """
        Copula Entropy based Convergent Cross Mapping (CE-CCM)
        Tests for causality from X to Y.
        """
        if self.random_seed is not None:
            np.random.seed(self.random_seed)

        X = np.asarray(X)
        Y = np.asarray(Y)

        try:
            X_emb, X_target = self._make_embedding(X)
            Y_emb, Y_target = self._make_embedding(Y)
        except ValueError as e:
            print(f"Error making embedding: {e}")
            return 0.0
            
        N = len(X_target)

        if lib_size is not None:
            lib_sizes_to_use = [lib_size]
        elif self.lib_sizes is not None:
            lib_sizes_to_use = self.lib_sizes
        else:
            lib_sizes_to_use = np.arange(self.E + 1, N, max(1, N // 20))

        all_skills = []

        for L in lib_sizes_to_use:
            if L > N:
                continue
            skills = []
            for _ in range(self.n_samples):
                idx = np.random.choice(N, L, replace=self.replace)
                pred_X = self._crossmap(Y_emb[idx], X_target[idx])
                true_X = X_target[idx[:len(pred_X)]]
                skill = self._mi_skill(pred_X, true_X)
                skills.append(skill)
            all_skills.extend(skills)
        
        return np.mean(all_skills) if all_skills else 0.0

def ccm(X, Y, E=2, tau=1, lib_size=None):
    ce_ccm_instance = CE_CCM(E=E, tau=tau)
    return ce_ccm_instance.causality(X, Y, lib_size=lib_size)

def ccm_matrix(data, E=2, tau=1, lib_size=None):
    num_series = data.shape[1]
    matrix = np.ones((num_series, num_series))
    for i in range(num_series):
        for j in range(num_series):
            if i == j:
                continue
            matrix[j, i] = ccm(data[:, i], data[:, j], E, tau, lib_size=lib_size)
    return matrix