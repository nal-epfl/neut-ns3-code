# This file is for experiments done after Sep 2021 related to tomography inference based on loss
# To run this script please go back to the original GitHub directory which is compatible with version <= ns3.35
# Link: https://github.com/zeinabshmeis/neut-ns3-code.git
from project_run_env.RunConfig import *
from multiprocessing import Process

class ExperimentParameters:
    def __init__(self, link_rate, duration, is_tcp=0, tcp_protocol='TcpCubic', seed=3,
                 app_type=0, app_name='Poisson_Probes', pkt_size=1228, p_lambda=0.001, replay_trace='empty',
                 background_dir='empty', exp_batch='', noncommon_links_delays='empty', noncommon_links_rates='empty'):
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
        self.background_dir = background_dir
        self.exp_batch = exp_batch
        self.noncommon_links_delays = noncommon_links_delays
        self.noncommon_links_rates = noncommon_links_rates


def run_probing_experiment_with_params(params):
    run_probing_experiment(link_rate=params.m_link_rate, duration=params.m_duration,
                           is_tcp=params.m_is_tcp, tcp_protocol=params.tcp_protocol, seed=params.seed,
                           app_type=params.app_type, app_name=params.app_name,
                           pkt_size=params.pkt_size, p_lambda=params.p_lambda, replay_trace=params.replay_trace,
                           background_dir=params.m_background_dir, exp_batch=params.m_exp_batch,
                           noncommon_links_delays=params.noncommon_links_delays,
                           noncommon_links_rates=params.noncommon_links_rates)


def run_probing_experiment(link_rate, duration, is_tcp=0, tcp_protocol='TcpCubic', seed=3,
                           app_type=0, app_name='Poisson_Probes', pkt_size=1228, p_lambda=0.001, replay_trace='empty',
                           background_dir='empty', exp_batch='',
                           noncommon_links_delays='empty', noncommon_links_rates='empty'):
    # run the ns3 simulation
    result_folder_name = '9_2021/tomo_with_loss/{}/link_{}/{}/{}/seed_{}/{}'.format(
        app_name, link_rate, background_dir, exp_batch, seed, tcp_protocol if is_tcp else 'udp'
    )

    os.system('mkdir -p {}/scratch/wehe_p_tomography/results/{}'.format(get_ns3_path(), result_folder_name))

    os.system(
        # '{}/waf --run "wehe_p_tomography" > log.out 2>&1 --command-template="%s'.format(get_ns3_path()) +
        '{}/waf --run "wehe_p_tomography" --command-template="%s'.format(get_ns3_path()) +
        ' --RngSeed={}'.format(seed) +
        ' --RngRun=1' +
        ' --linkRate={}'.format(link_rate) +
        ' --duration={}'.format(duration) +
        ' --resultsFolderName={}'.format(result_folder_name) +
        ' --appProtocol={}'.format(is_tcp) +
        ' --TCPProtocol=ns3::{}'.format(tcp_protocol) +
        ' --appType={}'.format(app_type) +
        ' --pktSize={}'.format(pkt_size) +
        ' --lambda={}'.format(p_lambda) +
        ' --replayTrace={}'.format(replay_trace) +
        ' --backgroundDir={}'.format(background_dir) +
        ' --nonCommonlinksDelays={}'.format(noncommon_links_delays) +
        ' --nonCommonlinksDataRates={}'.format(noncommon_links_rates) +
        '"'
    )


def rebuild_project():
    os.system('{}/waf build'.format(get_ns3_path()))


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
    rebuild_project()

    # constant rate probing + poisson replay
    run_parallel_experiments([
        ExperimentParameters(link_rate='200Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
                             app_type=4, app_name='Infinite_Tcp', pkt_size=1228,
                             background_dir='back_traffic_long', exp_batch=''),
        ExperimentParameters(link_rate='250Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
                             app_type=4, app_name='Infinite_Tcp', pkt_size=1228,
                             background_dir='back_traffic_long', exp_batch=''),
        ExperimentParameters(link_rate='280Mbps', duration=590, is_tcp=1, tcp_protocol='TcpCubic',
                             app_type=4, app_name='Infinite_Tcp', pkt_size=1228,
                             background_dir='back_traffic_long', exp_batch=''),
    ])
