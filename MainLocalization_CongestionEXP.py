# This file is for experiments done after May 2022 related to neutrality-violations localization algorithm
# It is only compatible with the files in the localization_experiments_scenarios

from helper_methods import *


# This is to specify which experiments I am currently focusing on
TEST_DATE = '7_2022'
TEST_TYPE = 'Localized_Eval_NonCommon_Congestion'


if __name__ == '__main__':
    rebuild_project()

    m_background_dir = 'chicago_2010_back_traffic_10min_control_cbp_2links'
    m_duration = 30

    # use a continuous tcp flow as measurements
    m_app_setup = MeasurementAppSetup(
        app_type=MeasurementAppType.INFINITE_TCP, app_name="Infinite_Paced_TCP",
        control_test_duration=m_duration, suspected_test_duration=m_duration,
        transport_protocol=TransportProtocol.TCP, tcp_protocol='TcpCubic', pkt_size=1228, app_data_rate='20Mbps')

    # use different policing rates
    m_policer_configs, m_burst_period = [], 0.02
    for m_prate in [25, 30, 35, 45, 55]:
        m_policer_configs.append(('shared_common_policer', PolicerLocation.COMMON_LINK, m_prate))
        m_policer_configs.append(('shared_noncommon_policers', PolicerLocation.BOTH_NONCOMMON_LINKS, m_prate/2))

    # vary the location and intensity of congestion

    m_network_setups = []
    for m_link_rate in ['180Mbps', '190Mbps', '200Mbps', '210Mbps', '220Mbps']:
        m_network_setups.append((
            'Localized_Eval_Common_Congestion',
            'common_link_congestion_{}'.format(m_link_rate),
            NetworkSetup(m_link_rate, 'empty', '1Gbps,1Gbps')
        ))
    for m_link_rate in ['90Mbps', '95Mbps', '100Mbps', '105Mbps', '110Mbps']:
        m_network_setups.append((
            'Localized_Eval_NonCommon_Congestion',
            'noncommon_links_congestion_p12_{}'.format(m_link_rate),
            NetworkSetup('10Gbps', 'empty', '{},{}'.format(m_link_rate, m_link_rate))
        ))

    # Run experiments
    for m_seed in PRIMES[0: 10]:
        m_exp_params = []

        for m_test_type, m_network_setup_tag, m_network_setup in m_network_setups:

            # test case with no policing
            m_exp_params.append(ExperimentParameters(
                exp_type='{}/{}'.format(TEST_DATE, m_test_type), seed=m_seed,
                background_setup=BackgroundTrafficSetup(m_background_dir),
                exp_batch='{}/no_policing'.format(m_network_setup_tag),
                network_setup=m_network_setup, measurement_app_setup=m_app_setup,
                neutrality_setup=NeutralitySetup(is_neutral=0)
            ))

            # test case with different policing configurations
            for m_ptype, m_plocation, m_prate in m_policer_configs:
                m_neutrality_setup = NeutralitySetup(
                    is_neutral=1, policing_rate=m_prate, burst_length=m_burst_period,
                    policer_location=m_plocation, policer_type=PolicerType.SHARED,
                    pct_of_throttled_background=0.3
                )

                m_exp_params.append(ExperimentParameters(
                    exp_type='{}/{}'.format(TEST_DATE, m_test_type), seed=m_seed,
                    background_setup=BackgroundTrafficSetup(m_background_dir),
                    exp_batch='{}/{}_{}Mbps_{}s_30p'.format(m_network_setup_tag, m_ptype, m_prate, m_burst_period),
                    network_setup=m_network_setup, measurement_app_setup=m_app_setup,
                    neutrality_setup=m_neutrality_setup
                ))
        run_parallel_experiments(run_experiment_with_params, m_exp_params, nb_threads=1)
