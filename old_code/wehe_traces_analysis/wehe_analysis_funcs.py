import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
from project_run_env.RunConfig import *


def read_wehe_trace_df(filename):
    header = ['frame_nb', 'protocol', 'timestamp', 'src_ip', 'src_port', 'dst_ip', 'dst_port', 'payload_size']
    return pd.read_csv(filename, names=header)


def change_timestamp_to_poisson(wehe_df):
    avg_inter_arr = wehe_df['timestamp'].diff().mean()
    nb_samples = wehe_df.shape[0]
    exp_inter_arr = np.random.exponential(scale=avg_inter_arr, size=nb_samples-1)

    poisson_wehe = wehe_df.copy()
    poisson_wehe.loc[1:, 'timestamp'] = exp_inter_arr
    poisson_wehe['timestamp'] = poisson_wehe['timestamp'].cumsum()
    return poisson_wehe


def generate_poisson_wehe_traces(app_name):
    data_path = get_project_path() + '/data'

    server_df = read_wehe_trace_df(data_path + '/' + app_name + '_packetMeta_server')
    poisson_server_df = change_timestamp_to_poisson(server_df)
    poisson_server_df.to_csv(data_path + '/' + app_name + '_poisson_packetMeta_server', header=False, index=False)

    client_df = read_wehe_trace_df(data_path + '/' + app_name + '_packetMeta_client')
    poisson_client_df = change_timestamp_to_poisson(client_df)
    poisson_client_df.to_csv(data_path + '/' + app_name + '_poisson_packetMeta_client', header=False, index=False)
