import matplotlib.pyplot as plt
import pandas as pd
import sys
import os

# 自动定位 CSV 文件路径 (假设在当前运行目录下)
csv_path = 'marc_results.csv'

if not os.path.exists(csv_path):
    print(f"Error: '{csv_path}' not found. Please run the simulation first.")
    sys.exit(1)

try:
    # 读取数据
    df = pd.read_csv(csv_path)
    print("Data loaded successfully.")
    
    # 数据转换: Bytes -> MB
    df['Size_MB'] = df['Size_Bytes'] / (1024 * 1024)
    
    # 生成易读的标签
    def get_label(x):
        if x < 1024*1024:
            return f"{x/1024:.0f}KB"
        else:
            return f"{x/(1024*1024):.0f}MB"
            
    df['Label'] = df['Size_Bytes'].apply(get_label)

    # 绘图初始化
    plt.figure(figsize=(10, 6))
    
    # 绘制 Baseline (黑色圆点)
    plt.plot(df['Size_MB'], df['Baseline_Time'], 
             marker='o', linestyle='-', color='black', 
             label='Baseline (Serial Unicast)', linewidth=2, markersize=8)
    
    # 绘制 MARC (红色方块)
    plt.plot(df['Size_MB'], df['MARC_Time'], 
             marker='s', linestyle='-', color='maroon', 
             label='MARC (Hardware Multicast)', linewidth=3, markersize=8)
    
    # 设置对数坐标 (因为差异巨大)
    plt.yscale('log')
    plt.xscale('log')
    
    # 标签与标题
    plt.xlabel('Message Size (MB)', fontsize=12)
    plt.ylabel('Transmission Time (s)', fontsize=12)
    plt.title('Figure 5 Reproduction: MARC vs Baseline', fontsize=14, fontweight='bold')
    
    # 网格与图例
    plt.grid(True, which="both", ls="-", alpha=0.2)
    plt.legend(fontsize=12)
    
    # 优化 X 轴刻度显示
    plt.xticks(df['Size_MB'], df['Label'], rotation=45)
    plt.minorticks_off()

    # 保存图片
    output_file = 'MARC_Final_Result.png'
    plt.tight_layout()
    plt.savefig(output_file, dpi=300)
    print(f"Chart generated: {output_file}")
    
except Exception as e:
    print(f"Error during plotting: {e}")
    sys.exit(1)
