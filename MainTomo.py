# This file is for experiments done after Sep 2021 related to tomography inference based on loss
from project_run_env.RunConfig import *
from multiprocessing import Process

app_name = 'Poisson_Probes'


class ExperimentParameters:
    def __init__(self, link_rate, duration, is_tcp=0, tcp_protocol='TcpCubic', seed=3, scenario=0, exp_batch='',
                 pkt_size=1228, p_lambda=0.001, replay_trace='empty'):
        self.link_rate = link_rate
        self.duration = duration
        self.is_tcp = is_tcp
        self.tcp_protocol = tcp_protocol
        self.seed = seed
        self.scenario = scenario
        self.exp_batch = exp_batch
        self.pkt_size = pkt_size
        self.p_lambda = p_lambda
        self.replay_trace = replay_trace


def run_probing_experiment_with_params(params):
    run_probing_experiment(params.link_rate, params.duration, params.is_tcp, params.tcp_protocol, params.seed,
                           params.app_type, params.exp_batch, params.pkt_size, params.p_lambda, params.replay_trace)


def run_probing_experiment(link_rate, duration, is_tcp, tcp_protocol='TcpCubic', seed=3, scenario=0, exp_batch='',
                           pkt_size=1228, p_lambda=0.001, replay_trace='empty'):
    # run the ns3 simulation
    result_folder_name = '/9_2021/tomo_with_loss/' + app_name + '/link_' + link_rate + '/' + exp_batch
    if is_tcp:
        result_folder_name = result_folder_name + '/' + tcp_protocol
    else:
        result_folder_name = result_folder_name + '/udp'
    results_path = get_ns3_path() + '/scratch/wehe_p_tomography/results' + result_folder_name

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
              " --scenario=" + str(scenario) +
              " --pktSize=" + str(pkt_size) +
              " --lambda=" + str(p_lambda) +
              " --replayTrace=" + replay_trace +
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

    # constant rate probing + poisson replay
    run_parallel_experiments([
        ExperimentParameters(link_rate='200Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
                             exp_batch='back_traffic_long/infinite_tcp',
                             scenario=1, pkt_size=1228, p_lambda=0),
        ExperimentParameters(link_rate='250Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
                             exp_batch='back_traffic_long/infinite_tcp',
                             scenario=1, pkt_size=1228, p_lambda=0),
        ExperimentParameters(link_rate='280Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
                             exp_batch='back_traffic_long/infinite_tcp',
                             scenario=1, pkt_size=1228, p_lambda=0)
    ])
    #
    #     ExperimentParameters(link_rate='200Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
    #                          exp_batch='back_traffic_long/lambda_2.5ms_pktSize_1228/same_poisson',
    #                          scenario=3, replay_trace="poisson_trace_lambda_2.5ms_pktSize_1228_10min"),
    #     ExperimentParameters(link_rate='250Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
    #                          exp_batch='back_traffic_long/lambda_2.5ms_pktSize_1228/same_poisson',
    #                          scenario=3, replay_trace="poisson_trace_lambda_2.5ms_pktSize_1228_10min"),
    #     ExperimentParameters(link_rate='280Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
    #                          exp_batch='back_traffic_long/lambda_2.5ms_pktSize_1228/same_poisson',
    #                          scenario=3, replay_trace="poisson_trace_lambda_2.5ms_pktSize_1228_10min"),
    #
    # ])

    # # badabing + wehe replay - Done
    # run_parallel_experiments([
    #     ExperimentParameters(link_rate='200Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
    #                          exp_batch='back_traffic_long/badabing',
    #                          scenario=3, replay_trace="badabing_probes"),
    #     ExperimentParameters(link_rate='250Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
    #                          exp_batch='back_traffic_long/badabing',
    #                          scenario=3, replay_trace="badabing_probes"),
    #     ExperimentParameters(link_rate='280Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
    #                          exp_batch='back_traffic_long/badabing',
    #                          scenario=3, replay_trace="badabing_probes"),
    #
    #     ExperimentParameters(link_rate='200Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
    #                          exp_batch='back_traffic_long/wehe/Netflix_12122018',
    #                          scenario=3, replay_trace="Very_Long_Netflix_12122018_packetMeta_processed_new"),
    #     ExperimentParameters(link_rate='250Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
    #                          exp_batch='back_traffic_long/wehe/Netflix_12122018',
    #                          scenario=3, replay_trace="Very_Long_Netflix_12122018_packetMeta_processed_new"),
    #     ExperimentParameters(link_rate='280Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
    #                          exp_batch='back_traffic_long/wehe/Netflix_12122018',
    #                          scenario=3, replay_trace="Very_Long_Netflix_12122018_packetMeta_processed_new"),
    # ])

    # # wehe replay
    # run_parallel_experiments([
    #     ExperimentParameters(link_rate='200Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
    #                          exp_batch='back_traffic_long/wehe/Amazon_01042019',
    #                          scenario=3, replay_trace="Very_Long_Amazon_01042019_packetMeta_processed"),
    #     ExperimentParameters(link_rate='250Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
    #                          exp_batch='back_traffic_long/wehe/Amazon_01042019',
    #                          scenario=3, replay_trace="Very_Long_Amazon_01042019_packetMeta_processed"),
    #     ExperimentParameters(link_rate='280Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
    #                          exp_batch='back_traffic_long/wehe/Amazon_01042019',
    #                          scenario=3, replay_trace="Very_Long_Amazon_01042019_packetMeta_processed"),
    #
    #
    #     ExperimentParameters(link_rate='200Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
    #                          exp_batch='back_traffic_long/wehe/FacebookVideo_04112019',
    #                          scenario=3, replay_trace="Very_Long_FacebookVideo_04112019_packetMeta_processed"),
    #     ExperimentParameters(link_rate='250Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
    #                          exp_batch='back_traffic_long/wehe/FacebookVideo_04112019',
    #                          scenario=3, replay_trace="Very_Long_FacebookVideo_04112019_packetMeta_processed"),
    #     ExperimentParameters(link_rate='280Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
    #                          exp_batch='back_traffic_long/wehe/FacebookVideo_04112019',
    #                          scenario=3, replay_trace="Very_Long_FacebookVideo_04112019_packetMeta_processed"),
    # ])