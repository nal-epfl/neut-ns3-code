# This file is for experiments done after Sep 2021 related to tomography inference based on loss
from project_run_env.RunConfig import *
from multiprocessing import Process


class ExperimentParameters:
    def __init__(self, link_rate, duration, is_tcp=0, tcp_protocol='TcpCubic', seed=3,
                 app_type=0, app_name='Poisson_Probes', pkt_size=1228, p_lambda=0.001, replay_trace='empty',
                 exp_batch='', scenario=0, is_neutral=0, policing_rate=0.0, burst_length=0.0, throttle_udp=0):
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
        self.exp_batch = exp_batch
        self.scenario = scenario
        self.is_neutral = is_neutral
        self.policing_rate = policing_rate
        self.burst_length = burst_length
        self.throttle_udp = throttle_udp


def run_probing_experiment_with_params(params):
    run_probing_experiment(link_rate=params.link_rate, duration=params.duration,
                           is_tcp=params.is_tcp, tcp_protocol=params.tcp_protocol, seed=params.seed,
                           app_type=params.app_type, app_name=params.app_name,
                           pkt_size=params.pkt_size, p_lambda=params.p_lambda, replay_trace=params.replay_trace,
                           exp_batch=params.exp_batch, scenario=params.scenario,
                           is_neutral=params.is_neutral, policing_rate=params.policing_rate,
                           burst_length=params.burst_length, throttle_udp=params.throttle_udp)


def run_probing_experiment(link_rate, duration, is_tcp, tcp_protocol='TcpCubic', seed=3,
                           app_type=0, app_name='Poisson_Probes', pkt_size=1228, p_lambda=0.001, replay_trace='empty',
                           exp_batch='', scenario=0, is_neutral=0, policing_rate=0.0, burst_length=0.0, throttle_udp=0):
    # run the ns3 simulation
    result_folder_name = '/12_2021/neut_with_loss/' + app_name + '/link_' + link_rate + '/' + exp_batch + '/seed_' + str(seed)
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
              " --linkRate=" + link_rate +
              " --duration=" + str(duration) +
              " --resultsFolderName=" + result_folder_name +
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

    exps = [
        ('back_traffic_long_3/ideal', 0),
        ('back_traffic_long_3/differentRTTs_onp3_8ms', 3),
        ('back_traffic_long_3/nonCommonCongestion_onp3_100Mbps', 5),
        ('back_traffic_long_3/nonCommonCongestion_onp3_150Mbps_onp4_100Mbps', 6)
    ]

    for case, scenario in exps:
        print(case, scenario)
        run_parallel_experiments([
            ExperimentParameters(link_rate="210Mbps", duration=240, is_tcp=1, tcp_protocol='TcpCubic', seed=3,
                                 app_type=4, app_name='Infinite_Tcp', pkt_size=1228,
                                 exp_batch=case+'/neutral', scenario=scenario,
                                 is_neutral=0),
            ExperimentParameters(link_rate="210Mbps", duration=240, is_tcp=1, tcp_protocol='TcpCubic', seed=3,
                                 app_type=4, app_name='Infinite_Tcp', pkt_size=1228,
                                 exp_batch=case+'/single_policer_40Mbps_0.03s_80p', scenario=scenario,
                                 is_neutral=1, policing_rate=40, burst_length=0.03),
            ExperimentParameters(link_rate="210Mbps", duration=240, is_tcp=1, tcp_protocol='TcpCubic', seed=3,
                                 app_type=4, app_name='Infinite_Tcp', pkt_size=1228,
                                 exp_batch=case+'/single_noncommon_policer_20Mbps_0.03s_80p', scenario=scenario,
                                 is_neutral=3, policing_rate=20, burst_length=0.03),
            ExperimentParameters(link_rate="210Mbps", duration=240, is_tcp=1, tcp_protocol='TcpCubic', seed=3,
                                 app_type=4, app_name='Infinite_Tcp', pkt_size=1228,
                                 exp_batch=case+'/single_noncommon_policer_onp4_20Mbps_0.03s_80p', scenario=scenario,
                                 is_neutral=5, policing_rate=20, burst_length=0.03),
        ])
