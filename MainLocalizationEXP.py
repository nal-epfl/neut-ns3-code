# This file is for experiments done after May 2022 related to neutrality-violations localization algorithm
# It is only compatible with the files in the localization_experiments_scenarios

from helper_methods import *


# This is to specify which experiments I am currently focusing on
TEST_DATE = '7_2022'
TEST_TYPE = 'Wehe_Test_Cases_DRY_RUN'


if __name__ == '__main__':
    # rebuild_project()

    # -------------------------------------- START TESTS -------------------------------------- #
    m_background_dir = 'chicago_2010_back_traffic_10min_control_cbp_2links'

    m_common_link_rate, m_noncommon_link_rates = '10Gbps', '1Gbps,1Gbps,10Gbps'
    m_wehe_tests = [
        # ('VG7er5CtV7', WeheApp.Youtube.value, 18, 124, 28),
        ('9wz6eplwl8', 'Youtube_12122018', 40, 134, 58), #[1.19%])
        ('0D7Y0OrR73', 'Youtube_12122018', 46, 79, 41), #[2.65%])
        ('vxde6fi9mc', 'Youtube_12122018', 46, 67, 40), #[2.29%])
        ('oyvlljwir1', 'Youtube_12122018', 46, 115, 27), #[6.12%])
        ('4uoalqmrpm', 'Youtube_12122018', 46, 20, 36), #[3.75%])
        ('y1b77sgi53', 'Youtube_12122018', 46, 59, 60), #[0.62%])
        ('89twsgj1pb', 'Youtube_12122018', 33, 80, 35), #[4.67%])
        ('WCkJlqTlQ2', 'Youtube_12122018', 45, 113, 35), #[4.42%])
        ('x99AyQDr7F', 'Youtube_12122018', 45, 27, 38), #[3.36%])
        ('S1I7FxOQWT', 'Youtube_12122018', 29, 39, 40), #[2.4%])
        ('i40HQwwAjN', 'Youtube_12122018', 31, 50, 45), #[1.26%])
        ('iATMBAqaf5', 'Youtube_12122018', 29, 60, 43), #[2.06%])
        ('7AEYFCm1mg', 'Youtube_12122018', 30, 39, 59), #[0.86%])
        ('r4thi26qhc', 'Youtube_12122018', 46, 99, 55), #[1.68%])
        ('b1fmmqkbjw', 'Youtube_12122018', 46, 72, 60), #[0.67%])
        ('vwjl3su6wz', 'Youtube_12122018', 46, 44, 56), #[1.51%])
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
            m_burst_length = m_rtt / 1000 #0.03
            m_policing_scenarios = [
                ('shared_common_policer', PolicerLocation.COMMON_LINK, m_throttling_rate),
                ('shared_noncommon_policers', PolicerLocation.BOTH_NONCOMMON_LINKS, m_throttling_rate/2),
            ]
            for m_ptype, m_plocation, m_prate in m_policing_scenarios:
                m_neutrality_setup = NeutralitySetup(
                    is_neutral=1, policing_rate=m_prate, burst_length=m_burst_length, policer_location=m_plocation,
                    policer_type=PolicerType.SHARED, pct_of_throttled_background=0.3
                )
                m_exp_batch = '{}/{}_{}Mbps_{}s_30p'.format(m_user, m_ptype, m_prate, m_burst_length)
                m_exp_params.append(ExperimentParameters(
                    exp_type='{}/{}'.format(TEST_DATE, TEST_TYPE),
                    network_setup=m_network_setup, measurement_app_setup=m_app_setup, background_dir=m_background_dir,
                    neutrality_setup=m_neutrality_setup, exp_batch=m_exp_batch, seed=m_seed)
                )
        run_parallel_experiments(run_experiment_with_params, m_exp_params, nb_threads=1)