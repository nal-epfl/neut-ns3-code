# This file is for experiments done after Sep 2021 related to tomography inference based on loss
from project_run_env.RunConfig import *
from multiprocessing import Process

# This is to specify how I am performing the test
TEST_TYPE = 'b2b_neut_with_loss'


class ExperimentParameters:
    def __init__(self, link_rate, duration, is_tcp=0, tcp_protocol='TcpCubic', seed=3,
                 app_type=0, app_name='Poisson_Probes', pkt_size=1228, p_lambda=0.001, replay_trace='empty',
                 background_dir='empty', exp_batch='', noncommon_links_delays='empty', noncommon_links_rates='empty',
                 is_neutral=0, policing_rate=0.0, burst_length=0.0, throttle_udp=0):
        self.link_rate = link_rate
        self.duration = duration
        self.is_tcp = is_tcp
        self.tcp_protocol = tcp_protocol
        self.seed = seed
        self.app_type = app_type
        self.app_name = app_name
        self.pkt_size = pkt_size
        self.p_lambda = p_lambda
        self.replay_trace = replay_trace
        self.background_dir = background_dir
        self.exp_batch = exp_batch
        self.noncommon_links_delays = noncommon_links_delays
        self.noncommon_links_rates = noncommon_links_rates
        self.is_neutral = is_neutral
        self.policing_rate = policing_rate
        self.burst_length = burst_length
        self.throttle_udp = throttle_udp


def run_probing_experiment_with_params(params):
    run_probing_experiment(link_rate=params.link_rate, duration=params.duration,
                           is_tcp=params.is_tcp, tcp_protocol=params.tcp_protocol, seed=params.seed,
                           app_type=params.app_type, app_name=params.app_name,
                           pkt_size=params.pkt_size, p_lambda=params.p_lambda, replay_trace=params.replay_trace,
                           background_dir=params.background_dir, exp_batch=params.exp_batch,
                           noncommon_links_delays=params.noncommon_links_delays,
                           noncommon_links_rates=params.noncommon_links_rates,
                           is_neutral=params.is_neutral, policing_rate=params.policing_rate,
                           burst_length=params.burst_length, throttle_udp=params.throttle_udp)


def run_probing_experiment(link_rate, duration, is_tcp, tcp_protocol='TcpCubic', seed=3,
                           app_type=0, app_name='Poisson_Probes', pkt_size=1228, p_lambda=0.001, replay_trace='empty',
                           background_dir='empty', exp_batch='',
                           noncommon_links_delays='empty', noncommon_links_rates='empty',
                           is_neutral=0, policing_rate=0.0, burst_length=0.0, throttle_udp=0):
    # run the ns3 simulation
    result_folder_name = '1_2022/{}/{}/link_{}/{}/{}/seed_{}/{}'.format(
        TEST_TYPE, app_name, link_rate, background_dir, exp_batch, seed, tcp_protocol if is_tcp else 'udp'
    )

    os.system('mkdir -p {}/scratch/wehe_p_tomography/results/{}'.format(get_ns3_path(), result_folder_name))
    os.system(
        # '{}/waf --run "wehe_p_tomography" > log.out 2>&1 --command-template="%s'.format(get_ns3_path()) +
        '{}/waf --run "wehe_p_tomography" --command-template="%s'.format(get_ns3_path()) +
        ' --RngSeed={}'.format(seed) +
        ' --RngRun=1' +
        ' --linkRate={}'.format(link_rate) +
        ' --duration={}'.format(duration) +
        ' --resultsFolderName={}'.format(result_folder_name) +
        ' --appProtocol={}'.format(is_tcp) +
        ' --TCPProtocol=ns3::{}'.format(tcp_protocol) +
        ' --appType={}'.format(app_type) +
        ' --pktSize={}'.format(pkt_size) +
        ' --lambda={}'.format(p_lambda) +
        ' --replayTrace={}'.format(replay_trace) +
        ' --backgroundDir={}'.format(background_dir) +
        ' --neutral={}'.format(is_neutral) +
        ' --policingRate={}'.format(policing_rate) +
        ' --policingBurstLength={}'.format(burst_length) +
        ' --throttleUdp={}'.format(throttle_udp) +
        ' --nonCommonlinksDelays={}'.format(noncommon_links_delays) +
        ' --nonCommonlinksDataRates={}'.format(noncommon_links_rates) +
        '"'
    )


def rebuild_project():
    os.system('{}/waf build'.format(get_ns3_path()))


def run_parallel_experiments(experiments):
    processes = [Process(target=run_probing_experiment_with_params, kwargs={'params': params}) for params in
                 experiments]

    # kick them off
    for process in processes:
        process.start()

    # now wait for them to finish
    for process in processes:
        process.join()


