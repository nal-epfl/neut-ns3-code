# This file is for experiments done after May 2022 related to neutrality-violations localization algorithm
# It is only compatible with the files in the localization_experiments_scenarios

from helper_methods import *


# This is to specify which experiments I am currently focusing on
TEST_DATE = '7_2022'
TEST_TYPE = 'Localized_Eval_Mixed'


if __name__ == '__main__':
    rebuild_project()

    m_background_dir = 'chicago_2010_back_traffic_10min_control_cbp_2links'
    m_duration = 90

    # use a continuous tcp flow as measurements
    m_app_setup = MeasurementAppSetup(
        app_type=MeasurementAppType.INFINITE_TCP, app_name="Infinite_Paced_TCP",
        control_test_duration=m_duration, suspected_test_duration=m_duration,
        transport_protocol=TransportProtocol.TCP, tcp_protocol='TcpCubic', pkt_size=1228, app_data_rate='20Mbps')

    # try different network setups
    m_network_setups = [
        ('no_congestion_at_all', NetworkSetup('10Gbps', 'empty', '1Gbps,1Gbps')),

        ('no_congestion_at_all_p2_d2ms', NetworkSetup('10Gbps', '5ms,2ms', '1Gbps,1Gbps')),
        ('no_congestion_at_all_p2_d15ms', NetworkSetup('10Gbps', '5ms,15ms', '1Gbps,1Gbps')),

        ('congestion_on_noncommon_links_p12_r90Mbps', NetworkSetup('10Gbps', 'empty', '90Mbps,90Mbps')),
        ('congestion_on_noncommon_links_p12_r100Mbps', NetworkSetup('10Gbps', 'empty', '100Mbps,100Mbps')),
        ('congestion_on_noncommon_links_p1_r90Mbps_p2_r100Mbps', NetworkSetup('10Gbps', 'empty', '90Mbps,100Mbps')),

        ('congestion_on_common_link', NetworkSetup('180Mbps', 'empty', '1Gbps,1Gbps')),
        ('congestion_on_common_link', NetworkSetup('200Mbps', 'empty', '1Gbps,1Gbps')),
        ('congestion_on_common_link', NetworkSetup('220Mbps', 'empty', '1Gbps,1Gbps')),

        ('congestion_on_common_link_p2_d2ms', NetworkSetup('180Mbps', '5ms,2ms', '1Gbps,1Gbps')),
        ('congestion_on_common_link_p2_d15ms', NetworkSetup('180Mbps', '5ms,15ms', '1Gbps,1Gbps')),
        ('congestion_on_all_links_p12_r95Mbps', NetworkSetup('180Mbps', 'empty', '95Mbps,95Mbps')),
        ('congestion_on_all_links_p12_r100Mbps', NetworkSetup('180Mbps', 'empty', '100Mbps,100Mbps')),
        ('congestion_on_all_links_p1_r100Mbps_p2_r95Mbps', NetworkSetup('180Mbps', 'empty', '100Mbps,95Mbps')),
        ('congestion_on_all_links_p1_r95Mbps_p2_r100Mbps_d15ms', NetworkSetup('180Mbps', '5ms,15ms', '95Mbps,100Mbps')),

        ('congestion_on_common_link_p2_d2ms', NetworkSetup('200Mbps', '5ms,2ms', '1Gbps,1Gbps')),
        ('congestion_on_common_link_p2_d15ms', NetworkSetup('200Mbps', '5ms,15ms', '1Gbps,1Gbps')),
        ('congestion_on_all_links_p12_r95Mbps', NetworkSetup('200Mbps', 'empty', '95Mbps,95Mbps')),
        ('congestion_on_all_links_p12_r100Mbps', NetworkSetup('200Mbps', 'empty', '100Mbps,100Mbps,100Mbps')),
        ('congestion_on_all_links_p1_r100Mbps_p2_r95Mbps', NetworkSetup('200Mbps', 'empty', '100Mbps,95Mbps')),
        ('congestion_on_all_links_p1_r95Mbps_p2_r100Mbps_d15ms', NetworkSetup('200Mbps', '5ms,15ms', '95Mbps,100Mbps')),
    ]

    # try different policer configuration
    m_policer_configs = []
    for m_prate in [25, 28, 30, 35, 40]:
        m_policer_configs.append(('shared_common_policer', NeutralitySetup(
            is_neutral=1, policing_rate=m_prate, burst_length=0.03, policer_location=PolicerLocation.COMMON_LINK,
            policer_type=PolicerType.SHARED, pct_of_throttled_background="0.3,0.3"
        )))
    for m_prate in [13, 14, 15, 18, 20]:
        m_policer_configs.append(('shared_noncommon_policers', NeutralitySetup(
            is_neutral=1, policing_rate=m_prate, burst_length=0.03, policer_location=PolicerLocation.BOTH_NONCOMMON_LINKS,
            policer_type=PolicerType.SHARED, pct_of_throttled_background="0.3,0.3"
        )))

    # Run all different combinations of network setups and policer configurations
    for m_seed in PRIMES[0: 1]:
        m_exp_params = []
        for m_network_setup_tag, m_network_setup in m_network_setups:
            for m_policer_tag, m_policer_config in m_policer_configs:
                m_exp_batch = '{}/{}_{}Mbps_{}s_30p'.format(
                    m_network_setup_tag, m_policer_tag, m_policer_config.policing_rate, m_policer_config.burst_length
                )
                m_exp_params.append(ExperimentParameters(
                    exp_type='{}/{}'.format(TEST_DATE, TEST_TYPE),
                    network_setup=m_network_setup, measurement_app_setup=m_app_setup,
                    background_setup=BackgroundTrafficSetup(m_background_dir),
                    neutrality_setup=m_policer_config, exp_batch=m_exp_batch, seed=m_seed)
                )
        run_parallel_experiments(run_experiment_with_params, m_exp_params, nb_threads=1)