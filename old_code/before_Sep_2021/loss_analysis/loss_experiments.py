import matplotlib.pyplot as plt


def compare_iid_empvar_sail(data_path, direction):
    f = open(data_path + '/compare_ci_' + direction + '.csv', 'w')

    btlk_pkts_df = read_df(data_path + '/bottleneck_packets_'+direction+'.csv')
    lbtlk_loss_vals, lbtlk_loss_ts = extract_loss_events(btlk_pkts_df)
    lbtlk_loss_ratio = to_pct(compute_loss_ratio(lbtlk_loss_vals))
    print('Bottleneck Loss Ratio: ', lbtlk_loss_ratio, file=f)

    back_pkts_df = btlk_pkts_df[btlk_pkts_df.DestinationIP == '10.1.2.1'].copy().reset_index(drop=True)
    back_loss_vals, back_loss_ts = extract_loss_events(back_pkts_df)
    back_loss_ratio = to_pct(compute_loss_ratio(back_loss_vals))
    print('Background traffic Loss Ratio: ', back_loss_ratio, file=f)

    wehe_pkts_df = btlk_pkts_df[btlk_pkts_df.DestinationIP == '10.1.1.1'].copy().reset_index(drop=True)
    p0_loss_vals, p0_loss_ts = extract_loss_events(wehe_pkts_df)
    p0_loss_ratio = to_pct(compute_loss_ratio(p0_loss_vals))
    print('Path Loss Ratio: ', p0_loss_ratio, file=f)
    p0_iid_error = to_pct(compute_ci_precision(compute_iid_ci(p0_loss_vals)))
    print('iid precision: ', p0_iid_error, file=f)
    p0_empvar_error = to_pct(compute_ci_precision(compute_empirical_variance_ci(p0_loss_vals)))
    print('empirical variance precision: ', p0_empvar_error, file=f)
    p0_sail_error = to_pct(compute_ci_precision(compute_sail_ci(p0_loss_vals, p0_loss_ts)))
    print('sail precision: ', p0_sail_error, file=f)

    fig, ax = plt.subplots(figsize=(4, 4))
    labels = ('IID', 'Emp. Var.', 'SAIL')
    index = np.arange(0, 3)

    ypoints = [p0_loss_ratio, p0_loss_ratio, p0_loss_ratio]
    yerror = [p0_iid_error, p0_empvar_error, p0_sail_error]

    ax.errorbar(index, ypoints, yerr=yerror, fmt='ko', ecolor='black')
    ax.hlines(lbtlk_loss_ratio, xmin=index[0], xmax=index[2], linestyles='dashed', label='Bottleneck Loss Ratio')
    plt.xticks(index, labels, rotation=90)

    plt.ylabel('packet loss (%)')
    plt.ylim([0, 30])
    plt.legend()
    plt.tight_layout()

    plt.savefig(data_path + '/compare_ci_' + direction + '.pdf')

    f.close()


