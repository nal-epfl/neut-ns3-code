# This file is for experiments done after May 2022 related to neutrality-violations localization algorithm
# It is only compatible with the files in the localization_experiments_scenarios

from helper_methods import *


# This is to specify which experiments I am currently focusing on
TEST_DATE = '10_2022'
TEST_TYPE = 'Localized_Test_Policer_Queue_Config_Burst_Based_2'


if __name__ == '__main__':
    rebuild_project()

    m_background_dir = 'chicago_2010_back_traffic_10min_control_cbp_2links'
    m_duration = 90
    m_network_setup_tag, m_network_setup = 'no_congestion', NetworkSetup('10Gbps', 'empty', '1Gbps,1Gbps')
    m_app_setup = MeasurementAppSetup(
        app_type=MeasurementAppType.INFINITE_TCP, app_name="Infinite_Paced_TCP",
        control_test_duration=m_duration, suspected_test_duration=m_duration,
        transport_protocol=TransportProtocol.TCP, tcp_protocol='TcpCubic', pkt_size=1228, app_data_rate='20Mbps')

    # for different policing rates
    m_policer_configs, m_burst_period, m_plocation, m_mtu = [], 0.02, PolicerLocation.COMMON_LINK, 1500
    for m_prate in [25, 30, 35, 45, 55]:
        #for m_queue_size in np.array([10, 20, 30, 40, 50, 100, 150, 200, 250]) * m_mtu:
        for m_queue_size in np.array([0.25, 0.5, 0.75, 1, 1.5, 2]) * m_prate * m_burst_period * 125000:
            print(m_queue_size)
            m_policer_configs.append(('shared_common_policer', m_prate, int(m_queue_size)))

    # Run experiments
    for m_seed in PRIMES[0: 10]:
        m_exp_params = []
        try:
            # test case with different policing configurations
            for m_ptype, m_prate, m_queue_size in m_policer_configs:
                m_neutrality_setup = NeutralitySetup(
                    is_neutral=1, policing_rate=m_prate, burst_length=m_burst_period, queue_size=m_queue_size,
                    policer_location=m_plocation, policer_type=PolicerType.SHARED,
                    pct_of_throttled_background="0.3,0.3"
                )
                m_exp_batch = '{}/{}_{}Mbps_{}s_{}B_30p'.format(m_network_setup_tag, m_ptype, m_prate, m_burst_period, m_queue_size)

                m_exp_params.append(ExperimentParameters(
                    exp_type='{}/{}'.format(TEST_DATE, TEST_TYPE), seed=m_seed,
                    background_setup=BackgroundTrafficSetup(m_background_dir),
                    exp_batch=m_exp_batch,
                    network_setup=m_network_setup, measurement_app_setup=m_app_setup,
                    neutrality_setup=m_neutrality_setup
                ))
        except Exception as e:
            print('seed {} failed'.format(m_seed))
        run_parallel_experiments(run_experiment_with_params, m_exp_params, nb_threads=4)