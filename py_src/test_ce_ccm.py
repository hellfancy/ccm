import unittest
import numpy as np
import sys
import os

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '.')))

from ce_ccm import ccm, ccm_matrix

class TestCECCM(unittest.TestCase):

    def setUp(self):
        """为所有测试统一设置测试数据和参数。"""
        np.random.seed(42)
        self.E = 3
        self.tau = 1
        self.n = 500

    def test_coupled_logistic_map(self):
        """测试 CCM 在 x 驱动 y 的耦合系统上的表现。"""
        rx = 3.8
        ry = 3.5
        beta_xy = 0.1

        x = np.zeros(self.n)
        y = np.zeros(self.n)
        x[0], y[0] = 0.4, 0.2

        for t in range(self.n - 1):
            x[t+1] = x[t] * (rx - rx * x[t])
            y[t+1] = y[t] * (ry - ry * y[t] - beta_xy * x[t])

        rho_x_xmap_y = ccm(x, y, self.E, self.tau)
        rho_y_xmap_x = ccm(y, x, self.E, self.tau)

        print(f"\n耦合系统: rho(x->y) = {rho_x_xmap_y:.4f}, rho(y->x) = {rho_y_xmap_x:.4f}")
        
        self.assertGreater(rho_x_xmap_y, 0.8, "x 预测 y 的相关性应该很高。")
        self.assertLess(rho_y_xmap_x, 0.1, "y 预测 x 的相关性应该很低。")
        self.assertTrue(rho_x_xmap_y > rho_y_xmap_x, "因果关系的方向应该是从 x 到 y。")

    def test_uncoupled_random_series(self):
        """测试 CCM 在两个独立的随机序列上的表现。"""
        x = np.random.rand(self.n)
        y = np.random.rand(self.n)

        rho = ccm(x, y, self.E, self.tau)
        print(f"非耦合系统: rho(x->y) = {rho:.4f}")
        self.assertLess(rho, 0.1, "非耦合序列的相关性应该很低。")

    def test_identical_series(self):
        """测试 CCM 在两个完全相同的序列上的表现。"""
        x = np.zeros(self.n)
        rx = 3.8
        x[0] = 0.4
        for t in range(self.n - 1):
            x[t+1] = x[t] * (rx - rx * x[t])
        
        y = x.copy()

        rho = ccm(x, y, self.E, self.tau)
        print(f"相同序列: rho(x->y) = {rho:.4f}")
        self.assertGreater(rho, 2.0, "相同序列的相关性应该很高。")

    def test_ccm_matrix(self):
        """使用已知的因果链测试 ccm_matrix 函数。"""
        n = 500
        x = np.zeros(n)
        y = np.zeros(n)
        z = np.zeros(n)
        x[0], y[0], z[0] = 0.4, 0.2, 0.3
        r = 3.8
        C_xy = 0.1
        C_yz = 0.1

        for t in range(n - 1):
            fx = x[t] * (r - r * x[t])
            fy = y[t] * (r - r * y[t])
            fz = z[t] * (r - r * z[t])
            x[t+1] = fx
            y[t+1] = (1 - C_xy) * fy + C_xy * fx
            z[t+1] = (1 - C_yz) * fz + C_yz * fy
        
        data = np.vstack([x, y, z]).T
        
        matrix = ccm_matrix(data, self.E, self.tau)

        print(f"\n因果关系矩阵:\n{matrix}")

        self.assertGreater(matrix[1, 0], 0.6, "从 0->1 的因果关系应该很强。")
        self.assertGreater(matrix[2, 1], 0.55, "从 1->2 的因果关系应该很强。")
        self.assertLess(matrix[0, 1], 0.1, "从 1->0 的因果关系应该很弱。")
        self.assertLess(matrix[1, 2], 0.1, "从 2->1 的因果关系应该很弱。")
        self.assertAlmostEqual(matrix[0, 0], 1.0)
        self.assertAlmostEqual(matrix[1, 1], 1.0)
        self.assertAlmostEqual(matrix[2, 2], 1.0)

if __name__ == '__main__':
    unittest.main()
