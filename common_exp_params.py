from helper_methods import *

# policer configuration parameters:
rates = np.array([24, 29, 36, 48, 55])
rates_ratio = np.array([1.3, 1.5, 2, 2.5])
limits_as_ratios = np.array([0.25, 0.5, 1])


def get_burst(rate, burst_period):
    return max(int(rate * burst_period * 125000), 15000)


# Bandwidth
d_c_bandwidth, d_nc_bandwidth = '10Gbps', '1Gbps'
c_bandwidths = np.array(['150Mbps', '160Mbps', '180Mbps']) # np.array(['180Mbps', '190Mbps', '200Mbps', '210Mbps', '220Mbps'])
nc_bandwidths = np.array(['65Mbps', '70Mbps', '75Mbps']) # np.array(['90Mbps', '95Mbps', '100Mbps', '105Mbps', '110Mbps'])
c_bandwidths_ratios = [1.05, 1.15, 1.2]


# RTTs
rtts_ms = np.array([15, 25, 35, 60, 120])
d_rtt_ms = 35
dp_c_ms, dp_interm_ms = 3, 4


def ms_2_s(t):
    return t * 1e-3


def get_nc_dp(rtt_ms):
    return rtt_ms/2 - (dp_interm_ms + dp_c_ms)


# other experiment parameters
d_background_dir = 'chicago_2010_back_traffic_10min_control_cbp_2links'
d_background_setup = BackgroundTrafficSetup(d_background_dir)
empty_background_setup = BackgroundTrafficSetup('empty')
d_duration = 30


# Wehe application volumes
# for some reason the POC do not send the exact volume of background (it sends 25Mbps instead of 32Mbps)
# This leads to higher loss in simulation. For this we take more volume into account here.
v_extra = 5
app_volumes = {
    'Wehe_{}'.format(UDPWeheApp.GoogleMeet.value): 1,
    'Wehe_{}'.format(UDPWeheApp.ProbeGoogleMeet.value): 1,

    'Wehe_{}'.format(UDPWeheApp.Webex.value): 2,
    'Wehe_{}'.format(UDPWeheApp.Probe2Webex.value): 2,
    'Wehe_{}'.format(UDPWeheApp.IncProbeWebex.value): 2,

    'Wehe_{}'.format(UDPWeheApp.Zoom.value): 2.5,
    'Wehe_{}'.format(UDPWeheApp.ProbeZoom.value): 2.5,

    'Wehe_{}'.format(UDPWeheApp.WhatsApp.value): 4,
    'Wehe_{}'.format(UDPWeheApp.Probe2WhatsApp.value): 4,
    'Wehe_{}'.format(UDPWeheApp.IncProbeWhatsApp.value): 4,

    'Wehe_{}'.format(UDPWeheApp.MicrosoftTeam.value): 2,
    'Wehe_{}'.format(UDPWeheApp.ProbeMicrosoftTeam.value): 2,

    'Wehe_{}'.format(UDPWeheApp.Skype.value): 3,
    'Wehe_{}'.format(UDPWeheApp.Probe2Skype.value): 3,
    'Wehe_{}'.format(UDPWeheApp.IncProbeSkype.value): 3,

    'Wehe_{}'.format(WeheApp.Youtube.value): 22,
    'Wehe_{}'.format(WeheApp.DisneyPlus.value): 42,
    'Wehe_{}'.format(WeheApp.Netflix.value): 15,
    'Wehe_{}'.format(WeheApp.Amazon.value): 20,
    'Wehe_{}'.format(WeheApp.Twitch.value): 50,
    'Wehe_{}'.format(WeheApp.Hulu.value): 45,
    'Wehe_{}'.format(WeheApp.FacebookVideo.value): 18,
    'Wehe_{}'.format(WeheApp.NbcSports.value): 22,

    'Infinite_Paced_TCP': 25
}
back_volume_by_pct = {
    '0.25': 30,
    '0.3': 30,
    '0.5': 60,
    '0.75': 90,
    '1': 110
}


def get_traffic_volume(app_name, background_pct):
    return app_volumes[app_name] + back_volume_by_pct[background_pct]
