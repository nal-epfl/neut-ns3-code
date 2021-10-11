# This file is for experiments done after Sep 2021 related to tomography inference based on loss
from project_run_env.RunConfig import *
from data.data_preparation import *


def run_poisson_experiment(link_rate, duration, is_tcp, tcp_protocol='TcpCubic', seed=3, scenario=0, exp_bacth='', pkt_size=1228, p_lambda=0.001, replay_trace=""):
    # run the ns3 simulation
    result_folder_name = '/9_2021/tomo_with_loss/' + app_name + '/link_' + link_rate + '/' + exp_bacth
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



if __name__ == '__main__':
    app_name = 'Poisson_Probes'
    results_folder = 'wout_background'

    # run_poisson_experiment(link_rate='250Mbps', duration=600, is_tcp=0, exp_bacth='back_traffic_long/constant_1ms_pktSize_1228/same_constant', scenario=1, pkt_size=1228, p_lambda=0.001)
    # run_poisson_experiment(link_rate='250Mbps', duration=600, is_tcp=0, exp_bacth='back_traffic_long/lambda_1ms_pktSize_1228/2_different_poisson', scenario=2, pkt_size=1228, p_lambda=0.001)
    run_poisson_experiment(link_rate='250Mbps', duration=600, is_tcp=0, exp_bacth='back_traffic_long/lambda_1ms_pktSize_1228/same_poisson', scenario=3, replay_trace="poisson_trace_lambda_1ms_pktSize_1228_10min")

    # run_poisson_experiment(link_rate='280Mbps', duration=600, is_tcp=0, exp_bacth='back_traffic_long/constant_1ms_pktSize_1228/same_constant', scenario=1, pkt_size=1228, p_lambda=0.001)
    # run_poisson_experiment(link_rate='280Mbps', duration=600, is_tcp=0, exp_bacth='back_traffic_long/lambda_1ms_pktSize_1228/2_different_poisson', scenario=2, pkt_size=1228, p_lambda=0.001)
    run_poisson_experiment(link_rate='280Mbps', duration=600, is_tcp=0, exp_bacth='back_traffic_long/lambda_1ms_pktSize_1228/same_poisson', scenario=3, replay_trace="poisson_trace_lambda_1ms_pktSize_1228_10min")

    # run_poisson_experiment(link_rate='200Mbps', duration=600, is_tcp=0, exp_bacth='back_traffic_long/constant_1ms_pktSize_1228/same_constant', scenario=1, pkt_size=1228, p_lambda=0.001)
    # run_poisson_experiment(link_rate='200Mbps', duration=600, is_tcp=0, exp_bacth='back_traffic_long/lambda_1ms_pktSize_1228/2_different_poisson', scenario=2, pkt_size=1228, p_lambda=0.001)
    run_poisson_experiment(link_rate='200Mbps', duration=600, is_tcp=0, exp_bacth='back_traffic_long/lambda_1ms_pktSize_1228/same_poisson', scenario=3, replay_trace="poisson_trace_lambda_1ms_pktSize_1228_10min")


# run_poisson_experiment(link_rate='250Mbps', duration=600, is_tcp=0, exp_bacth='caida_traffic_only', pkt_size=1228, p_lambda=0.001)
    # run_poisson_experiment(link_rate='220Mbps', duration=600, is_tcp=0, exp_bacth='caida_traffic_only', pkt_size=256, p_lambda=0.001)
    
    # run_poisson_experiment(link_rate='250Mbps', duration=600, is_tcp=0, exp_bacth='back_traffic_long/lambda_10ms_pktSize_1228/2_different_poisson', pkt_size=1228, p_lambda=0.01)
    # run_poisson_experiment(link_rate='250Mbps', duration=60, is_tcp=0, exp_bacth='lambda_1ms_pktSize_1228/same_poisson', pkt_size=1228, p_lambda=0.001)
    # run_poisson_experiment(link_rate='250Mbps', duration=60, is_tcp=0, exp_bacth='lambda_10ms_pktSize_1228/same_poisson', pkt_size=1228, p_lambda=0.01)



    # run_poisson_experiment(link_rate='20Mbps', duration=10, is_tcp=1, tcp_protocol='TcpCubic')