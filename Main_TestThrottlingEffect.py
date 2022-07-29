# This file is for experiments done after May 2022 related to neutrality-violations localization algorithm
# It is only compatible with the files in the localization_experiments_scenarios

from helper_methods import *


# This is to specify which experiments I am currently focusing on
TEST_DATE = '7_2022'
TEST_TYPE = 'Test_Throttling_Effect'


if __name__ == '__main__':
    rebuild_project()

    m_background_dir = 'chicago_2010_back_traffic_10min_control_cbp_2links'
    m_duration = 60
    m_network_setup_tag, m_network_setup = 'no_congestion', NetworkSetup('10Gbps', 'empty', '1Gbps,1Gbps')

    # for different policing rates
    m_policer_configs, m_burst_period = [], 0.02
    for m_prate in [25, 28, 30, 32, 34, 36, 38, 40, 42, 44]:
        m_policer_configs.append(('shared_common_policer', PolicerLocation.COMMON_LINK, m_prate))
        m_policer_configs.append(('shared_noncommon_policers', PolicerLocation.BOTH_NONCOMMON_LINKS, m_prate/2))

    # Run experiments
    for m_seed in PRIMES[0: 1]:
        m_exp_params = []

        for app in WeheApp:
            try:
                m_app_setup = get_weheCS_app_setup(
                    wehe_app=app.value, original_traffic_duration=m_duration,
                    transport_protocol=TransportProtocol.TCP, tcp_protocol='TcpCubic',
                )

                # test case with different policing configurations
                for m_ptype, m_plocation, m_prate in m_policer_configs:
                    m_neutrality_setup = NeutralitySetup(
                        is_neutral=1, policing_rate=m_prate, burst_length=m_burst_period,
                        policer_location=m_plocation, policer_type=PolicerType.SHARED,
                        pct_of_throttled_background=0.3
                    )

                    m_exp_batch = '{}/{}_{}Mbps_{}s_30p'.format(m_network_setup_tag, m_ptype, m_prate, m_burst_period)
                    m_exp_params.append(ExperimentParameters(
                        exp_type='{}/{}'.format(TEST_DATE, TEST_TYPE), seed=m_seed, background_dir=m_background_dir,
                        exp_batch='{}/{}_{}Mbps_{}s_30p'.format(m_network_setup_tag, m_ptype, m_prate, m_burst_period),
                        network_setup=m_network_setup, measurement_app_setup=m_app_setup,
                        neutrality_setup=m_neutrality_setup
                    ))
            except Exception as e:
                print('app {} failed'.format(app.value))
        run_parallel_experiments(run_experiment_with_params, m_exp_params, nb_threads=1)
