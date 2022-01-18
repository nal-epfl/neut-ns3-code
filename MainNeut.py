# This file is for experiments done after Sep 2021 related to tomography inference based on loss
from project_run_env.RunConfig import *
from multiprocessing import Process


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
    result_folder_name = '12_2021/neut_with_loss/{}/link_{}/{}/{}/seed_{}/{}'.format(
        app_name, link_rate, background_dir, exp_batch, seed, tcp_protocol if is_tcp else 'udp'
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

    background_dirs = [
        'chicago_2010_3mins_traffics_database/chicago_2010_back_traffic_3min_1',
        # 'chicago_2010_3mins_traffics_database/chicago_2010_back_traffic_3min_2',
        # 'chicago_2010_3mins_traffics_database/chicago_2010_back_traffic_3min_3',
        # 'chicago_2010_3mins_traffics_database/chicago_2010_back_traffic_3min_4',
        # 'chicago_2010_3mins_traffics_database/chicago_2010_back_traffic_3min_5',
        # 'chicago_2010_3mins_traffics_database/chicago_2010_back_traffic_3min_6',
    ]

    # case, noncommon_links_delays, noncommon_links_rates = 'ideal', 'empty', 'empty'
    # duration = 170
    # seed = 7
    #
    # for background_dir in background_dirs:
    #     run_parallel_experiments([
    #         ExperimentParameters(
    #             link_rate="210Mbps", duration=duration, is_tcp=0, seed=seed,
    #             app_type=3, app_name='Poisson_Udp', replay_trace='poisson_trace_lambda_2.5ms_pktSize_1228_10min',
    #             background_dir=background_dir,
    #             exp_batch=case+'/lambda2.5ms/neutral',
    #             noncommon_links_delays=noncommon_links_delays, noncommon_links_rates=noncommon_links_rates,
    #             is_neutral=0
    #         ),
    #         ExperimentParameters(
    #             link_rate="210Mbps", duration=duration, is_tcp=0, seed=seed,
    #             app_type=3, app_name='Poisson_Udp', replay_trace='poisson_trace_lambda_2.5ms_pktSize_1228_10min',
    #             background_dir=background_dir,
    #             exp_batch=case+'/lambda2.5ms/single_policer_40Mbps_0.03s_40p',
    #             noncommon_links_delays=noncommon_links_delays, noncommon_links_rates=noncommon_links_rates,
    #             is_neutral=1, policing_rate=40, burst_length=0.03
    #         ),
    #         ExperimentParameters(
    #             link_rate="210Mbps", duration=duration, is_tcp=0, seed=seed,
    #             app_type=3, app_name='Poisson_Udp', replay_trace='poisson_trace_lambda_2.5ms_pktSize_1228_10min',
    #             background_dir=background_dir,
    #             exp_batch=case+'/lambda2.5ms/single_policer_50Mbps_0.03s_40p',
    #             noncommon_links_delays=noncommon_links_delays, noncommon_links_rates=noncommon_links_rates,
    #             is_neutral=1, policing_rate=50, burst_length=0.03
    #         ),
    #         ExperimentParameters(
    #             link_rate="210Mbps", duration=duration,  is_tcp=0, seed=seed,
    #             app_type=3, app_name='Poisson_Udp', replay_trace='poisson_trace_lambda_2.5ms_pktSize_1228_10min',
    #             background_dir=background_dir,
    #             exp_batch=case+'/lambda2.5ms/single_noncommon_policer_15Mbps_0.03s_40p',
    #             noncommon_links_delays=noncommon_links_delays, noncommon_links_rates=noncommon_links_rates,
    #             is_neutral=3, policing_rate=15, burst_length=0.03
    #         ),
    #         ExperimentParameters(
    #             link_rate="210Mbps", duration=duration,  is_tcp=0, seed=seed,
    #             app_type=3, app_name='Poisson_Udp', replay_trace='poisson_trace_lambda_2.5ms_pktSize_1228_10min',
    #             background_dir=background_dir,
    #             exp_batch=case+'/lambda2.5ms/single_noncommon_policer_18Mbps_0.03s_40p',
    #             noncommon_links_delays=noncommon_links_delays, noncommon_links_rates=noncommon_links_rates,
    #             is_neutral=3, policing_rate=18, burst_length=0.03
    #         ),
    #     ])

    case, noncommon_links_delays, noncommon_links_rates = 'test', 'empty', 'empty'
    duration = 170
    seed = 7

    for background_dir in background_dirs:
        run_parallel_experiments([
            ExperimentParameters(
                link_rate="210Mbps", duration=duration, is_tcp=1, tcp_protocol='TcpCubic', seed=seed,
                app_type=4, app_name='Infinite_Tcp', pkt_size=1228, background_dir=background_dir,
                exp_batch=case+'/neutral',
                noncommon_links_delays=noncommon_links_delays, noncommon_links_rates=noncommon_links_rates,
                is_neutral=0
            ),
            ExperimentParameters(
                link_rate="210Mbps", duration=duration, is_tcp=1, tcp_protocol='TcpCubic', seed=seed,
                app_type=4, app_name='Infinite_Tcp', pkt_size=1228, background_dir=background_dir,
                exp_batch=case+'/single_policer_40Mbps_0.03s_40p',
                noncommon_links_delays=noncommon_links_delays, noncommon_links_rates=noncommon_links_rates,
                is_neutral=1, policing_rate=40, burst_length=0.03
            ),
            ExperimentParameters(
                link_rate="210Mbps", duration=duration, is_tcp=1, tcp_protocol='TcpCubic', seed=seed,
                app_type=4, app_name='Infinite_Tcp', pkt_size=1228, background_dir=background_dir,
                exp_batch=case+'/single_policer_50Mbps_0.03s_40p',
                noncommon_links_delays=noncommon_links_delays, noncommon_links_rates=noncommon_links_rates,
                is_neutral=1, policing_rate=50, burst_length=0.03
            ),
        ])


    # exps = [
    #     ('ideal', 'empty', 'empty'),
    #     # ('differentRTTs_onp3_8ms', "5ms,5ms,8ms,5ms,5ms", 'empty'),
    #     # ('nonCommonCongestion_onp3_100Mbps', 'empty', '1Gbps,1Gbps,100Mbps,1Gbps,10Gbps'),
    #     # ('nonCommonCongestion_onp3_150Mbps_onp4_100Mbps', 'empty', '1Gbps,1Gbps,150Mbps,100Mbps,10Gbps')
    # ]
    #
    # seed = 3
    # background_dir = 'chicago_2010_3mins_traffics_database/chicago_2010_back_traffic_3min_1'
    # for case, noncommon_links_delays, noncommon_links_rates in exps:
    #     print(case, noncommon_links_delays, noncommon_links_rates)
    #
    #     run_parallel_experiments([
    #         ExperimentParameters(
    #             link_rate="210Mbps", duration=240, is_tcp=1, tcp_protocol='TcpCubic', seed=seed,
    #             app_type=4, app_name='Infinite_Tcp', pkt_size=1228, background_dir=background_dir,
    #             exp_batch=case+'/neutral',
    #             noncommon_links_delays=noncommon_links_delays, noncommon_links_rates=noncommon_links_rates,
    #             is_neutral=0
    #         ),
    #         ExperimentParameters(
    #             link_rate="210Mbps", duration=240, is_tcp=1, tcp_protocol='TcpCubic', seed=seed,
    #             app_type=4, app_name='Infinite_Tcp', pkt_size=1228, background_dir=background_dir,
    #             exp_batch=case+'/single_policer_40Mbps_0.03s_80p',
    #             noncommon_links_delays=noncommon_links_delays, noncommon_links_rates=noncommon_links_rates,
    #             is_neutral=1, policing_rate=40, burst_length=0.03
    #         ),
    #         ExperimentParameters(
    #             link_rate="210Mbps", duration=240, is_tcp=1, tcp_protocol='TcpCubic', seed=seed,
    #             app_type=4, app_name='Infinite_Tcp', pkt_size=1228, background_dir=background_dir,
    #             exp_batch=case+'/single_noncommon_policer_20Mbps_0.03s_80p',
    #             noncommon_links_delays=noncommon_links_delays, noncommon_links_rates=noncommon_links_rates,
    #             is_neutral=3, policing_rate=20, burst_length=0.03
    #         ),
    #         ExperimentParameters(
    #             link_rate="210Mbps", duration=240, is_tcp=1, tcp_protocol='TcpCubic', seed=seed,
    #             app_type=4, app_name='Infinite_Tcp', pkt_size=1228, background_dir=background_dir,
    #             exp_batch=case+'/single_noncommon_policer_onp4_20Mbps_0.03s_80p',
    #             noncommon_links_delays=noncommon_links_delays, noncommon_links_rates=noncommon_links_rates,
    #             is_neutral=5, policing_rate=20, burst_length=0.03
    #         ),
    #     ])
