# This file is for experiments done after May 2022 related to neutrality-violations localization algorithm
# It is only compatible with the files in the localization_experiments_scenarios

from helper_methods import *
from common_exp_params import *


# This is to specify which experiments I am currently focusing on
TEST_DATE = '11_2022'
TEST_TYPE = 'Test_Throttling_Effect_on_Wehe'


if __name__ == '__main__':
    rebuild_project()

    # network setup for no congestion
    m_network_setup_tag= 'no_congestion'
    m_nc_dps = '{}ms,{}ms'.format(get_nc_dp(d_rtt_ms), get_nc_dp(d_rtt_ms))
    m_nc_bandwidths = '{},{}'.format(d_nc_bandwidth, d_nc_bandwidth)
    m_network_setup = NetworkSetup(d_c_bandwidth, m_nc_dps, m_nc_bandwidths)

    # test with different policer configuration
    m_policer_configs, m_burst_period = [], ms_2_s(d_rtt_ms)
    for p_rate in rates:
        for p_limit_ratio in limits_as_ratios:
            m_policer_configs.append(('shared_common_policer', PolicerLocation.COMMON_LINK, p_rate, p_limit_ratio))

    # Run experiments
    for m_seed in PRIMES[0: 10]:
        m_exp_params = []

        for app in WeheApp:
            try:
                app_setup = get_weheCS_app_setup(
                    wehe_app=app.value, original_traffic_duration=d_duration,
                    transport_protocol=TransportProtocol.TCP, tcp_protocol='TcpCubic',
                )

                for p_type, p_location, p_rate, p_limit_ratio in m_policer_configs:
                    # build policer configuration setup
                    neutrality_tag = '{}_r{}Mbps_b{}s_l{}_30p'.format(p_type, p_rate, m_burst_period, p_limit_ratio)
                    limit = int(p_limit_ratio * get_burst(p_rate, m_burst_period))
                    neutrality_setup = NeutralitySetup(
                        is_neutral=1, policing_rate=p_rate, burst_length=m_burst_period, queue_size=limit,
                        policer_location=p_location, policer_type=PolicerType.SHARED,
                        pct_of_throttled_background=0.3
                    )

                    # build and add experiment setup
                    m_exp_params.append(ExperimentParameters(
                        exp_type='{}/{}'.format(TEST_DATE, TEST_TYPE), seed=m_seed, background_setup=d_background_setup,
                        exp_batch='{}/{}'.format(m_network_setup_tag, neutrality_tag),
                        network_setup=m_network_setup, measurement_app_setup=app_setup,
                        neutrality_setup=neutrality_setup
                    ))
            except Exception as e:
                print('app {} failed'.format(app.value))

        run_parallel_experiments(run_experiment_with_params, m_exp_params, nb_threads=4)
