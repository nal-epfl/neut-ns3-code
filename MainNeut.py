# This file is for experiments done after Sep 2021 related to tomography inference based on loss
from project_run_env.RunConfig import *
from multiprocessing import Process

app_name = 'Poisson_Probes'


class ExperimentParameters:
    def __init__(self, link_rate, duration, is_tcp=0, tcp_protocol='TcpCubic', seed=3, app_type=0, exp_batch='',
                 pkt_size=1228, p_lambda=0.001, replay_trace='empty', is_neutral=0, policing_rate=0.0,
                 burst_length=0.0, throttle_udp=0, scenario=0):
        self.link_rate = link_rate
        self.duration = duration
        self.is_tcp = is_tcp
        self.tcp_protocol = tcp_protocol
        self.seed = seed
        self.app_type = app_type
        self.exp_batch = exp_batch
        self.pkt_size = pkt_size
        self.p_lambda = p_lambda
        self.replay_trace = replay_trace
        self.is_neutral = is_neutral
        self.policing_rate = policing_rate
        self.burst_length = burst_length
        self.throttle_udp = throttle_udp
        self.scenario = scenario


def run_probing_experiment_with_params(params):
    run_probing_experiment(link_rate=params.link_rate, duration=params.duration, is_tcp=params.is_tcp,
                           tcp_protocol=params.tcp_protocol, seed=params.seed, app_type=params.app_type,
                           exp_batch=params.exp_batch, pkt_size=params.pkt_size, p_lambda=params.p_lambda,
                           replay_trace=params.replay_trace, is_neutral=params.is_neutral,
                           policing_rate=params.policing_rate, burst_length=params.burst_length,
                           throttle_udp=params.throttle_udp, scenario=params.scenario)