if __name__ == '__main__':
    rebuild_project()

    # common_link_rate, seed, duration = '210Mbps', 3, 240
    # background_dir = 'chicago_2010_back_traffic_5min_control_cbp_2links'
    # exps = [
    #     ('ideal', 'empty', 'empty'),
    #     ('differentRTTs_onp3_8ms', "5ms,5ms,8ms,5ms,5ms", 'empty'),
    #     ('differentRTTs_onp3_15ms', "5ms,5ms,15ms,5ms,5ms", 'empty'),
    #     ('nonCommonCongestion_onp3_80Mbps', 'empty', '1Gbps,1Gbps,80Mbps,1Gbps,10Gbps'),
    #     ('nonCommonCongestion_onp3_100Mbps', 'empty', '1Gbps,1Gbps,100Mbps,1Gbps,10Gbps'),
    #     ('nonCommonCongestion_onp3_80Mbps_onp4_100Mbps', 'empty', '1Gbps,1Gbps,80Mbps,100Mbps,10Gbps'),
    #     ('nonCommonCongestion_onp3_150Mbps_onp4_100Mbps', 'empty', '1Gbps,1Gbps,150Mbps,100Mbps,10Gbps'),
    #     ('nonCommonCongestion_onp34_100Mbps', 'empty', '1Gbps,1Gbps,100Mbps,100Mbps,10Gbps'),
    # ]
    # for case, noncommon_links_delays, noncommon_links_rates in exps:
    #     print(case, noncommon_links_delays, noncommon_links_rates)
    #
    #     run_parallel_experiments([
    #         ExperimentParameters(
    #             link_rate=common_link_rate, duration=duration, is_tcp=1, tcp_protocol='TcpCubic', seed=seed,
    #             app_type=4, app_name='Infinite_Tcp', pkt_size=1228, background_dir=background_dir,
    #             exp_batch=case+'/neutral',
    #             noncommon_links_delays=noncommon_links_delays, noncommon_links_rates=noncommon_links_rates,
    #             is_neutral=0
    #         ),
    #         ExperimentParameters(
    #             link_rate=common_link_rate, duration=duration, is_tcp=1, tcp_protocol='TcpCubic', seed=seed,
    #             app_type=4, app_name='Infinite_Tcp', pkt_size=1228, background_dir=background_dir,
    #             exp_batch=case+'/single_policer_40Mbps_0.03s_80p',
    #             noncommon_links_delays=noncommon_links_delays, noncommon_links_rates=noncommon_links_rates,
    #             is_neutral=1, policing_rate=40, burst_length=0.03
    #         ),
    #         ExperimentParameters(
    #             link_rate=common_link_rate, duration=duration, is_tcp=1, tcp_protocol='TcpCubic', seed=seed,
    #             app_type=4, app_name='Infinite_Tcp', pkt_size=1228, background_dir=background_dir,
    #             exp_batch=case+'/single_noncommon_policer_20Mbps_0.03s_80p',
    #             noncommon_links_delays=noncommon_links_delays, noncommon_links_rates=noncommon_links_rates,
    #             is_neutral=3, policing_rate=20, burst_length=0.03
    #         ),
    #         ExperimentParameters(
    #             link_rate=common_link_rate, duration=duration, is_tcp=1, tcp_protocol='TcpCubic', seed=seed,
    #             app_type=4, app_name='Infinite_Tcp', pkt_size=1228, background_dir=background_dir,
    #             exp_batch=case+'/single_noncommon_policer_onp4_20Mbps_0.03s_80p',
    #             noncommon_links_delays=noncommon_links_delays, noncommon_links_rates=noncommon_links_rates,
    #             is_neutral=5, policing_rate=20, burst_length=0.03
    #         ),
    #     ])

    seed, duration = 3, 240
    background_dir = 'chicago_2010_back_traffic_5min_control_cbp_2links'
    run_parallel_experiments([
        # congestion is on the common link only
        ExperimentParameters(
            link_rate='200Mbps', duration=duration, is_tcp=1, tcp_protocol='TcpCubic', seed=seed,
            app_type=4, app_name='Infinite_Tcp', pkt_size=1228, background_dir=background_dir,
            exp_batch='congestion_on_common_link_only/neutral',
            noncommon_links_delays='empty', noncommon_links_rates='1Gbps,1Gbps,10Gbps',
            is_neutral=0
        ),
        # congestion is on the non-common links only
        ExperimentParameters(
            link_rate='10Gbps', duration=duration, is_tcp=1, tcp_protocol='TcpCubic', seed=seed,
            app_type=4, app_name='Infinite_Tcp', pkt_size=1228, background_dir=background_dir,
            exp_batch='congestion_on_noncommon_links_only/neutral',
            noncommon_links_delays='empty', noncommon_links_rates='80Mbps,80Mbps,10Gbps',
            is_neutral=0
        ),
        # no congestion but a similar policer on each of the non-common links
        ExperimentParameters(
            link_rate='10Gbps', duration=duration, is_tcp=1, tcp_protocol='TcpCubic', seed=seed,
            app_type=4, app_name='Infinite_Tcp', pkt_size=1228, background_dir=background_dir,
            exp_batch='/policer_on_noncommon_links_only/single_noncommon_policer_20Mbps_0.03s_80p',
            noncommon_links_delays='empty', noncommon_links_rates='200Mbps,200Mbps,10Gbps',
            is_neutral=3, policing_rate=20, burst_length=0.03
        )
    ])
