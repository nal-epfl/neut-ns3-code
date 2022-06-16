# This file is for experiments done after May 2022 related to neutrality-violations localization algorithm
# It is only compatible with the files in the localization_experiments_scenarios

from helper_methods import *

if __name__ == '__main__':
    # rebuild_project()

    # -------------------------------------- START TESTS -------------------------------------- #
    m_background_dir = 'chicago_2010_back_traffic_10min_control_cbp_2links'
    TEST_TYPE = 'Wehe_Test_Cases_DRY_RUN'

    m_common_link_rate, m_noncommon_link_rates = '10Gbps', '1Gbps,1Gbps,10Gbps'
    m_wehe_tests = [
        ('feqHGJ240F_9', WeheApp.Hulu.value, 14, 45, 38),
        # ('v8azm5f2qe_31', WeheApp.Youtube.value, 18, 61, 27),
        # ('y3emavi0lu_17', WeheApp.Youtube.value, 14, 82, 29),
    ]

    #for each user I need the following information: (user, app, duration, rtt, throttling_rate)
    for m_seed in [3]:
        m_exp_params = []
        for m_user, m_wehe_app, m_duration, m_rtt, m_throttling_rate in m_wehe_tests:
            m_network_setup = NetworkSetup(
                m_common_link_rate, '{}ms,{}ms,5ms'.format((m_rtt-10)/2, (m_rtt-10)/2), m_noncommon_link_rates
            )
            m_app_setup = get_weheCS_app_setup(
                wehe_app=m_wehe_app, transport_protocol=TransportProtocol.TCP, tcp_protocol='TcpCubic',
                original_traffic_duration=m_duration
            )
            m_burst_length = 0.03
            m_policing_scenarios = [
                ('shared_common_policer', PolicerLocation.COMMON_LINK, m_throttling_rate),
                # ('shared_noncommon_policers', PolicerLocation.BOTH_NONCOMMON_LINKS, m_throttling_rate/2),
            ]
            for m_ptype, m_plocation, m_prate in m_policing_scenarios:
                m_neutrality_setup = NeutralitySetup(
                    is_neutral=1, policing_rate=m_prate, burst_length=m_burst_length, policer_location=m_plocation,
                    policer_type=PolicerType.SHARED, pct_of_throttled_background=0.03
                )
                m_exp_batch = '{}/{}_{}Mbps_{}s_30p'.format(m_user, m_ptype, m_prate, m_burst_length)
                m_exp_params.append(ExperimentParameters(
                    network_setup=m_network_setup, measurement_app_setup=m_app_setup, background_dir=m_background_dir,
                    neutrality_setup=m_neutrality_setup, exp_batch=m_exp_batch, seed=m_seed)
                )
        run_parallel_experiments(run_experiment_with_params, m_exp_params, nb_threads=1)