def run_probing_experiment(link_rate, duration, is_tcp, tcp_protocol='TcpCubic', seed=3, app_type=0, exp_batch='',
                           pkt_size=1228, p_lambda=0.001, replay_trace='empty', is_neutral=0, policing_rate=0.0,
                           burst_length=0.0, throttle_udp=0, scenario=0):
    # run the ns3 simulation
    result_folder_name = '/10_2021/neut_with_loss/' + app_name + '/link_' + link_rate + '/' + exp_batch + '/seed_' + str(seed)
    if is_tcp:
        result_folder_name = result_folder_name + '/' + tcp_protocol
    else:
        result_folder_name = result_folder_name + '/udp'
    results_path = get_ns3_path() + '/scratch/wehe_p_tomography/results' + result_folder_name
    print(results_path)

    os.system("mkdir -p " + results_path)
    # os.system(get_ns3_path() + "/waf --run \"wehe_p_tomography\" > log.out 2>&1 --command-template=\"%s" +
    os.system(get_ns3_path() + "/waf --run \"wehe_p_tomography\" --command-template=\"%s" +
              " --RngSeed=" + str(seed) +
              " --RngRun=1" +
              " --duration=" + str(duration) +
              " --resultsFolderName=" + result_folder_name +
              " --linkRate=" + link_rate +
              " --appProtocol=" + str(is_tcp) +
              " --TCPProtocol=ns3::" + tcp_protocol +
              " --appType=" + str(app_type) +
              " --pktSize=" + str(pkt_size) +
              " --lambda=" + str(p_lambda) +
              " --replayTrace=" + replay_trace +
              " --neutral=" + str(is_neutral) +
              " --policingRate=" + str(policing_rate) +
              " --policingBurstLength=" + str(burst_length) +
              " --throttleUdp=" + str(throttle_udp) +
              " --scenario=" + str(scenario) +
              "\"")


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

    # run_parallel_experiments([
    #     ExperimentParameters(link_rate="5Mbps", duration=50, is_tcp=1, tcp_protocol='TcpBbr',
    #                          exp_batch='back_traffic_long_3/test_bbr',
    #                          scenario=4, pkt_size=1228, seed=3,
    #                          is_neutral=0, policing_rate=2, burst_length=0.03, case=0),
    # ])


    run_parallel_experiments([
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp/neutral',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=0, scenario=0),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp/single_policer_40Mbps_0.03s_80p_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=1, policing_rate=40, burst_length=0.03, scenario=0),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp/single_noncommon_policer_20Mbps_0.03s_80p_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=3, policing_rate=20, burst_length=0.03, scenario=0),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp/independent_policer_2.5Mbps_0.03s_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=2, policing_rate=2.5, burst_length=0.03, scenario=0),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp/independent_noncommon_policer_2.5Mbps_0.03s_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=4, policing_rate=2.5, burst_length=0.03, scenario=0)
    ])

    run_parallel_experiments([
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp_differentRTT_p2_15ms/neutral',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=0, scenario=3),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp_differentRTT_p2_15ms/single_policer_40Mbps_0.03s_80p_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=1, policing_rate=40, burst_length=0.03, scenario=3),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp_differentRTT_p2_15ms/single_noncommon_policer_20Mbps_0.03s_80p_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=3, policing_rate=20, burst_length=0.03, scenario=3),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp_differentRTT_p2_15ms/independent_policer_2.5Mbps_0.03s_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=2, policing_rate=2.5, burst_length=0.03, scenario=3),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp_differentRTT_p2_15ms/independent_noncommon_policer_2.5Mbps_0.03s_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=4, policing_rate=2.5, burst_length=0.03, scenario=3)
    ])


    run_parallel_experiments([
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p23_100Mbps/neutral',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=0, scenario=4),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p23_100Mbps/single_policer_40Mbps_0.03s_80p_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=1, policing_rate=40, burst_length=0.03, scenario=4),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p23_100Mbps/single_noncommon_policer_20Mbps_0.03s_80p_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=3, policing_rate=20, burst_length=0.03, scenario=4),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p23_100Mbps/independent_policer_2.5Mbps_0.03s_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=2, policing_rate=2.5, burst_length=0.03, scenario=4),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p23_100Mbps/independent_noncommon_policer_2.5Mbps_0.03s_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=4, policing_rate=2.5, burst_length=0.03, scenario=4)
    ])

    run_parallel_experiments([
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p2_100Mbps/neutral',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=0, scenario=5),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p2_100Mbps/single_policer_40Mbps_0.03s_80p_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=1, policing_rate=40, burst_length=0.03, scenario=5),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p2_100Mbps/single_noncommon_policer_20Mbps_0.03s_80p_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=3, policing_rate=20, burst_length=0.03, scenario=5),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p2_100Mbps/independent_policer_2.5Mbps_0.03s_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=2, policing_rate=2.5, burst_length=0.03, scenario=5),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p2_100Mbps/independent_noncommon_policer_2.5Mbps_0.03s_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=4, policing_rate=2.5, burst_length=0.03, scenario=5)
    ])

    run_parallel_experiments([
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpCubic',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p2_100Mbps/neutral',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=0, scenario=5),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpCubic',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p2_100Mbps/single_policer_40Mbps_0.03s_80p_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=1, policing_rate=40, burst_length=0.03, scenario=5),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpCubic',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p2_100Mbps/single_noncommon_policer_20Mbps_0.03s_80p_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=3, policing_rate=20, burst_length=0.03, scenario=5),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpCubic',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p2_100Mbps/independent_policer_2.5Mbps_0.03s_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=2, policing_rate=2.5, burst_length=0.03, scenario=5),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpCubic',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p2_100Mbps/independent_noncommon_policer_2.5Mbps_0.03s_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=4, policing_rate=2.5, burst_length=0.03, scenario=5)
    ])

    run_parallel_experiments([
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p2_150Mbps_p3_100Mbps/neutral',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=0, scenario=6),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p2_150Mbps_p3_100Mbps/single_policer_40Mbps_0.03s_80p_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=1, policing_rate=40, burst_length=0.03, scenario=6),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p2_150Mbps_p3_100Mbps/single_noncommon_policer_20Mbps_0.03s_80p_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=3, policing_rate=20, burst_length=0.03, scenario=6),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p2_150Mbps_p3_100Mbps/independent_policer_2.5Mbps_0.03s_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=2, policing_rate=2.5, burst_length=0.03, scenario=6),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpBbr',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p2_150Mbps_p3_100Mbps/independent_noncommon_policer_2.5Mbps_0.03s_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=4, policing_rate=2.5, burst_length=0.03, scenario=6)
    ])

    run_parallel_experiments([
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpCubic',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p2_150Mbps_p3_100Mbps/neutral',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=0, scenario=6),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpCubic',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p2_150Mbps_p3_100Mbps/single_policer_40Mbps_0.03s_80p_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=1, policing_rate=40, burst_length=0.03, scenario=6),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpCubic',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p2_150Mbps_p3_100Mbps/single_noncommon_policer_20Mbps_0.03s_80p_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=3, policing_rate=20, burst_length=0.03, scenario=6),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpCubic',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p2_150Mbps_p3_100Mbps/independent_policer_2.5Mbps_0.03s_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=2, policing_rate=2.5, burst_length=0.03, scenario=6),
        ExperimentParameters(link_rate="210Mbps", duration=300, is_tcp=1, tcp_protocol='TcpCubic',
                             exp_batch='back_traffic_long_3/infinite_tcp_noncommon_congestion_p2_150Mbps_p3_100Mbps/independent_noncommon_policer_2.5Mbps_0.03s_5',
                             app_type=4, pkt_size=1228, seed=3,
                             is_neutral=4, policing_rate=2.5, burst_length=0.03, scenario=6)
    ])
