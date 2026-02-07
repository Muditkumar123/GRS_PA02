# Roll No: MT25073
# File: MT25073_Part_D_Plots.py
# Description: Generates ALL required comprehensive subplots using HARDCODED data.

import matplotlib.pyplot as plt
import numpy as np

# ==========================================
# HARDCODED DATA (Extracted from your CSV)
# ==========================================

sizes_kb = [1, 32, 128, 1024]
threads = [1, 2, 4, 8]

# --- 1. THROUGHPUT DATA (Gbps) ---
# Format: [1KB, 32KB, 128KB, 1MB]
tp_two_t1 = [6.57, 33.27, 39.54, 45.63]
tp_two_t2 = [10.75, 51.59, 67.35, 71.42]
tp_two_t4 = [13.06, 78.33, 93.21, 59.71]
tp_two_t8 = [13.91, 82.20, 83.75, 41.25]

tp_one_t1 = [5.48, 40.22, 49.04, 48.96]
tp_one_t2 = [9.81, 65.24, 79.50, 73.99]
tp_one_t4 = [12.68, 86.92, 97.00, 55.56]
tp_one_t8 = [13.24, 87.40, 82.36, 48.85]

tp_zero_t1 = [1.23, 20.10, 34.53, 38.58]
tp_zero_t2 = [2.15, 32.10, 58.76, 67.41]
tp_zero_t4 = [3.16, 48.06, 79.11, 81.61]
tp_zero_t8 = [3.34, 55.87, 76.29, 40.39]

# --- 2. LATENCY DATA (microseconds) ---
# Format: [Thread1, Thread2, Thread4, Thread8]
lat_two_1k = [1.24, 0.76, 0.62, 0.58]
lat_two_32k = [7.87, 5.08, 3.34, 3.18]
lat_two_128k = [26.51, 15.56, 11.25, 12.52]
lat_two_1mb = [183.84, 117.45, 140.48, 203.35]

lat_one_1k = [1.49, 0.83, 0.64, 0.61]
lat_one_32k = [6.51, 4.01, 3.01, 2.99]
lat_one_128k = [21.38, 13.18, 10.81, 12.73]
lat_one_1mb = [171.32, 113.37, 150.96, 171.72]

lat_zero_1k = [6.63, 3.81, 2.59, 2.45]
lat_zero_32k = [13.03, 8.16, 5.45, 4.69]
lat_zero_128k = [30.36, 17.84, 13.25, 13.74]
lat_zero_1mb = [217.44, 124.43, 102.78, 207.66]

# --- 3. RAW CYCLES DATA (For Efficiency Calculation) ---
# Format: [1KB, 32KB, 128KB, 1MB]
# I extracted these manually from your CSV for ALL threads.

# TwoCopy Cycles
cyc_two_t1 = [15073965817, 17904004913, 18240266365, 18453193248]
cyc_two_t2 = [33992460383, 32940938262, 34136797066, 33464440564]
cyc_two_t4 = [60770988973, 58780517175, 58155996712, 63592121557]
cyc_two_t8 = [65752536870, 68919139793, 69636277388, 94581164504]

# OneCopy Cycles
cyc_one_t1 = [18698380139, 18162125586, 18487106927, 17277769418]
cyc_one_t2 = [34218636557, 32906946512, 33051614689, 32504435226]
cyc_one_t4 = [65747746552, 59936550632, 56409000308, 52054915268]
cyc_one_t8 = [72201999000, 64554221068, 60948102791, 60562707243]

# ZeroCopy Cycles
cyc_zero_t1 = [18006355019, 18113528493, 18785648751, 14715863848]
cyc_zero_t2 = [33479100012, 33697395614, 33547156022, 33275294050]
cyc_zero_t4 = [59530465546, 58982283606, 59805457743, 49065540505]
cyc_zero_t8 = [73934434652, 71341924277, 41013029141, 16361841639]

# --- 4. CACHE MISS DATA (Millions) - Thread=1 Only ---
miss_two = [173.6, 1967.4, 2214.8, 2501.9]
miss_one = [180.2, 1497.6, 1773.0, 1726.5]
miss_zero = [186.8, 704.5, 1147.3, 1081.1]

# ==========================================
# HELPER FUNCTIONS
# ==========================================

def calc_cpb(cycles_arr, tp_arr):
    # Calculates CPU Cycles Per Byte
    # Formula: Cycles / (Throughput_Gbps * 1e9 / 8 * 5 seconds)
    cpb = []
    for cyc, tp in zip(cycles_arr, tp_arr):
        if tp == 0: cpb.append(0)
        else:
            total_bytes = (tp * 1e9 / 8) * 5
            cpb.append(cyc / total_bytes)
    return cpb

