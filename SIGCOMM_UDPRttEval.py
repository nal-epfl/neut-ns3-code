# This file is for experiments done after May 2022 related to neutrality-violations localization algorithm
# It is only compatible with the files in the localization_experiments_scenarios
import itertools

from helper_methods import *
from common_exp_params import *


# This is to specify which experiments I am currently focusing on
TEST_DATE = '02_2023'
TEST_TYPE = 'UDP_Rtt_Eval'


if __name__ == '__main__':
    rebuild_project()

    # network setup for no congestion but different RTTs
    m_nc_bandwidths = '{},{}'.format(d_nc_bandwidth, d_nc_bandwidth)
    m_network_setups = []
    for m_rtt in rtts_ms:
        m_network_setup_tag = 'no_congestion_p1_rtt{}_p2_rtt{}'.format(d_rtt_ms, m_rtt)
        m_nc_dps = '{}ms,{}ms'.format(get_nc_dp(d_rtt_ms), get_nc_dp(m_rtt))
        m_network_setups.append((m_network_setup_tag, NetworkSetup(d_c_bandwidth, m_nc_dps, m_nc_bandwidths)))
    for m_rtt in rtts_ms:
        m_network_setup_tag = 'no_congestion_p1p2_rtt{}'.format(m_rtt)
        m_nc_dps = '{}ms,{}ms'.format(get_nc_dp(m_rtt), get_nc_dp(m_rtt))
        m_network_setups.append((m_network_setup_tag, NetworkSetup(d_c_bandwidth, m_nc_dps, m_nc_bandwidths)))

    # select which applications to test
    m_apps = [
        UDPWeheApp.Webex, UDPWeheApp.Probe2Webex, UDPWeheApp.IncProbeWebex,
        UDPWeheApp.Skype, UDPWeheApp.Probe2Skype, UDPWeheApp.IncProbeSkype,
        UDPWeheApp.WhatsApp, UDPWeheApp.Probe2WhatsApp, UDPWeheApp.IncProbeWhatsApp,
    ]

    # test with different policer configuration
    m_policer_configs, m_burst_period = [], 0.035
    m_rate_ratios, m_limit_ratios = rates_ratio, limits_as_ratios

    # Run experiments
    for m_seed in PRIMES[0: 5]:
        m_exp_params = []

        for app in m_apps:

            for network_setup_tag, network_setup in m_network_setups:

                try:
                    app_setup = get_weheCS_app_setup(
                        wehe_app=app.value, original_traffic_duration=60, transport_protocol=TransportProtocol.UDP,
                    )

                    # the policer configurations
                    m_policer_configs = []
                    for p_rate_ratio, p_limit_ratio in itertools.product(m_rate_ratios, m_limit_ratios):
                        p_rate = int(np.round(app_volumes[app_setup.app_name] / p_rate_ratio))
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
                        print('Exp: {}, {}, {}, {}'.format(app, p_type, p_rate, p_limit_ratio))
                        m_exp_params.append(ExperimentParameters(
                            exp_type='{}/{}'.format(TEST_DATE, TEST_TYPE), seed=m_seed, background_setup=d_background_setup,
                            exp_batch='{}/{}'.format(network_setup_tag, neutrality_tag),
                            network_setup=network_setup, measurement_app_setup=app_setup,
                            neutrality_setup=neutrality_setup
                        ))
                except Exception as e:
                    print('app {} failed'.format(app.value))

        run_parallel_experiments(run_experiment_with_params, m_exp_params, nb_threads=1)
