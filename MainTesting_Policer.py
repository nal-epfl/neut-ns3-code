from helper_methods import *


# This is to specify which experiments I am currently focusing on
TEST_DATE = '7_2022'
TEST_TYPE = 'Policer_Configuration_Testing'


if __name__ == '__main__':
    rebuild_project()

    # -------------------------------------- START TESTS -------------------------------------- #
    m_background_dir = 'empty'

    # use a continuous tcp flow as measurements
    m_duration = 90
    m_app_setup = MeasurementAppSetup(
        app_type=MeasurementAppType.INFINITE_TCP, app_name="Infinite_Paced_TCP",
        control_test_duration=m_duration, suspected_test_duration=m_duration,
        transport_protocol=TransportProtocol.TCP, tcp_protocol='TcpCubic', pkt_size=1228, app_data_rate='20Mbps')

    # try clean network setup
    m_rtt = 14
    m_network_setup_tag, m_network_setup = 'no_congestion_at_all', NetworkSetup('10Gbps', '{}ms'.format((m_rtt-4)/2), '1Gbps')

    # try different policer configuration
    m_policer_configs = []
    for m_prate in [1, 5, 15]:
        m_policer_configs.append(('shared_common_policer_queue_1MTU_shi', NeutralitySetup(
            is_neutral=1, policing_rate=m_prate, burst_length=m_rtt/1e3, policer_location=PolicerLocation.COMMON_LINK,
            policer_type=PolicerType.SHARED, pct_of_throttled_background=0.3
        )))

    # Run all different  policer configurations
    for m_seed in [3]:
        m_exp_params = []
        for m_policer_tag, m_policer_config in m_policer_configs:
            m_exp_batch = '{}/{}_{}Mbps_{}s'.format(
                    m_network_setup_tag, m_policer_tag, m_policer_config.policing_rate, m_policer_config.burst_length
            )
            m_exp_params.append(ExperimentParameters(
                exp_type='{}/{}'.format(TEST_DATE, TEST_TYPE),
                network_setup=m_network_setup, measurement_app_setup=m_app_setup, background_dir=m_background_dir,
                neutrality_setup=m_policer_config, exp_batch=m_exp_batch, seed=m_seed)
            )
        run_parallel_experiments(run_experiment_with_params, m_exp_params, nb_threads=1)