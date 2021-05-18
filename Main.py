from project_run_env.RunConfig import *
from data.data_preparation import *


wehe_apps_names = ['Amazon_01042019', 'Amazon_12122018', 'AppleMusic_04112019', 'FacebookVideo_04112019',
                   'Hulu_04112019', 'NBCSports_01042019', 'NBCSports_12122018', 'Netflix_12122018',
                   'Skype_12122018', 'Spotify_01042019', 'Spotify_12122018', 'Twitch_04112019', 'Vimeo_12122018',
                   'WhatsApp_04112019', 'Youtube_12122018']

def run_weheCS_udp_experiment(app_name, link_rate, is_neutral=1, policing_rate=0.0, burst_length=0.0, case='neutral', seed=3, scenario=0):
    # prepare the wehe trace
    project_data_path = get_ns3_path() + "/scratch/wehe_p_tomography/data"
    generate_weheCS_trace(project_data_path, app_name, 'weheCS_trace')
    duration = get_weheCS_duration(project_data_path + '/weheCS_trace')

    # run the ns3 simulation
    result_folder_name = '/3_2021/wehe_tests/' + app_name + '/link_' + link_rate + '/' + case
    results_path = get_ns3_path() + '/scratch/wehe_p_tomography/results' + result_folder_name

    os.system("mkdir -p " + results_path)
    os.system(get_ns3_path() + "/waf --run \"wehe_p_tomography\"  --command-template=\"%s" +
              " --RngSeed=" + str(seed) +
              " --RngRun=1" +
              " --duration=" + str(duration) +
              " --resultsFolderName=" + result_folder_name +
              " --linkRate=" + link_rate +
              " --weheAppProtocol=0" +
              " --neutral=" + str(is_neutral) +
              " --policingRate=" + str(policing_rate) +
              " --policingBurstLength=" + str(burst_length) +
              " --scenario=" + str(scenario) +
              "\"")


def run_weheCS_tcp_experiment(app_name, link_rate, tcp_protocol, is_neutral=1, policing_rate=0.0, burst_length=0.0, case='neutral', seed=3, scenario=0):
    # prepare the wehe trace
    project_data_path = get_ns3_path() + "/scratch/wehe_p_tomography/data"
    generate_weheCS_trace(project_data_path, app_name, 'weheCS_trace')
    duration = 15 #get_weheCS_duration(project_data_path + '/weheCS_trace')

    # run the ns3 simulation
    result_folder_name = '/5_2021/wehe_tests/' + app_name + '/link_' + link_rate + '/' + case + '/' + tcp_protocol
    results_path = get_ns3_path() + '/scratch/wehe_p_tomography/results' + result_folder_name

    os.system("mkdir -p " + results_path)
    os.system(get_ns3_path() + "/waf --run \"wehe_p_tomography\"  --command-template=\"%s" +
              " --RngSeed=" + str(seed) +
              " --RngRun=1" +
              " --duration=" + str(duration) +
              " --resultsFolderName=" + result_folder_name +
              " --linkRate=" + link_rate +
              " --weheAppProtocol=1" +
              " --TCPProtocol=ns3::" + tcp_protocol +
              " --neutral=" + str(is_neutral) +
              " --policingRate=" + str(policing_rate) +
              " --policingBurstLength=" + str(burst_length) +
              " --scenario=" + str(scenario) +
              "\"")


def generate_README(exp_batch, exp_description, app_name, link_rate):
    results_path = (get_ns3_path() + '/scratch/wehe_p_tomography/results/3_2021/wehe_tests/'
                    + app_name + '/link_' + link_rate + '/' + exp_batch)

    os.system("mkdir -p " + results_path)
    f = open(results_path + '/README.txt', "w")
    f.write(exp_description)
    f.close()


