# This file is for experiments done after May 2022 related to neutrality-violations localization algorithm
# It is only compatible with the files in the localization_experiments_scenarios
import json
from helper_methods import *


def load_wehe_tests_info(tests_info_dir):
    tests_infos = []
    for test in os.listdir(tests_info_dir):
        if '.json' in test:
            with open('{}/{}'.format(tests_info_dir, test)) as f: info = json.load(f)
            if info['est_throttling_rate'] <= 0: continue
            tests_infos.append((
                '{}_{}'.format(info['user_id'], info['test_id']),
                get_wehe_app(info['app'].split('-')[0]).value,
                math.floor(info['d_o']), math.floor(info['rtt_o']*1e3),
                info['est_throttling_rate'],
                '{}/{}'.format(tests_info_dir, info['overflow_trace'])
            ))
    return tests_infos


# This is to specify which experiments I am currently focusing on
TEST_DATE = '7_2022'
TEST_TYPE = 'Wehe_Test_Cases_DRY_RUN_3'

if __name__ == '__main__':
    rebuild_project()

    m_background_dir = 'chicago_2010_back_traffic_10min_control_cbp_2links'
    m_common_link_delay = 3
    m_tests_info_dir = '{}/data/localization_wehe_tests_exp_input'.format(get_project_path())
    m_wehe_tests = load_wehe_tests_info(m_tests_info_dir)
    [print(info) for info in m_wehe_tests]

    for m_seed in PRIMES[0: 3]:
        m_exp_params = []
        for m_user, m_wehe_app, m_duration, m_rtt, m_throttling_rate, m_overflow_trace in m_wehe_tests:

            m_network_setup = NetworkSetup(
                common_link_rate='10Gbps',
                noncommon_links_delays='{}ms,{}ms'.format(m_rtt/2 - m_common_link_delay, m_rtt/2 - m_common_link_delay),
                noncommon_links_rates='1Gbps,1Gbps'
            )
            m_app_setup = get_weheCS_app_setup(
                wehe_app=m_wehe_app, original_traffic_duration=m_duration,
                transport_protocol=TransportProtocol.TCP, tcp_protocol='TcpCubic',
            )

            m_burst_period = 0.02 #m_rtt/1e3
            m_policing_scenarios = [
                ('shared_common_policer', PolicerLocation.COMMON_LINK, m_throttling_rate),
                ('shared_noncommon_policers', PolicerLocation.BOTH_NONCOMMON_LINKS, m_throttling_rate/2),
            ]

            for m_ptype, m_plocation, m_prate in m_policing_scenarios:
                m_neutrality_setup = NeutralitySetup(
                    is_neutral=1, policing_rate=m_prate, burst_length=m_burst_period,
                    policer_location=m_plocation, policer_type=PolicerType.SHARED,
                    pct_of_throttled_background=0.3, overflowEventsTrace=m_overflow_trace
                )

                m_exp_params.append(ExperimentParameters(
                    exp_type='{}/{}'.format(TEST_DATE, TEST_TYPE), seed=m_seed, background_dir=m_background_dir,
                    exp_batch='{}/{}_{}Mbps_{}s_30p'.format(m_user, m_ptype, m_prate, m_burst_period),
                    network_setup=m_network_setup, measurement_app_setup=m_app_setup,
                    neutrality_setup=m_neutrality_setup)
                )
        run_parallel_experiments(run_experiment_with_params, m_exp_params, nb_threads=1)
