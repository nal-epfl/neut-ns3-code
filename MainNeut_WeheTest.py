# This file is for experiments done after Sep 2021 related to tomography inference based on loss
from project_run_env.RunConfig import *
from data.data_preparation import *
from multiprocessing import Process
from multiprocessing import cpu_count
import time

primes = np.array([
    3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73,
    79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
    179, 181, 191, 193, 197, 199, 211, 223, 227, 233, 239
])

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
                 is_neutral=0, policing_rate=0.0, burst_length=0.0, throttle_udp=0, throttled_back_pct=0.03):
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
        self.throttled_back_pct = throttled_back_pct


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
                           burst_length=params.burst_length, throttle_udp=params.throttle_udp,
                           throttled_back_pct=params.throttled_back_pct)


def run_probing_experiment(link_rate, duration, is_tcp, tcp_protocol='TcpCubic', seed=3, app_name='Poisson_Probes',
                           app_type=0, pkt_size=1228, p_lambda=0.001, replay_trace='empty', app_data_rate='20Mbps',
                           background_dir='empty', exp_batch='',
                           noncommon_links_delays='empty', noncommon_links_rates='empty',
                           is_neutral=0, policing_rate=0.0, burst_length=0.0, throttle_udp=0, throttled_back_pct=0.03):
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
        ' --controlTestDuration={}'.format(duration) +
        ' --suspectedTestDuration={}'.format(duration) +
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
        ' --backThrottledPct={}'.format(throttled_back_pct) +
        ' --nonCommonlinksDelays={}'.format(noncommon_links_delays) +
        ' --nonCommonlinksDataRates={}'.format(noncommon_links_rates) +
        '"'
    )


def run_weheCS_experiment_with_params(params):
    run_weheCS_experiment(link_rate=params.link_rate, original_traffic_duration=params.duration,
                          is_tcp=params.is_tcp, tcp_protocol=params.tcp_protocol,
                          seed=params.seed, app_name=params.app_name,
                          background_dir=params.background_dir, exp_batch=params.exp_batch,
                          noncommon_links_delays=params.noncommon_links_delays,
                          noncommon_links_rates=params.noncommon_links_rates,
                          is_neutral=params.is_neutral, policing_rate=params.policing_rate,
                          burst_length=params.burst_length,
                          throttled_back_pct=params.throttled_back_pct)


def run_weheCS_experiment(link_rate, original_traffic_duration=0, is_tcp=1, tcp_protocol='TcpCubic', seed=3, app_name='Netflix_12122018',
                          background_dir='empty', exp_batch='',
                          noncommon_links_delays='empty', noncommon_links_rates='empty',
                          is_neutral=0, policing_rate=0.0, burst_length=0.0, throttled_back_pct=0.03):

    # prepare the wehe trace
    wehe_trace = 'weheCS_{}_trace'.format(app_name)
    project_data_path = '{}/scratch/wehe_p_tomography/data'.format(get_ns3_path())
    randomized_test_duration = get_weheCS_duration('{}/{}'.format(project_data_path, wehe_trace))
    if original_traffic_duration == 0:
        original_traffic_duration = randomized_test_duration

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
        ' --controlTestDuration={}'.format(randomized_test_duration) +
        ' --suspectedTestDuration={}'.format(original_traffic_duration) +
        ' --resultsFolderName={}'.format(result_folder_name) +
        ' --appProtocol={}'.format(is_tcp) +
        ' --TCPProtocol=ns3::{}'.format(tcp_protocol) +
        ' --appType={}'.format(5) +
        ' --replayTrace={}'.format(wehe_trace) +
        ' --backgroundDir={}'.format(background_dir) +
        ' --neutral={}'.format(is_neutral) +
        ' --policingRate={}'.format(policing_rate) +
        ' --policingBurstLength={}'.format(burst_length) +
        ' --backThrottledPct={}'.format(throttled_back_pct) +
        ' --nonCommonlinksDelays={}'.format(noncommon_links_delays) +
        ' --nonCommonlinksDataRates={}'.format(noncommon_links_rates) +
        '"'
    )


def run_tcp_test(link_rate, duration, is_tcp, tcp_protocol='TcpCubic', seed=3, app_name='TCP_APP', pkt_size=1228,
                 exp_batch='', is_neutral=0, policing_rate=0.0, burst_length=0.0, throttled_back_pct=0.03):
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
        ' --backThrottledPct={}'.format(throttled_back_pct) +
        '"'
    )


def rebuild_project():
    os.system('{}/waf build'.format(get_ns3_path()))


