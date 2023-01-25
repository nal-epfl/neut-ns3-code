# This file is for experiments done after May 2022 related to neutrality-violations localization algorithm
# It is only compatible with the files in the localization_experiments_scenarios and ns3 versions >= ns3.36.1

from data.wehe_traces_processing import *

import os
import time
import numpy as np

from enum import Enum
from multiprocessing import Process, cpu_count

__ns3_path = os.popen('locate "ns-3.36.1" | grep /ns-3.36.1$').read().splitlines()[0]


def get_ns3_path(): return __ns3_path
def get_project_path(): return '{}/scratch/wehe_p_tomography'.format(get_ns3_path())


PRIMES = np.array([
    3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109,
    113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 233, 239
])


class WeheApp(Enum):
    MicrosoftTeam = 'MicrosoftTeam_04282020'
    Spotify = 'Spotify_01042019'
    Deezer = 'Deezer_09012019'
    Zoom = 'Zoom_04282020'
    Netflix = 'Netflix_12122018'
    AppleMusic = 'AppleMusic_04112019'
    FacebookVideo = 'FacebookVideo_04112019'
    Amazon = 'Amazon_11252020'
    Mycanal = 'Mycanal_10012019'
    Dailymotion = 'Dailymotion_09012019'
    Hulu = 'Hulu_04112019'
    Youtube = 'Youtube_12122018'
    Webex = 'Webex_04282020'
    GoogleMeet = 'GoogleMeet_04282020'
    Vimeo = 'Vimeo_12122018'
    Sfrplay = 'Sfrplay_08012019'
    TwitterVideo = 'TwitterVideo_04192021'
    Salto = 'Salto_11252020'
    Skype = 'Skype_12122018'
    Molotovtv = 'Molotovtv_08012019'
    OCS = 'OCS_08012019'
    WhatsApp = 'WhatsApp_04112019'
    Twitch = 'Twitch_04112019'
    DisneyPlus = 'DisneyPlus_05082020'
    NbcSports = 'NBCSports_01042019'


class UDPWeheApp(Enum):
    MicrosoftTeam = 'MicrosoftTeam_04282020'
    Zoom = 'Zoom_04282020'
    Webex = 'Webex_04282020'
    GoogleMeet = 'GoogleMeet_04282020'
    Skype = 'Skype_12122018'
    WhatsApp = 'WhatsApp_04112019'
    # the single path version
    SinglePathMicrosoftTeam = 'SinglePathMicrosoftTeam_04282020'
    SinglePathZoom = 'SinglePathZoom_04282020'
    SinglePathWebex = 'SinglePathWebex_04282020'
    SinglePathGoogleMeet = 'SinglePathGoogleMeet_04282020'
    SinglePathSkype = 'SinglePathSkype_12122018'
    SinglePathWhatsApp = 'SinglePathWhatsApp_04112019'
    # the probe version
    ProbeMicrosoftTeam = 'ProbeMicrosoftTeam_04282020'
    ProbeZoom = 'ProbeZoom_04282020'
    ProbeWebex = 'ProbeWebex_04282020'
    Probe2Webex = 'ProbeL0d011Webex_04282020'
    IncProbeWebex = 'IncProbeWebex_04282020'
    ProbeGoogleMeet = 'ProbeGoogleMeet_04282020'
    ProbeSkype = 'ProbeSkype_12122018'
    Probe2Skype = 'ProbeL0d005Skype_12122018'
    IncProbeSkype = 'IncProbeSkype_12122018'
    ProbeWhatsApp = 'ProbeWhatsApp_04112019'


def get_wehe_app(app_name):
    for app in WeheApp:
        if app_name in app.value: return app


class TransportProtocol(Enum):
    UDP = 0
    TCP = 1


class MeasurementAppType(Enum):
    CONSTANT_PROBES = 1
    POISSON = 2
    TRACE_REPLAY = 3
    INFINITE_TCP = 4
    WEHE_CS = 5


class PolicerLocation(Enum):
    COMMON_LINK = 'c'
    NONCOMMON_LINK1 = 'nc1'
    NONCOMMON_LINK2 = 'nc2'
    BOTH_NONCOMMON_LINKS = 'nc'


class PolicerType(Enum):
    SHARED = 0
    PER_FLOW = 1


class NetworkSetup:
    def __init__(self, common_link_rate, noncommon_links_delays='empty', noncommon_links_rates='empty'):
        self.common_link_rate = common_link_rate
        self.noncommon_links_delays = noncommon_links_delays
        self.noncommon_links_rates = noncommon_links_rates


class MeasurementAppSetup:
    def __init__(self, app_type, app_name, control_test_duration, suspected_test_duration,
                 transport_protocol=TransportProtocol.UDP, tcp_protocol='TcpCubic',
                 pkt_size=1228, p_lambda=0.001, replay_trace='empty', app_data_rate='20Mbps'):
        self.app_type = app_type
        self.app_name = app_name
        self.control_test_duration = control_test_duration
        self.suspected_test_duration = suspected_test_duration
        self.transport_protocol = transport_protocol
        self.tcp_protocol = tcp_protocol
        self.pkt_size = pkt_size
        self.p_lambda = p_lambda
        self.replay_trace = replay_trace
        self.app_data_rate = app_data_rate


class BackgroundTrafficSetup:
    def __init__(self, background_dir, pct_of_paced_tcp=0.8):
        self.background_dir = background_dir
        self.pct_of_paced_tcp = pct_of_paced_tcp


