import sys
from project_run_env.RunConfig import *

# set configurations and variables
np.set_printoptions(threshold=sys.maxsize)
# data_path = '../results/29_10_2020/caida_m0_link2.3Gbps/control_w_wehe/'#get_results_path()
data_path = '../results/22_10_2020/try_5_link_140Mbps/control_with_wehe'#get_results_path()

# build_link_packets(data_path + "/bottleneck0_packets_down.csv", data_path + "/bottleneck1_packets_down.csv", data_path + "/bottleneck_packets_down.csv")
# build_link_packets(data_path + "/bottleneck0_packets_up.csv", data_path + "/bottleneck1_packets_up.csv", data_path + "/bottleneck_packets_up.csv")

# generate_stat_filees(data_path)

# compare_iid_empvar_sail(data_path, 'up')
# compare_iid_empvar_sail(data_path, 'down')
# plt.show()


if __name__ == '__main__':
    plot_CI_comparison(data_path, 'bottleneck_packets_down.csv')
    # plot_CI_comparison_Caida(data_path, 'bottleneck_packets_up.csv')
    plt.show()