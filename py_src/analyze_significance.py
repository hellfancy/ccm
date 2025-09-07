import pandas as pd
import os

from matplotlib import pyplot as plt
import seaborn as sns

def visualize_significance(results_dir="CCM_Final_Analysis", quantile=0.75):
    # 读取结果
    path = os.path.join(results_dir, "causality_features_to_target.csv")
    df = pd.read_csv(path, index_col=0)

    # -------- 原始因果值热力图 --------
    plt.figure(figsize=(max(15, df.shape[1]*0.6), max(10, df.shape[0]*0.5)))
    sns.heatmap(df, annot=False, cmap="coolwarm", center=0, cbar_kws={"label": "Causality (ρ)"})
    plt.title("Causality (Features -> Target)")
    plt.xlabel("Stock Code")
    plt.ylabel("Feature")
    plt.tight_layout()
    plt.savefig(os.path.join(results_dir, "heatmap_raw.png"))
    plt.close()

    # -------- 自动显著性阈值 --------
    threshold = df.values.flatten()
    threshold = pd.Series(threshold).quantile(quantile)
    print(f"Using ρ threshold = {threshold:.3f} (quantile={quantile})")

    # -------- 显著性分布直方图 --------
    plt.figure(figsize=(10, 6))
    sns.histplot(df.values.flatten(), bins=30, kde=True, color='skyblue')
    plt.axvline(threshold, color='red', linestyle='--', label=f'Threshold ({threshold:.3f})')
    plt.title("Distribution of Causality Values (ρ)")
    plt.xlabel("ρ")
    plt.ylabel("Frequency")
    plt.legend()
    plt.tight_layout()
    plt.savefig(os.path.join(results_dir, "causality_distribution.png"))
    plt.close()
    print(f"✅ Causality distribution histogram saved to causality_distribution.png")
    # -------- 显著性二值化热力图 --------
    sig_mask = (df > threshold).astype(int)

    plt.figure(figsize=(max(15, df.shape[1]*0.6), max(10, df.shape[0]*0.5)))
    sns.heatmap(sig_mask, annot=True, cmap="YlGnBu", cbar=False)
    plt.title(f"Significant Causality (ρ > {threshold})")
    plt.xlabel("Stock Code")
    plt.ylabel("Feature")
    plt.tight_layout()
    plt.savefig(os.path.join(results_dir, f"heatmap_significant_{threshold}.png"))
    plt.close()

    print(f"✅ Raw heatmap saved to heatmap_raw.png")
    print(f"✅ Significant heatmap saved to heatmap_significant_{threshold}.png")

if __name__ == "__main__":
    visualize_significance()
