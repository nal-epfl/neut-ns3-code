# This file is for experiments done after Sep 2021 related to tomography inference based on loss
from project_run_env.RunConfig import *
from data.data_preparation import *
from multiprocessing import Process

wehe_apps_names = ['Amazon_01042019', 'Amazon_12122018', 'AppleMusic_04112019', 'FacebookVideo_04112019',
                   'Hulu_04112019', 'NBCSports_01042019', 'NBCSports_12122018', 'Netflix_12122018',
                   'Skype_12122018', 'Spotify_01042019', 'Spotify_12122018', 'Twitch_04112019', 'Vimeo_12122018',
                   'WhatsApp_04112019', 'Youtube_12122018']

# This is to specify how I am performing the test
TEST_TYPE = 'neut_with_loss'


class ExperimentParameters:
    def __init__(self, link_rate, duration, is_tcp=0, tcp_protocol='TcpCubic', seed=3, app_name='Poisson_Probes',
                 app_type=0, pkt_size=1228, p_lambda=0.001, replay_trace='empty', app_data_rate='20Mbps',
                 background_dir='empty', exp_batch='', noncommon_links_delays='empty', noncommon_links_rates='empty',
                 is_neutral=0, policing_rate=0.0, burst_length=0.0, throttle_udp=0):
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
        self.app_data_rate = app_data_rate
        self.background_dir = background_dir
        self.exp_batch = exp_batch
        self.noncommon_links_delays = noncommon_links_delays
        self.noncommon_links_rates = noncommon_links_rates
        self.is_neutral = is_neutral
        self.policing_rate = policing_rate
        self.burst_length = burst_length
        self.throttle_udp = throttle_udp


def run_probing_experiment_with_params(params):
    run_probing_experiment(link_rate=params.link_rate, duration=params.duration,
                           is_tcp=params.is_tcp, tcp_protocol=params.tcp_protocol, seed=params.seed,
                           app_name=params.app_name, app_type=params.app_type, pkt_size=params.pkt_size,
                           p_lambda=params.p_lambda, replay_trace=params.replay_trace,
                           app_data_rate=params.app_data_rate,
                           background_dir=params.background_dir, exp_batch=params.exp_batch,
                           noncommon_links_delays=params.noncommon_links_delays,
                           noncommon_links_rates=params.noncommon_links_rates,
                           is_neutral=params.is_neutral, policing_rate=params.policing_rate,
                           burst_length=params.burst_length, throttle_udp=params.throttle_udp)


def run_probing_experiment(link_rate, duration, is_tcp, tcp_protocol='TcpCubic', seed=3, app_name='Poisson_Probes',
                           app_type=0, pkt_size=1228, p_lambda=0.001, replay_trace='empty', app_data_rate='20Mbps',
                           background_dir='empty', exp_batch='',
                           noncommon_links_delays='empty', noncommon_links_rates='empty',
                           is_neutral=0, policing_rate=0.0, burst_length=0.0, throttle_udp=0):
    # run the ns3 simulation
    result_folder_name = '2_2022/{}/{}/link_{}/{}/{}/seed_{}/{}'.format(
        TEST_TYPE, app_name, link_rate, background_dir, exp_batch, seed, tcp_protocol if is_tcp else 'udp'
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
        ' --appDataRate={}'.format(app_data_rate) +
        ' --backgroundDir={}'.format(background_dir) +
        ' --neutral={}'.format(is_neutral) +
        ' --policingRate={}'.format(policing_rate) +
        ' --policingBurstLength={}'.format(burst_length) +
        ' --throttleUdp={}'.format(throttle_udp) +
        ' --nonCommonlinksDelays={}'.format(noncommon_links_delays) +
        ' --nonCommonlinksDataRates={}'.format(noncommon_links_rates) +
        '"'
    )


def run_weheCS_experiment_with_params(params):
    run_weheCS_experiment(link_rate=params.link_rate, is_tcp=params.is_tcp, tcp_protocol=params.tcp_protocol,
                          seed=params.seed, app_name=params.app_name,
                          background_dir=params.background_dir, exp_batch=params.exp_batch,
                          noncommon_links_delays=params.noncommon_links_delays,
                          noncommon_links_rates=params.noncommon_links_rates,
                          is_neutral=params.is_neutral, policing_rate=params.policing_rate,
                          burst_length=params.burst_length)


def run_weheCS_experiment(link_rate, is_tcp, tcp_protocol='TcpCubic', seed=3, app_name='Netflix_12122018',
                          background_dir='empty', exp_batch='',
                          noncommon_links_delays='empty', noncommon_links_rates='empty',
                          is_neutral=0, policing_rate=0.0, burst_length=0.0):

    # prepare the wehe trace
    project_data_path = '{}/scratch/wehe_p_tomography/data'.format(get_ns3_path())
    # generate_weheCS_trace(project_data_path, app_name, 'weheCS_trace')
    duration = get_weheCS_duration(project_data_path + '/weheCS_trace')

    # run the ns3 simulation
    result_folder_name = '2_2022/{}/Wehe_{}/link_{}/{}/{}/seed_{}/{}'.format(
        TEST_TYPE, app_name, link_rate, background_dir, exp_batch, seed, tcp_protocol if is_tcp else 'udp'
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
        ' --appType={}'.format(5) +
        ' --backgroundDir={}'.format(background_dir) +
        ' --neutral={}'.format(is_neutral) +
        ' --policingRate={}'.format(policing_rate) +
        ' --policingBurstLength={}'.format(burst_length) +
        ' --nonCommonlinksDelays={}'.format(noncommon_links_delays) +
        ' --nonCommonlinksDataRates={}'.format(noncommon_links_rates) +
        '"'
    )


