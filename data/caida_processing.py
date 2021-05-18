import pandas as pd
import os

__ns3trace_header = ['id', 'timestamp', 'pkt_size']

CAIDA_trace_path = './caida_chicago_2010_6min_back_traffic.csv'
output_path = './chicago_2010_back_traffic_6min'

caida_df = pd.read_csv(CAIDA_trace_path)

# generate the udp file
udp_output_path = output_path + '/UDP'
if not os.path.exists(udp_output_path): os.makedirs(udp_output_path)

caida_df_udp = caida_df[caida_df.Protocol == 'UDP'].reset_index().reset_index()
caida_df_udp = caida_df_udp[caida_df_udp.Length > 30]

ns3_df = pd.DataFrame(columns=__ns3trace_header)
ns3_df[['id', 'timestamp', 'pkt_size']] = caida_df_udp[['level_0', 'Time', 'Length']]
ns3_df['pkt_size'] = ns3_df['pkt_size'] - 30
ns3_df.to_csv(udp_output_path + '/trace_0.csv', header=False, index=False)


#generate the tcp files
tcp_output_path = output_path + '/TCP'
if not os.path.exists(tcp_output_path): os.makedirs(tcp_output_path)

caida_df_tcp = caida_df[caida_df.Protocol == 'TCP']
caida_df_tcp = caida_df_tcp[caida_df_tcp.Length > 58]

# prune very small flows with less than 10 packets
min_pkts_per_flow = 500
tcp_perflow_count = caida_df_tcp.groupby(['Destination']).size()
tcp_pruned_dsts = tcp_perflow_count[tcp_perflow_count > min_pkts_per_flow].keys().to_numpy()
caida_df_tcp = caida_df_tcp[caida_df_tcp.Destination.isin(tcp_pruned_dsts)]


for i, dstIP in enumerate(caida_df_tcp['Destination'].unique()):
    temp_df = caida_df_tcp[caida_df_tcp.Destination == dstIP].reset_index().reset_index()

    ns3_df = pd.DataFrame(columns=__ns3trace_header)
    ns3_df[['id', 'timestamp', 'pkt_size']] = temp_df[['level_0', 'Time', 'Length']]
    ns3_df['pkt_size'] = ns3_df['pkt_size'] - 58
    ns3_df.to_csv(tcp_output_path + '/trace_' + str(i) + '.csv', header=False, index=False)