def run_TCP_test_experiment(link_rate, tcp_protocol, case, duration, seed=3, scenario=0, tcp_rate='2Mbps'):
    # run the ns3 simulation
    result_folder_name = '/3_2021/tcp_tests/Continuous_TCP_Flows/link_' + link_rate + '/' + case + '/' + tcp_protocol
    results_path = get_ns3_path() + '/scratch/wehe_p_tomography/results' + result_folder_name

    os.system("mkdir -p " + results_path)
    os.system(get_ns3_path() + "/waf --run \"wehe_p_tomography\"  --command-template=\" %s" +
              " --RngSeed=" + str(seed) +
              " --RngRun=" + str(seed) +
              " --duration=" + str(duration) +
              " --resultsFolderName=" + result_folder_name +
              " --linkRate=" + link_rate +
              " --weheAppProtocol=1" +
              " --TCPProtocol=ns3::" + tcp_protocol +
              " --scenario=" + str(scenario) +
              " --tcpRate=" + tcp_rate +
              "\"")


def run_TCP_test_experiment_policing(link_rate, tcp_protocol, case, duration, policing_rate, burst_length, seed=3, scenario=0, tcp_rate='2Mbps'):
    # run the ns3 simulation
    result_folder_name = '/3_2021/tcp_tests/Continuous_TCP_Flows/link_' + link_rate + '/' + case + '/' + tcp_protocol
    results_path = get_ns3_path() + '/scratch/wehe_p_tomography/results' + result_folder_name

    os.system("mkdir -p " + results_path)
    os.system(get_ns3_path() + "/waf --run \"wehe_p_tomography\"  --command-template=\"%s" +
              " --RngSeed=" + str(seed) +
              " --RngRun=" + str(seed) +
              " --duration=" + str(duration) +
              " --resultsFolderName=" + result_folder_name +
              " --linkRate=" + link_rate +
              " --weheAppProtocol=1" +
              " --TCPProtocol=ns3::" + tcp_protocol +
              " --neutral=0" +
              " --policingRate=" + policing_rate +
              " --policingBurstLength=" + str(burst_length) +
              " --scenario=" + str(scenario) +
              " --tcpRate=" + tcp_rate +
              "\"")


