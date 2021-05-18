import pandas as pd
import math as math


__ns3trace_header = ['id', 'timestamp', 'pkt_size']
__wehetrace_header = ['frame_nb', 'protocol', 'timestamp', 'src_ip', 'src_port', 'dst_ip', 'dst_port', 'payload_size']


def wehe_to_ns3trace(wehe_trace_path, ns3_trace_path):
    wehe_df = pd.read_csv(wehe_trace_path, names=__wehetrace_header)
    wehe_df = wehe_df[wehe_df.payload_size != 0]
    ns3_df = pd.DataFrame(columns=__ns3trace_header)
    ns3_df[['id', 'timestamp', 'pkt_size']] = wehe_df[['frame_nb', 'timestamp', 'payload_size']]
    ns3_df.to_csv(ns3_trace_path, header=False, index=False)


def ns3result_to_ns3trace(ns3_result_path, ns3_trace_path, cut_interval=()):
    ns3_result_df = pd.read_csv(ns3_result_path).sort_values(by=['SentTime'], ascending=True).reset_index(drop=True)
    if cut_interval:
        ns3_result_df = ns3_result_df[ns3_result_df['SentTime'].between(cut_interval[0]*1e9, cut_interval[1]*1e9)].reset_index(drop=True)
        ns3_result_df['SentTime'] = ns3_result_df['SentTime'] - cut_interval[0]*1e9

    ns3_df = pd.DataFrame(columns=__ns3trace_header)
    ns3_df['id'] = ns3_result_df.index
    ns3_df['timestamp'] = ns3_result_df['SentTime'] / 1e9
    ns3_df['pkt_size'] = ns3_result_df['PayloadSize']

    ns3_df.to_csv(ns3_trace_path, header=False, index=False)


def generate_weheCS_trace(data_path, app_name, ns3_trace_name):
    wehe_client_df = pd.read_csv(data_path + '/' + app_name + '_packetMeta_client', names=__wehetrace_header)
    wehe_client_df = wehe_client_df[wehe_client_df.payload_size != 0]
    wehe_client_df['app_side'] = 'client'

    wehe_server_df = pd.read_csv(data_path + '/' + app_name + '_packetMeta_server', names=__wehetrace_header)
    wehe_server_df = wehe_server_df[wehe_server_df.payload_size != 0]
    wehe_server_df['app_side'] = 'server'

    wehe_df = pd.concat([wehe_server_df, wehe_client_df]).sort_values(by=['timestamp'], ascending=True)

    ns3_df = pd.DataFrame(columns=__ns3trace_header)
    ns3_df[['id', 'timestamp', 'pkt_size', 'app_side']] = wehe_df[['frame_nb', 'timestamp', 'payload_size', 'app_side']]
    ns3_df.to_csv(data_path + '/' + ns3_trace_name, header=False, index=False)


def get_ns3trace_duration(filename):
    return math.ceil(pd.read_csv(filename, names=__ns3trace_header).timestamp.iat[-1])


def get_weheCS_duration(filename):
    return math.ceil(pd.read_csv(filename, names=['id', 'timestamp', 'pkt_size', 'app_side']).timestamp.iat[-1])


if __name__ == '__main__':
    # wehe_to_ns3trace('./Skype_12122018_poisson_packetMeta_client', './wehe_trace_client')
    # wehe_to_ns3trace('./Skype_12122018_packetMeta_server', './wehe_trace_server')
    # ns3result_to_ns3trace('../results/24_9_2020_results/Skype_12122018_poisson_wehe_1pct_results_back_alone/bottleneck_packets_down.csv', './control_trace')
    # ns3result_to_ns3trace('../results/bursty_traffic/nal-epfl/path_packets_up.csv', './control_trace')

    # back_trace_path = ('~/Desktop/experiments/ns-allinone-3.30.1/ns-3.30.1/scratch/wehe_plus_tomography' +
    #                    '/results/5_11_2020/working_scenario_1/synthesized_burst_traffic/path_packets_up.csv')
    # ns3result_to_ns3trace(back_trace_path, './ppb_back_2', cut_interval=(100, 200)) # old value was (120, 195)

    generate_weheCS_trace('./', 'Amazon_01042019', 'weheCS_trace')