def plot_setup():
    # --- FIGURE 1: THROUGHPUT GRID (2x2) ---
    fig1, axs1 = plt.subplots(2, 2, figsize=(14, 10))
    fig1.suptitle('Throughput vs Message Size (Across Thread Counts)', fontsize=16)

    def plot_grid(ax, t2, t1, t0, x_vals, x_labels, title, y_label):
        ax.plot(x_vals, t2, 'o-', label='TwoCopy', color='tab:blue')
        ax.plot(x_vals, t1, 's-', label='OneCopy', color='tab:orange')
        ax.plot(x_vals, t0, '^-', label='ZeroCopy', color='tab:green')
        if x_labels:
            ax.set_xscale('log')
            ax.set_xticks(x_vals)
            ax.set_xticklabels(x_labels)
        else:
            ax.set_xticks(x_vals)
        ax.set_title(title)
        ax.set_ylabel(y_label)
        ax.grid(True)
        ax.legend()

    plot_grid(axs1[0, 0], tp_two_t1, tp_one_t1, tp_zero_t1, sizes_kb, ['1K','32K','128K','1MB'], 'Threads = 1', 'Throughput (Gbps)')
    plot_grid(axs1[0, 1], tp_two_t2, tp_one_t2, tp_zero_t2, sizes_kb, ['1K','32K','128K','1MB'], 'Threads = 2', 'Throughput (Gbps)')
    plot_grid(axs1[1, 0], tp_two_t4, tp_one_t4, tp_zero_t4, sizes_kb, ['1K','32K','128K','1MB'], 'Threads = 4', 'Throughput (Gbps)')
    plot_grid(axs1[1, 1], tp_two_t8, tp_one_t8, tp_zero_t8, sizes_kb, ['1K','32K','128K','1MB'], 'Threads = 8', 'Throughput (Gbps)')
    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.savefig('MT25073_Throughput_Grid.png')

    # # --- FIGURE 2: SCALING GRID (Throughput vs Thread Count) ---
    # # Rearranging data to plot vs Threads
    # fig2, axs2 = plt.subplots(2, 2, figsize=(14, 10))
    # fig2.suptitle('Throughput Scaling vs Thread Count', fontsize=16)

    # # Helper to slice data by Index (0=1K, 1=32K, 2=128K, 3=1MB)
    # def get_scaling_data(idx, arr1, arr2, arr4, arr8):
    #     return [arr1[idx], arr2[idx], arr4[idx], arr8[idx]]

    # for i, (size_label, idx) in enumerate(zip(['1KB', '32KB', '128KB', '1MB'], [0, 1, 2, 3])):
    #     row, col = i // 2, i % 2
    #     d2 = get_scaling_data(idx, tp_two_t1, tp_two_t2, tp_two_t4, tp_two_t8)
    #     d1 = get_scaling_data(idx, tp_one_t1, tp_one_t2, tp_one_t4, tp_one_t8)
    #     d0 = get_scaling_data(idx, tp_zero_t1, tp_zero_t2, tp_zero_t4, tp_zero_t8)
    #     plot_grid(axs2[row, col], d2, d1, d0, threads, None, f'Msg Size = {size_label}', 'Throughput (Gbps)')

    # plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    # plt.savefig('MT25073_Scaling_Grid.png')

    # --- FIGURE 3: LATENCY GRID (2x2) ---
    fig3, axs3 = plt.subplots(2, 2, figsize=(14, 10))
    fig3.suptitle('Latency vs Thread Count', fontsize=16)
    plot_grid(axs3[0, 0], lat_two_1k, lat_one_1k, lat_zero_1k, threads, None, 'Msg Size = 1KB', 'Latency (us)')
    plot_grid(axs3[0, 1], lat_two_32k, lat_one_32k, lat_zero_32k, threads, None, 'Msg Size = 32KB', 'Latency (us)')
    plot_grid(axs3[1, 0], lat_two_128k, lat_one_128k, lat_zero_128k, threads, None, 'Msg Size = 128KB', 'Latency (us)')
    plot_grid(axs3[1, 1], lat_two_1mb, lat_one_1mb, lat_zero_1mb, threads, None, 'Msg Size = 1MB', 'Latency (us)')
    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.savefig('MT25073_Latency_Grid.png')

    # --- FIGURE 4: EFFICIENCY GRID (CPB vs Msg Size) ---
    # Calculate CPB for all
    fig4, axs4 = plt.subplots(2, 2, figsize=(14, 10))
    fig4.suptitle('CPU Cycles Per Byte (Efficiency) vs Msg Size', fontsize=16)

    def plot_cpb_subplot(ax, cyc, tp, title):
        c2 = calc_cpb(cyc[0], tp[0])
        c1 = calc_cpb(cyc[1], tp[1])
        c0 = calc_cpb(cyc[2], tp[2])
        plot_grid(ax, c2, c1, c0, sizes_kb, ['1K','32K','128K','1MB'], title, 'Cycles/Byte')

    plot_cpb_subplot(axs4[0,0], [cyc_two_t1, cyc_one_t1, cyc_zero_t1], [tp_two_t1, tp_one_t1, tp_zero_t1], 'Threads = 1')
    plot_cpb_subplot(axs4[0,1], [cyc_two_t2, cyc_one_t2, cyc_zero_t2], [tp_two_t2, tp_one_t2, tp_zero_t2], 'Threads = 2')
    plot_cpb_subplot(axs4[1,0], [cyc_two_t4, cyc_one_t4, cyc_zero_t4], [tp_two_t4, tp_one_t4, tp_zero_t4], 'Threads = 4')
    plot_cpb_subplot(axs4[1,1], [cyc_two_t8, cyc_one_t8, cyc_zero_t8], [tp_two_t8, tp_one_t8, tp_zero_t8], 'Threads = 8')
    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.savefig('MT25073_Efficiency_Grid.png')

    # --- FIGURE 5: CACHE MISSES (Single Plot) ---
    plt.figure(figsize=(8, 6))
    x = np.arange(len(sizes_kb))
    width = 0.25
    plt.bar(x - width, miss_two, width, label='TwoCopy')
    plt.bar(x, miss_one, width, label='OneCopy')
    plt.bar(x + width, miss_zero, width, label='ZeroCopy')
    plt.xticks(x, ['1K', '32K', '128K', '1MB'])
    plt.ylabel('L1 Cache Misses (Millions)')
    plt.title('L1 Cache Misses (Threads=1)')
    plt.legend()
    plt.grid(True, axis='y')
    plt.savefig('MT25073_Cache_Misses.png')

    print("Generated 5 images: Throughput, Scaling, Latency, Efficiency, Cache_Misses")
    plt.show()

if __name__ == "__main__":
    plot_setup()