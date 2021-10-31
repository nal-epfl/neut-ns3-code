# This file is for experiments done after Sep 2021 related to tomography inference based on loss
from project_run_env.RunConfig import *
from multiprocessing import Process

app_name = 'Poisson_Probes'


class ExperimentParameters:
    def __init__(self, link_rate, duration, is_tcp=0, tcp_protocol='TcpCubic', seed=3, scenario=0, exp_batch='',
                 pkt_size=1228, p_lambda=0.001, replay_trace='empty', is_neutral=1, policing_rate=0.0,
                 burst_length=0.0):
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
        self.is_neutral = is_neutral
        self.policing_rate = policing_rate
        self.burst_length = burst_length


def run_probing_experiment_with_params(params):
    run_probing_experiment(link_rate=params.link_rate, duration=params.duration, is_tcp=params.is_tcp,
                           tcp_protocol=params.tcp_protocol, seed=params.seed, scenario=params.scenario,
                           exp_batch=params.exp_batch, pkt_size=params.pkt_size, p_lambda=params.p_lambda,
                           replay_trace=params.replay_trace, is_neutral=params.is_neutral,
                           policing_rate=params.policing_rate, burst_length=params.burst_length)


def run_probing_experiment(link_rate, duration, is_tcp, tcp_protocol='TcpCubic', seed=3, scenario=0, exp_batch='',
                           pkt_size=1228, p_lambda=0.001, replay_trace='empty', is_neutral=1, policing_rate=0.0,
                           burst_length=0.0):
    # run the ns3 simulation
    result_folder_name = '/10_2021/neut_with_loss/' + app_name + '/link_' + link_rate + '/' + exp_batch
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
              " --scenario=" + str(scenario) +
              " --pktSize=" + str(pkt_size) +
              " --lambda=" + str(p_lambda) +
              " --replayTrace=" + replay_trace +
              " --neutral=" + str(is_neutral) +
              " --policingRate=" + str(policing_rate) +
              " --policingBurstLength=" + str(burst_length) +
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

    run_parallel_experiments([
        ExperimentParameters(link_rate="100Mbps", duration=20, is_tcp=0,
                             exp_batch='test_policing',
                             scenario=3, replay_trace='poisson_trace_lambda_2.5ms_pktSize_1228_10min',
                             is_neutral=0, policing_rate=2, burst_length=0.03)
    ])

    # for link_rate in ['210Mbps', '250Mbps', '280Mbps']:
    #     run_parallel_experiments([
    #         ExperimentParameters(link_rate=link_rate, duration=590, is_tcp=0,
    #                              exp_batch='back_traffic_long/wehe/WhatsApp_04112019/perPath_policer_3Mbps_0.03s',
    #                              scenario=3, replay_trace='Very_Long_WhatsApp_041120198_packetMeta_processed',
    #                              is_neutral=0, policing_rate=3, burst_length=0.03)
    #     ])
    #
    # for link_rate in ['210Mbps', '250Mbps', '280Mbps']:
    #     run_parallel_experiments([
    #         ExperimentParameters(link_rate=link_rate, duration=590, is_tcp=0,
    #                              exp_batch='back_traffic_long/wehe/Skype_12122018/perPath_policer_3Mbps_0.03s',
    #                              scenario=3, replay_trace='Very_Long_Skype_12122018_packetMeta_processed',
    #                              is_neutral=0, policing_rate=3, burst_length=0.03)
    #     ])

    # for link_rate in ['210Mbps', '250Mbps', '280Mbps']:
    #     run_parallel_experiments([
    #         ExperimentParameters(link_rate=link_rate, duration=590, is_tcp=0,
    #                              exp_batch='back_traffic_long/badabing/perPath_policer_3Mbps_0.03s',
    #                              scenario=3, replay_trace='badabing_probes',
    #                              is_neutral=0, policing_rate=3, burst_length=0.03)
    #     ])
    #
    # for link_rate in ['210Mbps', '250Mbps', '280Mbps']:
    #     run_parallel_experiments([
    #         ExperimentParameters(link_rate=link_rate, duration=590, is_tcp=0,
    #                              exp_batch='back_traffic_long/constant_2.5ms_pktSize_1228/perPath_policer_3Mbps_0.03s',
    #                              scenario=1, pkt_size=1228, p_lambda=0.0025,
    #                              is_neutral=0, policing_rate=3, burst_length=0.03),
    #     ])
    #
    # for link_rate in ['210Mbps', '250Mbps', '280Mbps']:
    #     run_parallel_experiments([
    #         ExperimentParameters(link_rate='210Mbps', duration=590, is_tcp=0,
    #                              exp_batch='back_traffic_long/lambda_2.5ms_pktSize_1228/perPath_policer_3Mbps_0.03s',
    #                              scenario=3, replay_trace='poisson_trace_lambda_2.5ms_pktSize_1228_10min',
    #                              is_neutral=0, policing_rate=3, burst_length=0.03)
    #     ])



    # # constant rate probing + poisson replay
    # run_parallel_experiments([
    #     ExperimentParameters(link_rate='210Mbps', duration=590, is_tcp=0,
    #                          exp_batch='back_traffic_long/constant_2.5ms_pktSize_1228/perPath_policer_3Mbps_0.03s',
    #                          scenario=1, pkt_size=1228, p_lambda=0.0025,
    #                          is_neutral=0, policing_rate=3, burst_length=0.03),
    #     ExperimentParameters(link_rate='250Mbps', duration=590, is_tcp=0,
    #                          exp_batch='back_traffic_long/constant_2.5ms_pktSize_1228/perPath_policer_3Mbps_0.03s',
    #                          scenario=1, pkt_size=1228, p_lambda=0.0025,
    #                          is_neutral=0, policing_rate=3, burst_length=0.03),
    #     ExperimentParameters(link_rate='280Mbps', duration=590, is_tcp=0,
    #                          exp_batch='back_traffic_long/constant_2.5ms_pktSize_1228/perPath_policer_3Mbps_0.03s',
    #                          scenario=1, pkt_size=1228, p_lambda=0.0025,
    #                          is_neutral=0, policing_rate=3, burst_length=0.03),
    #
    #     ExperimentParameters(link_rate='210Mbps', duration=590, is_tcp=0,
    #                          exp_batch='back_traffic_long/lambda_2.5ms_pktSize_1228/perPath_policer_3Mbps_0.03s',
    #                          scenario=3, replay_trace='poisson_trace_lambda_2.5ms_pktSize_1228_10min',
    #                          is_neutral=0, policing_rate=3, burst_length=0.03),
    #     ExperimentParameters(link_rate='250Mbps', duration=590, is_tcp=0,
    #                          exp_batch='back_traffic_long/lambda_2.5ms_pktSize_1228/perPath_policer_3Mbps_0.03s',
    #                          scenario=3, replay_trace='poisson_trace_lambda_2.5ms_pktSize_1228_10min',
    #                          is_neutral=0, policing_rate=3, burst_length=0.03),
    #     ExperimentParameters(link_rate='280Mbps', duration=590, is_tcp=0,
    #                          exp_batch='back_traffic_long/lambda_2.5ms_pktSize_1228/perPath_policer_3Mbps_0.03s',
    #                          scenario=3, replay_trace='poisson_trace_lambda_2.5ms_pktSize_1228_10min',
    #                          is_neutral=0, policing_rate=3, burst_length=0.03),
    #
    # ])

    # # badabing + wehe replay
    # run_parallel_experiments([
    #     ExperimentParameters(link_rate='210Mbps', duration=590, is_tcp=0,
    #                          exp_batch='back_traffic_long/badabing/perPath_policer_3Mbps_0.03s',
    #                          scenario=3, replay_trace='badabing_probes',
    #                          is_neutral=0, policing_rate=3, burst_length=0.03),
    #     ExperimentParameters(link_rate='250Mbps', duration=590, is_tcp=0,
    #                          exp_batch='back_traffic_long/badabing/perPath_policer_3Mbps_0.03s',
    #                          scenario=3, replay_trace='badabing_probes',
    #                          is_neutral=0, policing_rate=3, burst_length=0.03),
    #     ExperimentParameters(link_rate='280Mbps', duration=590, is_tcp=0,
    #                          exp_batch='back_traffic_long/badabing/perPath_policer_3Mbps_0.03s',
    #                          scenario=3, replay_trace='badabing_probes',
    #                          is_neutral=0, policing_rate=3, burst_length=0.03),
    #
    #     ExperimentParameters(link_rate='210Mbps', duration=590, is_tcp=0,
    #                          exp_batch='back_traffic_long/wehe/WhatsApp_04112019/perPath_policer_3Mbps_0.03s',
    #                          scenario=3, replay_trace='Very_Long_WhatsApp_041120198_packetMeta_processed',
    #                          is_neutral=0, policing_rate=3, burst_length=0.03),
    #     ExperimentParameters(link_rate='250Mbps', duration=590, is_tcp=0,
    #                          exp_batch='back_traffic_long/wehe/WhatsApp_04112019/perPath_policer_3Mbps_0.03s',
    #                          scenario=3, replay_trace='Very_Long_WhatsApp_04112019_packetMeta_processed',
    #                          is_neutral=0, policing_rate=3, burst_length=0.03),
    #     ExperimentParameters(link_rate='280Mbps', duration=590, is_tcp=0,
    #                          exp_batch='back_traffic_long/wehe/WhatsApp_04112019/perPath_policer_3Mbps_0.03s',
    #                          scenario=3, replay_trace='Very_Long_WhatsApp_04112019_packetMeta_processed',
    #                          is_neutral=0, policing_rate=3, burst_length=0.03),
    # ])

    # # wehe replay
    # run_parallel_experiments([
    #     ExperimentParameters(link_rate='210Mbps', duration=590, is_tcp=0,
    #                          exp_batch='back_traffic_long/wehe/Skype_12122018/perPath_policer_3Mbps_0.03s',
    #                          scenario=3, replay_trace='Very_Long_Skype_12122018_packetMeta_processed',
    #                          is_neutral=0, policing_rate=3, burst_length=0.03),
    #     ExperimentParameters(link_rate='250Mbps', duration=590, is_tcp=0,
    #                          exp_batch='back_traffic_long/wehe/Skype_12122018/perPath_policer_3Mbps_0.03s',
    #                          scenario=3, replay_trace='Very_Long_Skype_12122018_packetMeta_processed',
    #                          is_neutral=0, policing_rate=3, burst_length=0.03),
    #     ExperimentParameters(link_rate='280Mbps', duration=590, is_tcp=0,
    #                          exp_batch='back_traffic_long/wehe/Skype_12122018/perPath_policer_3Mbps_0.03s',
    #                          scenario=3, replay_trace='Very_Long_Skype_12122018_packetMeta_processed',
    #                          is_neutral=0, policing_rate=3, burst_length=0.03),
    # ])
