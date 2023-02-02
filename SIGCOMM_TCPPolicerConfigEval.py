# This file is for experiments done after May 2022 related to neutrality-violations localization algorithm
# It is only compatible with the files in the localization_experiments_scenarios
import itertools

from helper_methods import *
from common_exp_params import *


# This is to specify which experiments I am currently focusing on
TEST_DATE = '02_2023'
TEST_TYPE = 'TCP_Policer_Configs_Eval4'


if __name__ == '__main__':
    rebuild_project()

    # network setup for no congestion
    m_network_setup_tag= 'no_congestion'
    m_nc_dps = '{}ms,{}ms'.format(get_nc_dp(d_rtt_ms), get_nc_dp(d_rtt_ms))
    m_nc_bandwidths = '{},{}'.format(d_nc_bandwidth, d_nc_bandwidth)
    m_network_setup = NetworkSetup(d_c_bandwidth, m_nc_dps, m_nc_bandwidths)

    # select which applications to test
    m_duration = 60
    m_app_setups = [
        MeasurementAppSetup(
            app_type=MeasurementAppType.INFINITE_TCP, app_name="Infinite_Paced_TCP",
            control_test_duration=m_duration, suspected_test_duration=m_duration,
            transport_protocol=TransportProtocol.TCP, tcp_protocol='TcpCubic', pkt_size=1228, app_data_rate='20Mbps'
        )]

    m_apps = [
        WeheApp.NbcSports, WeheApp.Netflix, WeheApp.FacebookVideo, WeheApp.Youtube, WeheApp.Amazon
    ]
    for m_app in m_apps:
        m_app_setups.append(get_weheCS_app_setup(
            wehe_app=m_app.value, original_traffic_duration=m_duration,
            transport_protocol=TransportProtocol.TCP, tcp_protocol='TcpCubic',
        ))



    # test with different policer configuration
    m_policer_configs, m_burst_period = [], 0.035
    m_rate_ratios, m_limit_ratios = rates_ratio, limits_as_ratios

    # Run experiments
    for back_v in [2, 3, 4, 5, 6]:
        m_exp_params = []

        for m_app_setup in m_app_setups:
            try:
                # the policer configurations
                m_policer_configs = []
                for p_rate_ratio, p_limit_ratio in itertools.product(m_rate_ratios, m_limit_ratios):
                    p_rate = int(np.round(app_volumes[m_app_setup.app_name] / p_rate_ratio))
                    m_policer_configs.append(('shared_common_policer', PolicerLocation.COMMON_LINK, p_rate, p_limit_ratio))
                    m_policer_configs.append(('shared_noncommon_policers', PolicerLocation.BOTH_NONCOMMON_LINKS, p_rate/2, p_limit_ratio))

                # add to experiment params
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
                    print('Exp: {}, {}, {}, {}'.format(m_app_setup.app_name, p_type, p_rate, p_limit_ratio))
                    m_exp_params.append(ExperimentParameters(
                        exp_type='{}/{}'.format(TEST_DATE, TEST_TYPE), seed=3,
                        background_setup=BackgroundTrafficSetup('chicago_2010_back_traffic_5min_control_cbp_2links_v{}'.format(back_v)),
                        exp_batch='{}/{}'.format(m_network_setup_tag, neutrality_tag),
                        network_setup=m_network_setup, measurement_app_setup=m_app_setup,
                        neutrality_setup=neutrality_setup
                    ))
            except Exception as e:
                print('app {} failed'.format(m_app_setup.app_name))

        run_parallel_experiments(run_experiment_with_params, m_exp_params, nb_threads=1)