def run_parallel_experiments(func, experiments):
    processes = [Process(target=func, kwargs={'params': params}) for params in experiments]

    # kick them off
    for process_idx, process in enumerate(processes):
        time.sleep(5)
        os.system("taskset -p -c %d %d" % ((process_idx+1) % cpu_count(), os.getpid()))
        process.start()

    # now wait for them to finish
    for process in processes:
        process.join()


def run_parallel_experiments_safe(func, experiments):
    running_processes = []
    nb_threads = cpu_count() / 2 - 5

    for idx, params in enumerate(experiments):
        time.sleep(5)
        running_processes.append(Process(target=func, kwargs={'params': params}))
        os.system("taskset -p -c %d %d" % (nb_threads - idx, os.getpid()))
        running_processes[-1].start()

        if len(running_processes) >= nb_threads:
            [p.join() for p in running_processes]
            running_processes = []


if __name__ == '__main__':
    rebuild_project()

    ############################################################### Localization Tests ##############################################################
    netflix, hulu, youtube, twitch, facebook, vimeo = 'Netflix_12122018', 'Hulu_04112019', 'Youtube_12122018', 'Twitch_04112019', 'FacebookVideo_04112019', 'Vimeo_12122018'
    TEST_TYPE = 'Wehe_Test_Cases'
    background_dir = 'chicago_2010_back_traffic_10min_control_cbp_2links'

    link_rate, noncommon_link_rates = '10Gbps', '1Gbps,1Gbps,10Gbps'
    is_tcp, a_seed = 1, 13

    #for each user I need the following information
    wehe_tests = [
        ('feqHGJ240F_9', hulu, 14, 45, [('shared_common_policer', 1, 42, 0.03), ('shared_noncommon_policers', 3, 21, 0.03)]),
        ('198iWiLIfr_128', netflix, 19, 40, [('shared_common_policer', 1, 40, 0.03), ('shared_noncommon_policers', 3, 20, 0.03)]),
        ('3xyk5naej3_1', vimeo, 27, 45, [('shared_common_policer', 1, 40, 0.03), ('shared_noncommon_policers', 3, 20, 0.03)]),
        ('89twsgj1pb_263', youtube, 31, 45, [('shared_common_policer', 1, 27, 0.03), ('shared_noncommon_policers', 3, 13.5, 0.03)]),
        ('9lwmntt0pt_8', youtube, 22, 110, [('shared_common_policer', 1, 28, 0.03), ('shared_noncommon_policers', 3, 14, 0.03)]),
        ('2b98k2i1qy_1', youtube, 45, 109, [('shared_common_policer', 1, 25, 0.03), ('shared_noncommon_policers', 3, 12.5, 0.03)]),

        ('smzkg3b9c3_12', hulu, 40, 52, [('shared_common_policer', 1, 30, 0.03), ('shared_noncommon_policers', 3, 15, 0.03)]),
        ('y3emavi0lu_22', twitch, 45, 60, [('shared_common_policer', 1, 25, 0.03), ('shared_noncommon_policers', 12.5, 15, 0.03)]),
        ('v8azm5f2qe_30', twitch, 17, 59, [('shared_common_policer', 1, 40, 0.03), ('shared_noncommon_policers', 12.5, 20, 0.03)]),
        ('v8azm5f2qe_21', vimeo, 23, 41, [('shared_common_policer', 1, 40, 0.03), ('shared_noncommon_policers', 12.5, 20, 0.03)]),
        ('v8azm5f2qe_31', youtube, 18, 61, [('shared_common_policer', 1, 45, 0.03), ('shared_noncommon_policers', 12.5, 22.5, 0.03)]),
        ('y3emavi0lu_17', youtube, 14, 82, [('shared_common_policer', 1, 45, 0.03), ('shared_noncommon_policers', 12.5, 22.5, 0.03)]),


    ]

    for a_seed in [3, 7]:
        exp_params = []
        for user, wehe_app, rtt, duration, policers_configs in wehe_tests:
            dp = (rtt-10)/2
            for p_type, is_neutral, policing_rate, burst_length in policers_configs:
                exp_params.append(ExperimentParameters(
                    link_rate=link_rate, duration=duration, is_tcp=is_tcp, tcp_protocol='TcpCubic', seed=a_seed,
                    app_type=5, app_name=wehe_app, background_dir=background_dir,
                    exp_batch='{}/{}_{}Mbps_{}s_30p'.format(user, p_type, policing_rate, burst_length),
                    noncommon_links_delays='{}ms,{}ms,5ms'.format(dp, dp), noncommon_links_rates=noncommon_link_rates,
                    is_neutral=is_neutral, policing_rate=policing_rate, burst_length=burst_length
                ))

        run_parallel_experiments_safe(run_weheCS_experiment_with_params, exp_params)