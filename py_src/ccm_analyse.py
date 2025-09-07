import pandas as pd
import numpy as np # numpy==1.23.5
import matplotlib.pyplot as plt
import os
import glob
import ccm
import seaborn as sns

def permutation_test(x, y, E=3, tau=1, num_perm=100):
    """
    置换检验，返回 (原始因果值, p值)
    """
    # 原始因果值
    rho_obs = ccm.ccm(x, y, E=E, tau=tau)

    # 置换分布
    rho_perm = []
    for _ in range(num_perm):
        x_perm = np.random.permutation(x)
        rho_perm.append(ccm.ccm(x_perm, y, E=E, tau=tau))

    rho_perm = np.array(rho_perm)
    p_value = np.mean(rho_perm >= rho_obs)  # 单尾检验
    return rho_obs, p_value

def process_stock(stock_file):
    """
    Calculates pairwise causality between each feature and the target variable for a single stock.
    Returns a dictionary of causality scores.
    """
    basename = os.path.basename(stock_file)
    code = os.path.splitext(basename)[0]
    print(f'Processing {code}')

    try:
        df = pd.read_csv(stock_file)
        if 'trade_date' not in df.columns:
            print(f"Skipping {code}: no 'trade_date' column")
            return None
    except Exception as e:
        print(f'Skipping {code}: {e}')
        return None

    df = df.fillna(0)

    x_features = ['open_qfq','high_qfq','low_qfq','pct_chg','vol','volume_ratio','pe_ttm','ps_ttm','ema_qfq_5','ema_qfq_10','ema_qfq_20','ema_qfq_30','ema_qfq_60','kdj_qfq','kdj_d_qfq','kdj_k_qfq','rsi_qfq_12','rsi_qfq_24','rsi_qfq_6','macd_qfq','macd_dea_qfq','macd_dif_qfq']
    y_feature = 'close_qfq'

    all_cols = x_features + [y_feature]
    missing_cols = [col for col in all_cols if col not in df.columns]
    if missing_cols:
        print(f"Skipping {code}: Missing columns {missing_cols}")
        return None

    causality_results = {}
    target_series = df[y_feature].values

    for feature in x_features:
        feature_series = df[feature].values

        # Calculate causality from feature to target (feature -> y_feature)
        # CCM(x, y) tests causality from x to y
        f_to_t_causality = ccm.ccm(feature_series, target_series, E=3, tau=1)
        causality_results[f'{feature} -> {y_feature}'] = f_to_t_causality

        # Calculate causality from target to feature (y_feature -> feature)
        t_to_f_causality = ccm.ccm(target_series, feature_series, E=3, tau=1)
        causality_results[f'{y_feature} -> {feature}'] = t_to_f_causality
        
    return causality_results



# 主函数
def main():
    # 配置参数
    data_dir = 'py_src/ablation_exp'  # 股票数据存放目录
    results_dir = 'CCM_Final_Analysis'  # 结果保存目录
    os.makedirs(results_dir, exist_ok=True)

    # 获取所有股票数据文件
    stock_files = glob.glob(os.path.join(data_dir, '*.csv'))
    print(f"Found {len(stock_files)} stock files in '{data_dir}'")

    all_results = {}
    # 处理每只股票
    for i, stock_file in enumerate(stock_files):
        print(f"\nProcessing {i + 1}/{len(stock_files)}: {stock_file}")
        try:
            # Get the stock code to use as a column header
            code = os.path.splitext(os.path.basename(stock_file))[0]
            result = process_stock(stock_file)
            if result:
                all_results[code] = result
        except Exception as e:
            print(f"Error processing {stock_file}: {e}")
            continue

    if not all_results:
        print("No stocks were processed successfully. Exiting.")
        return

    # Convert the dictionary of results into a DataFrame
    final_df = pd.DataFrame.from_dict(all_results).sort_index()

    # --- Split DataFrame for Two Plots ---
    y_feature_name = 'close_qfq' # The target variable name from the analysis

    # 1. Features -> Target (X -> Y)
    df_x_to_y = final_df[final_df.index.str.contains(f' -> {y_feature_name}')].copy()
    df_x_to_y.index = df_x_to_y.index.str.replace(f' -> {y_feature_name}', '').str.replace('_qfq', '')

    # 2. Target -> Features (Y -> X)
    df_y_to_x = final_df[final_df.index.str.contains(f'{y_feature_name} -> ')].copy()
    df_y_to_x.index = df_y_to_x.index.str.replace(f'{y_feature_name} -> ', '').str.replace('_qfq', '')

    # --- Save and Plot ---
    # Function to save and plot a heatmap
    def save_and_plot(df, title, filename_suffix):
        # Save data table
        csv_path = os.path.join(results_dir, f'causality_{filename_suffix}.csv')
        df.to_csv(csv_path, float_format='%.4f')
        print(f"\nCausality data saved to {csv_path}")

        # Generate heatmap
        plt.figure(figsize=(max(15, len(df.columns) * 0.6), max(10, len(df.index) * 0.5)))
        sns.heatmap(df, annot=True, cmap='coolwarm', fmt='.2f')
        plt.title(title)
        plt.xlabel('Feature')
        plt.ylabel('Stock Code')
        plt.xticks(rotation=45, ha='right')
        plt.yticks(rotation=0)
        plt.tight_layout()
        
        heatmap_path = os.path.join(results_dir, f'causality_{filename_suffix}.png')
        plt.savefig(heatmap_path)
        plt.close()
        print(f"Heatmap saved to {heatmap_path}")

    # Plot for X -> Y (Rows: Stocks, Cols: Features)
    save_and_plot(df_x_to_y.T, 'Causality: Features to Target (Close Price)', 'features_to_target')

    # Plot for Y -> X (Rows: Stocks, Cols: Features)
    save_and_plot(df_y_to_x.T, 'Causality: Target (Close Price) to Features', 'target_to_features')

    print(f"\nAll done! Processed {len(all_results)}/{len(stock_files)} stocks successfully.")
    print(f"Results are saved in '{results_dir}'")


if __name__ == '__main__':
    main()