class NeutralitySetup:
    def __init__(self, is_neutral, policing_rate=0.0, burst_length=0.0, queue_size=15000,
                 policer_location=PolicerLocation.COMMON_LINK, policer_type=PolicerType.SHARED,
                 pct_of_throttled_background=0.03, overflow_events_trace='empty'):
        self.is_neutral = is_neutral
        self.policing_rate = policing_rate
        self.burst_length = burst_length
        self.queue_size = queue_size
        self.policer_location = policer_location
        self.policer_type = policer_type
        self.pct_of_throttled_background = pct_of_throttled_background
        self.overflow_events_trace = overflow_events_trace


class ExperimentParameters:
    def __init__(self, exp_type, network_setup, measurement_app_setup, background_setup, neutrality_setup, exp_batch, seed=3):
        self.exp_type = exp_type
        self.network_setup = network_setup
        self.measurement_app_setup = measurement_app_setup
        self.background_setup = background_setup
        self.neutrality_setup = neutrality_setup
        self.exp_batch = exp_batch
        self.seed = seed


def run_experiment_with_params(params):
    run_experiment(
        exp_type=params.exp_type, network_setup=params.network_setup, app_setup=params.measurement_app_setup,
        background_setup=params.background_setup, neutrality_setup=params.neutrality_setup,
        exp_batch=params.exp_batch, seed=params.seed
    )


def run_experiment(exp_type, network_setup, app_setup, background_setup, neutrality_setup, exp_batch, seed=3):
    result_folder_name = '{}/{}/link_{}/{}/{}/seed_{}/{}'.format(
        exp_type, app_setup.app_name, network_setup.common_link_rate, background_setup.background_dir, exp_batch, seed,
        app_setup.tcp_protocol if app_setup.transport_protocol == TransportProtocol.TCP else 'udp'
    )
    os.system('mkdir -p {}/scratch/wehe_p_tomography/results/{}'.format(get_ns3_path(), result_folder_name))
    os.system(
        '{}/ns3 run \'wehe_p_tomography '.format(get_ns3_path()) +
        ' --RngSeed={}'.format(seed) +
        ' --RngRun=1' +
        ' --commonLinkRate={}'.format(network_setup.common_link_rate) +
        ' --nonCommonLinksDelays={}'.format(network_setup.noncommon_links_delays) +
        ' --nonCommonLinksDataRates={}'.format(network_setup.noncommon_links_rates) +
        ' --resultsFolderName={}'.format(result_folder_name) +
        ' --appType={}'.format(app_setup.app_type.value) +
        ' --controlTestDuration={}'.format(app_setup.control_test_duration) +
        ' --suspectedTestDuration={}'.format(app_setup.suspected_test_duration) +
        ' --measurementsTransportProtocol={}'.format(app_setup.transport_protocol.value) +
        ' --TCPProtocol=ns3::{}'.format(app_setup.tcp_protocol) +
        ' --appDataRate={}'.format(app_setup.app_data_rate) +
        ' --pktSize={}'.format(app_setup.pkt_size) +
        ' --lambda={}'.format(app_setup.p_lambda) +
        ' --replayTrace={}'.format(app_setup.replay_trace) +
        ' --backgroundDir={}'.format(background_setup.background_dir) +
        ' --pctPacedTcpBack={}'.format(background_setup.pct_of_paced_tcp) +
        ' --isNeutral={}'.format(neutrality_setup.is_neutral) +
        ' --policingRate={}'.format(neutrality_setup.policing_rate) +
        ' --policingBurstLength={}'.format(neutrality_setup.burst_length) +
        ' --policerQueueSize={}'.format(neutrality_setup.queue_size) +
        ' --policerLocation={}'.format(neutrality_setup.policer_location.value) +
        ' --policerType={}'.format(neutrality_setup.policer_type.value) +
        ' --backThrottledPct={}'.format(neutrality_setup.pct_of_throttled_background) +
        ' --overflowEventsTrace={}'.format(neutrality_setup.overflow_events_trace) +
        '\' --no-build'
    )


def get_weheCS_app_setup(wehe_app, transport_protocol, tcp_protocol='TcpCubic', original_traffic_duration=0):
    wehe_trace = 'weheCS_{}_trace'.format(wehe_app)
    project_data_path = '{}/scratch/wehe_p_tomography/data'.format(get_ns3_path())
    randomized_test_duration = get_weheCS_duration('{}/{}'.format(project_data_path, wehe_trace))
    if original_traffic_duration == 0: original_traffic_duration = randomized_test_duration
    return MeasurementAppSetup(
        app_type=MeasurementAppType.WEHE_CS, app_name='Wehe_{}'.format(wehe_app),
        control_test_duration=randomized_test_duration, suspected_test_duration=original_traffic_duration,
        transport_protocol=transport_protocol, tcp_protocol=tcp_protocol, replay_trace=wehe_trace
    )


def rebuild_project():
    os.system('{}/ns3 build'.format(get_ns3_path()))


def run_parallel_experiments(func, experiments, nb_threads=1):
    if nb_threads >= (cpu_count()/2):
        raise Exception("ERROR!!! Number of threads more than number of physical cpus")

    processes_queue, last_idx = [], len(experiments)-1
    for params_idx, params in enumerate(experiments):
        processes_queue.append(Process(target=func, kwargs={'params': params}))
        if (len(processes_queue) >= nb_threads) or (params_idx == last_idx):
            for process_idx, process in enumerate(processes_queue):
                time.sleep(5)
                os.system("taskset -p -c %d %d" % (process_idx, os.getpid()))
                process.start()
            [p.join() for p in processes_queue]
            processes_queue = []