if __name__ == '__main__':

    #
    # exp_batch = 'back_traffic_9/without_time_pacing_and_with_flow_control/exp_batch_1'
    # app_name, link_rate, scenario = 'Very_Long_Netflix_12122018', '850Mbps', 1
    # run_weheCS_tcp_experiment(app_name, link_rate, "TcpCubic", is_neutral=1, case=exp_batch + '/neutral/seed_3', seed=3, scenario=scenario)
    # run_weheCS_tcp_experiment(app_name, link_rate, "TcpCubic", is_neutral=0, policing_rate=3.7, burst_length=0.03,
    #                           case=exp_batch + '/perFlow_policer_3.7Mbps_0.03s/seed_3', seed=3, scenario=scenario)
    # run_weheCS_tcp_experiment(app_name, link_rate, "TcpCubic", is_neutral=1, case=exp_batch + '/neutral/seed_7', seed=7, scenario=scenario)
    # run_weheCS_tcp_experiment(app_name, link_rate, "TcpCubic", is_neutral=0, policing_rate=3.7, burst_length=0.03,
    #                           case=exp_batch + '/perFlow_policer_3.7Mbps_0.03s/seed_7', seed=7, scenario=scenario)
    #
    #
    # exp_batch = 'back_traffic_9/without_time_pacing_and_with_flow_control/exp_batch_1'
    # app_name, link_rate, scenario = 'Very_Long_Netflix_12122018', '900Mbps', 1
    # run_weheCS_tcp_experiment(app_name, link_rate, "TcpCubic", is_neutral=1, case=exp_batch + '/neutral/seed_3', seed=3, scenario=scenario)
    # run_weheCS_tcp_experiment(app_name, link_rate, "TcpCubic", is_neutral=0, policing_rate=3.7, burst_length=0.03,
    #                           case=exp_batch + '/perFlow_policer_3.7Mbps_0.03s/seed_3', seed=3, scenario=scenario)
    # run_weheCS_tcp_experiment(app_name, link_rate, "TcpCubic", is_neutral=1, case=exp_batch + '/neutral/seed_7', seed=7, scenario=scenario)
    # run_weheCS_tcp_experiment(app_name, link_rate, "TcpCubic", is_neutral=0, policing_rate=3.7, burst_length=0.03,
    #                           case=exp_batch + '/perFlow_policer_3.7Mbps_0.03s/seed_7', seed=7, scenario=scenario)


    # exp_batch = 'back_traffic_9/without_time_pacing_and_with_flow_control/exp_batch_2'
    # app_name, link_rate, scenario = 'Very_Long_Netflix_12122018', '850Mbps', 3
    # run_weheCS_tcp_experiment(app_name, link_rate, "TcpCubic", is_neutral=1, case=exp_batch + '/neutral/seed_3', seed=3, scenario=scenario)
    # run_weheCS_tcp_experiment(app_name, link_rate, "TcpCubic", is_neutral=0, policing_rate=3.7, burst_length=0.03,
    #                           case=exp_batch + '/perFlow_policer_3.7Mbps_0.03s/seed_3', seed=3, scenario=scenario)
    # run_weheCS_tcp_experiment(app_name, link_rate, "TcpCubic", is_neutral=1, case=exp_batch + '/neutral/seed_7', seed=7, scenario=scenario)
    # run_weheCS_tcp_experiment(app_name, link_rate, "TcpCubic", is_neutral=0, policing_rate=3.7, burst_length=0.03,
    #                           case=exp_batch + '/perFlow_policer_3.7Mbps_0.03s/seed_7', seed=7, scenario=scenario)
    #
    #
    # exp_batch = 'back_traffic_9/without_time_pacing_and_with_flow_control/exp_batch_2'
    # app_name, link_rate, scenario = 'Very_Long_Netflix_12122018', '900Mbps', 3
    # run_weheCS_tcp_experiment(app_name, link_rate, "TcpCubic", is_neutral=1, case=exp_batch + '/neutral/seed_3', seed=3, scenario=scenario)
    # run_weheCS_tcp_experiment(app_name, link_rate, "TcpCubic", is_neutral=0, policing_rate=3.7, burst_length=0.03,
    #                           case=exp_batch + '/perFlow_policer_3.7Mbps_0.03s/seed_3', seed=3, scenario=scenario)
    # run_weheCS_tcp_experiment(app_name, link_rate, "TcpCubic", is_neutral=1, case=exp_batch + '/neutral/seed_7', seed=7, scenario=scenario)
    # run_weheCS_tcp_experiment(app_name, link_rate, "TcpCubic", is_neutral=0, policing_rate=3.7, burst_length=0.03,
    #                           case=exp_batch + '/perFlow_policer_3.7Mbps_0.03s/seed_7', seed=7, scenario=scenario)


    exp_batch = 'back_traffic_8/without_time_pacing_and_with_flow_control/exp_batch_4'
    app_name, link_rate, scenario = 'Very_Long_Netflix_12122018', '1000Mbps', 4
    run_weheCS_tcp_experiment(app_name, link_rate, "TcpCubic", is_neutral=1, case=exp_batch + '/neutral/seed_3', seed=3, scenario=scenario)
    # run_weheCS_tcp_experiment(app_name, link_rate, "TcpCubic", is_neutral=0, policing_rate=3.7, burst_length=0.03,
    #                           case=exp_batch + '/perFlow_policer_3.7Mbps_0.03s/seed_3', seed=3, scenario=scenario)
    # run_weheCS_tcp_experiment(app_name, link_rate, "TcpCubic", is_neutral=1, case=exp_batch + '/neutral/seed_7', seed=7, scenario=scenario)
    # run_weheCS_tcp_experiment(app_name, link_rate, "TcpCubic", is_neutral=0, policing_rate=3.7, burst_length=0.03,
    #                           case=exp_batch + '/perFlow_policer_3.7Mbps_0.03s/seed_7', seed=7, scenario=scenario)


    # exp_description = ('The topology of this experiment is as follow:\n'
    #                    + '             +___ d1\n'
    #                    + '             +___ d2\n'
    #                    + ' s1 --- r1 __+___ d3\n'
    #                    + '             +___ d4\n'
    #                    + '             +___ d5\n'
    #                    + 'We have 4 ' + app_name + ' Clinet-Server Wehe traces (2 control + 2 original)\n'
    #                    + 'With one path sending CAIDA background traffic (UDP + multiple TCP)\n'
    #                    + 'The bottleneck has a bandwidth of ' + link_rate + '\n'
    #                    + 'All links have a propagation delay of 10ms\n')
    # generate_README(exp_batch, exp_description, app_name, link_rate)
