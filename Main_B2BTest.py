from project_run_env.RunConfig import *
from data.data_preparation import *


# app_protocol : 0 for udp and 1 for tcp
def run_udp_wehe_experiment(app_name, link_rate, case='neutral'):
    # prepare the wehe trace
    project_data_path = get_ns3_path() + "/scratch/wehe_p_tomography/data"
    wehe_to_ns3trace(project_data_path + '/' + app_name + '_packetMeta_server', project_data_path + '/wehe_trace_server')
    duration = get_ns3trace_duration(project_data_path + '/wehe_trace_server')

    # run the ns3 simulation
    result_folder_name = '/2_2021/' + app_name + '/link_' + link_rate + '/' + case + '/back_w_wehe'
    results_path = get_ns3_path() + '/scratch/wehe_p_tomography/results' + result_folder_name

    os.system("mkdir -p " + results_path)
    os.system(get_ns3_path() + "/waf --run \"wehe_p_tomography\"  --command-template=\"%s" +
              " --RngSeed=" + str(5) +
              " --RngRun=1" +
              " --duration=" + str(duration) +
              " --resultsFolderName=" + result_folder_name +
              " --LinkRate=" + link_rate +
              " --WeheAppProtocol=0"
              "\"")


def run_tcp_wehe_experiment(app_name, link_rate, tcp_protocol, case='neutral'):
    # prepare the wehe trace
    project_data_path = get_ns3_path() + "/scratch/wehe_p_tomography/data"
    wehe_to_ns3trace(project_data_path + '/' + app_name + '_packetMeta_server', project_data_path + '/wehe_trace_server')
    duration = get_ns3trace_duration(project_data_path + '/wehe_trace_server')

    # run the ns3 simulation
    result_folder_name = '/2_2021/' + app_name + '/link_' + link_rate + '/' + case + '/back_w_wehe/' + tcp_protocol
    results_path = get_ns3_path() + '/scratch/wehe_p_tomography/results' + result_folder_name

    os.system("mkdir -p " + results_path)
    os.system(get_ns3_path() + "/waf --run \"wehe_p_tomography\"  --command-template=\"%s" +
              " --RngSeed=" + str(5) +
              " --RngRun=1" +
              " --duration=" + str(duration) +
              " --resultsFolderName=" + result_folder_name +
              " --LinkRate=" + link_rate +
              " --WeheAppProtocol=1" +
              " --TCPProtocol=ns3::" + tcp_protocol +
              "\"")


wehe_apps_names = ['Amazon_01042019', 'Amazon_12122018', 'AppleMusic_04112019', 'FacebookVideo_04112019',
                   'Hulu_04112019', 'NBCSports_01042019', 'NBCSports_12122018', 'Netflix_12122018',
                   'Skype_12122018', 'Spotify_01042019', 'Spotify_12122018', 'Twitch_04112019', 'Vimeo_12122018',
                   'WhatsApp_04112019', 'Youtube_12122018']


if __name__ == '__main__':
    run_tcp_wehe_experiment('Amazon_01042019', '100Mbps', 'TcpNewReno', 'neutral_vb2b')