def plot_CI_comparison(data_path, in_file):
    f = open(data_path + '/compare_ci.csv', 'w')
    df = read_df(data_path + '/' + in_file)

    base_df = df[df['DestinationIP'].isin(['10.1.1.1', '10.1.2.1'])].copy().reset_index(drop=True)
    base_loss_vals, base_loss_ts = extract_loss_events(base_df)
    base_loss_ratio = to_pct(compute_loss_ratio(base_loss_vals))
    print('Bottleneck Loss Ratio With respect to traffic type: ', base_loss_ratio, file=f)

    p1_df = base_df[base_df['DestinationIP'] == '10.1.1.1'].copy().reset_index(drop=True)
    p1_loss_ratio, p1_CIs = compute_CI_models(p1_df)
    print('Path 1 Loss Ratio: ', p1_loss_ratio, file=f)
    print('iid precision: ', p1_CIs['iid'], file=f)
    print('empirical variance precision: ', p1_CIs['empvar'], file=f)
    print('sail precision: ', p1_CIs['sail'], file=f)

    p2_df = base_df[base_df['DestinationIP'] == '10.1.2.1'].copy().reset_index(drop=True)
    p2_loss_ratio, p2_CIs = compute_CI_models(p2_df)
    print('Path 2 Loss Ratio: ', p2_loss_ratio, file=f)
    print('iid precision: ', p2_CIs['iid'], file=f)
    print('empirical variance precision: ', p2_CIs['empvar'], file=f)
    print('sail precision: ', p2_CIs['sail'], file=f)

    fig, ax = plt.subplots(figsize=(4, 4))
    labels = ('IID', 'Emp. Var.', 'SAIL')
    m_space_width = 0.1
    p_space_width = 0.4
    index = np.arange(0, np.around(3*p_space_width, decimals=2), p_space_width)

    p1_ypoints = [p1_loss_ratio, p1_loss_ratio, p1_loss_ratio]
    p2_ypoints = [p2_loss_ratio, p2_loss_ratio, p2_loss_ratio]

    ax.errorbar(index, p1_ypoints, yerr=p1_CIs.values(), fmt='ko', ecolor='black', label='path1')
    ax.errorbar(index+m_space_width, p2_ypoints, yerr=p2_CIs.values(), fmt='ko', ecolor='blue', label='path2')
    ax.hlines(base_loss_ratio, xmin=index[0]-m_space_width, xmax=index[2]+2*m_space_width,
              linestyles='dashed', colors='red', label='Bottleneck Loss Ratio w.r.t Wehe')
    plt.xticks(index + m_space_width/2, labels, rotation=90)

    plt.ylabel('packet loss (%)')
    plt.legend()
    plt.tight_layout()

    plt.savefig(data_path + '/compare_ci.pdf')

    f.close()

def plot_CI_comparison_Caida(data_path, in_file):
    f = open(data_path + '/compare_ci.csv', 'w')
    df = read_df(data_path + '/' + in_file)

    base_df = df[df['DestinationPort'].isin(['3001', '3002'])].copy().reset_index(drop=True)
    base_loss_vals, base_loss_ts = extract_loss_events(base_df)
    base_loss_ratio = to_pct(compute_loss_ratio(base_loss_vals))
    print('Bottleneck Loss Ratio With respect to traffic type: ', base_loss_ratio, file=f)

    p1_df = base_df[base_df['DestinationPort'].isin(['3001'])].copy().reset_index(drop=True)
    p1_loss_ratio, p1_CIs = compute_CI_models(p1_df)
    print('Path 1 Loss Ratio: ', p1_loss_ratio, file=f)
    print('iid precision: ', p1_CIs['iid'], file=f)
    print('empirical variance precision: ', p1_CIs['empvar'], file=f)
    print('sail precision: ', p1_CIs['sail'], file=f)

    p2_df = base_df[base_df['DestinationPort'].isin(['3002'])].copy().reset_index(drop=True)
    p2_loss_ratio, p2_CIs = compute_CI_models(p2_df)
    print('Path 2 Loss Ratio: ', p2_loss_ratio, file=f)
    print('iid precision: ', p2_CIs['iid'], file=f)
    print('empirical variance precision: ', p2_CIs['empvar'], file=f)
    print('sail precision: ', p2_CIs['sail'], file=f)

    fig, ax = plt.subplots(figsize=(4, 4))
    labels = ('IID', 'Emp. Var.', 'SAIL')
    m_space_width = 0.1
    p_space_width = 0.4
    index = np.arange(0, np.around(3*p_space_width, decimals=2), p_space_width)

    p1_ypoints = [p1_loss_ratio, p1_loss_ratio, p1_loss_ratio]
    p2_ypoints = [p2_loss_ratio, p2_loss_ratio, p2_loss_ratio]

    ax.errorbar(index, p1_ypoints, yerr=p1_CIs.values(), fmt='ko', ecolor='black', label='path1')
    ax.errorbar(index+m_space_width, p2_ypoints, yerr=p2_CIs.values(), fmt='ko', ecolor='blue', label='path2')
    ax.hlines(base_loss_ratio, xmin=index[0]-m_space_width, xmax=index[2]+2*m_space_width,
              linestyles='dashed', colors='red', label='Bottleneck Loss Ratio w.r.t Wehe')
    plt.xticks(index + m_space_width/2, labels, rotation=90)

    plt.ylabel('packet loss (%)')
    plt.legend()
    plt.tight_layout()

    plt.savefig(data_path + '/compare_ci.pdf')

    f.close()