def run_tcp_test(link_rate, duration, is_tcp, tcp_protocol='TcpCubic', seed=3, app_name='TCP_APP', pkt_size=1228,
                 exp_batch='', is_neutral=0, policing_rate=0.0, burst_length=0.0):
    # run the ns3 simulation
    result_folder_name = '2_2022/{}/{}/link_{}/{}/seed_{}/{}'.format(
        TEST_TYPE, app_name, link_rate, exp_batch, seed, tcp_protocol if is_tcp else 'udp'
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
        ' --TCPProtocol=ns3::{}'.format(tcp_protocol) +
        ' --pktSize={}'.format(pkt_size) +
        ' --neutral={}'.format(is_neutral) +
        ' --policingRate={}'.format(policing_rate) +
        ' --policingBurstLength={}'.format(burst_length) +
        '"'
    )


def rebuild_project():
    os.system('{}/waf build'.format(get_ns3_path()))


def run_parallel_experiments(func, experiments):
    processes = [Process(target=func, kwargs={'params': params}) for params in
                 experiments]

    # kick them off
    for process in processes:
        process.start()

    # now wait for them to finish
    for process in processes:
        process.join()


if __name__ == '__main__':
    rebuild_project()

    ############################################################### Localization Tests ##############################################################
    TEST_TYPE = 'neut_plus_localization'
    seed, duration = 3, 10
    background_dir = 'chicago_2010_back_traffic_10min_control_cbp_2links'

    exps = [
        ('no_congestion_at_all', '10Gbps', '1Gbps,1Gbps,10Gbps', 'empty'),

        ('no_congestion_at_all_p2_d2ms', '10Gbps', '1Gbps,1Gbps,10Gbps', '5ms,2ms,5ms'),
        ('no_congestion_at_all_p2_d15ms', '10Gbps', '1Gbps,1Gbps,10Gbps', '5ms,15ms,5ms'),

        ('congestion_on_noncommon_links_p12_r90Mbps', '10Gbps', '90Mbps,90Mbps,10Gbps', 'empty'),
        ('congestion_on_noncommon_links_p12_r100Mbps', '10Gbps', '100Mbps,100Mbps,10Gbps', 'empty'),
        ('congestion_on_noncommon_links_p1_r90Mbps_p2_r100Mbps', '10Gbps', '90Mbps,100Mbps,10Gbps', 'empty'),

        ('congestion_on_common_link', '180Mbps', '1Gbps,1Gbps,10Gbps', 'empty'),
        ('congestion_on_common_link', '200Mbps', '1Gbps,1Gbps,10Gbps', 'empty'),
        ('congestion_on_common_link', '220Mbps', '1Gbps,1Gbps,10Gbps', 'empty'),

        ('congestion_on_common_link_p2_d2ms', '180Mbps', '1Gbps,1Gbps,10Gbps', '5ms,2ms,5ms'),
        ('congestion_on_common_link_p2_d15ms', '180Mbps', '1Gbps,1Gbps,10Gbps', '5ms,15ms,5ms'),
        ('congestion_on_all_links_p12_r95Mbps', '180Mbps', '95Mbps,95Mbps,10Gbps', 'empty'),
        ('congestion_on_all_links_p12_r100Mbps', '180Mbps', '100Mbps,100Mbps,100Mbps', 'empty'),
        ('congestion_on_all_links_p1_r100Mbps_p2_r95Mbps', '180Mbps', '100Mbps,95Mbps,10Gbps', 'empty'),
        ('congestion_on_all_links_p1_r95Mbps_p2_r100Mbps_d15ms', '180Mbps', '95Mbps,100Mbps,10Gbps', '5ms,15ms,5ms'),

        ('congestion_on_common_link_p2_d2ms', '200Mbps', '1Gbps,1Gbps,10Gbps', '5ms,2ms,5ms'),
        ('congestion_on_common_link_p2_d15ms', '200Mbps', '1Gbps,1Gbps,10Gbps', '5ms,15ms,5ms'),
        ('congestion_on_all_links_p12_r95Mbps', '200Mbps', '95Mbps,95Mbps,10Gbps', 'empty'),
        ('congestion_on_all_links_p12_r100Mbps', '200Mbps', '100Mbps,100Mbps,100Mbps', 'empty'),
        ('congestion_on_all_links_p1_r100Mbps_p2_r95Mbps', '200Mbps', '100Mbps,95Mbps,10Gbps', 'empty'),
        ('congestion_on_all_links_p1_r95Mbps_p2_r100Mbps_d15ms', '200Mbps', '95Mbps,100Mbps,10Gbps', '5ms,15ms,5ms'),
    ]

    cases_per_exp = [
        [
            ('shared_common_policer_2Mbps_0.03s_30p', 1, 2),
            ('shared_common_policer_5Mbps_0.03s_30p', 1, 5),
            # ('shared_common_policer_10Mbps_0.03s_30p', 1, 10),
            # ('shared_common_policer_15Mbps_0.03s_30p', 1, 15),
            # ('shared_common_policer_20Mbps_0.03s_30p', 1, 20),
            # ('shared_common_policer_30Mbps_0.03s_30p', 1, 30),
            # ('shared_common_policer_28Mbps_0.03s_30p', 1, 28),
            # ('shared_common_policer_25Mbps_0.03s_30p', 1, 25),
        ],
        # [
        #     ('shared_noncommon_policers_20Mbps_0.03s_30p', 3, 20),
        #     ('shared_noncommon_policers_18Mbps_0.03s_30p', 3, 18),
        #     ('shared_noncommon_policers_15Mbps_0.03s_30p', 3, 15),
        #     ('shared_noncommon_policers_14Mbps_0.03s_30p', 3, 14),
        #     ('shared_noncommon_policers_13Mbps_0.03s_30p', 3, 13),
        # ],
    ]

    # cases_per_exp = [
    #     [
    #         ('shared_common_policer_40Mbps_0.03s_30p', 1, 40),
    #         ('shared_common_policer_35Mbps_0.03s_30p', 1, 35),
    #         ('shared_common_policer_30Mbps_0.03s_30p', 1, 30),
    #         ('shared_common_policer_28Mbps_0.03s_30p', 1, 28),
    #         ('shared_common_policer_25Mbps_0.03s_30p', 1, 25),
    #     ],
    #     [
    #         ('shared_noncommon_policers_20Mbps_0.03s_30p', 3, 20),
    #         ('shared_noncommon_policers_18Mbps_0.03s_30p', 3, 18),
    #         ('shared_noncommon_policers_15Mbps_0.03s_30p', 3, 15),
    #         ('shared_noncommon_policers_14Mbps_0.03s_30p', 3, 14),
    #         ('shared_noncommon_policers_13Mbps_0.03s_30p', 3, 13),
    #     ],
    #     # [
    #     #     ('shared_noncommonp2_policer_20Mbps_0.03s_30p', 5, 20),
    #     #     ('shared_noncommonp2_policer_18Mbps_0.03s_30p', 5, 18),
    #     #     ('shared_noncommonp2_policer_15Mbps_0.03s_30p', 5, 15),
    #     #     ('shared_noncommonp2_policer_14Mbps_0.03s_30p', 5, 14),
    #     #     ('shared_noncommonp2_policer_13Mbps_0.03s_30p', 5, 13),
    #     # ],
    # ]
    # for a_seed in [7]:
    #     for exp_batch, link_rate, noncommon_link_rates, noncommon_link_delays in exps:
    #         print('---------------- Running: {} - {} / seed: {} ----------------'.format(link_rate, exp_batch, a_seed))
    #         for mini_cases_per_exp in cases_per_exp:
    #             rebuild_project()
    #             run_parallel_experiments(run_probing_experiment_with_params, [
    #                 ExperimentParameters(
    #                     link_rate=link_rate, duration=duration, is_tcp=1, tcp_protocol='TcpCubic', seed=a_seed,
    #                     app_type=4, app_name='Infinite_Paced_Tcp', pkt_size=1228, background_dir=background_dir,
    #                     app_data_rate='20Mbps', exp_batch='{}/{}'.format(exp_batch, case),
    #                     noncommon_links_delays=noncommon_link_delays, noncommon_links_rates=noncommon_link_rates,
    #                     is_neutral=is_neutral, policing_rate=policing_rate, burst_length=0.03
    #                 ) for case, is_neutral, policing_rate in mini_cases_per_exp
    #             ])

    wehe_app, is_tcp = 'Long_Skype_12122018', 0
    generate_weheCS_trace('{}/scratch/wehe_p_tomography/data'.format(get_ns3_path()), wehe_app, 'weheCS_trace')
    for a_seed in [3]:
        for exp_batch, link_rate, noncommon_link_rates, noncommon_link_delays in exps:
            print('---------------- Running: {} - {} / seed: {} ----------------'.format(link_rate, exp_batch, a_seed))
            for mini_cases_per_exp in cases_per_exp:
                rebuild_project()
                run_parallel_experiments(run_weheCS_experiment_with_params, [
                    ExperimentParameters(
                        link_rate=link_rate, duration=duration, is_tcp=is_tcp, tcp_protocol='TcpCubic', seed=a_seed,
                        app_type=5, app_name=wehe_app, background_dir=background_dir,
                        exp_batch='{}/{}'.format(exp_batch, case),
                        noncommon_links_delays=noncommon_link_delays, noncommon_links_rates=noncommon_link_rates,
                        is_neutral=is_neutral, policing_rate=policing_rate, burst_length=0.03
                    ) for case, is_neutral, policing_rate in mini_cases_per_exp
                ])