from helper_methods import *

# policer configuration parameters:
rates = np.array([24, 29, 36, 48, 55])
rates_ratio = np.array([1.3, 1.5, 2, 2.5])
limits_as_ratios = np.array([0.25, 0.5, 1, 2])


def get_burst(rate, burst_period):
    return max(int(rate * burst_period * 125000), 15000)


# Bandwidth
d_c_bandwidth, d_nc_bandwidth = '10Gbps', '1Gbps'
c_bandwidths = np.array(['180Mbps', '200Mbps', '220Mbps']) # np.array(['180Mbps', '190Mbps', '200Mbps', '210Mbps', '220Mbps'])
nc_bandwidths = np.array(['90Mbps', '100Mbps', '110Mbps']) # np.array(['90Mbps', '95Mbps', '100Mbps', '105Mbps', '110Mbps'])


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
app_volumes = {
    UDPWeheApp.Webex: 27, UDPWeheApp.Probe2Webex: 27, UDPWeheApp.IncProbeWebex: 27,
    UDPWeheApp.Skype: 28, UDPWeheApp.Probe2Skype: 28, UDPWeheApp.IncProbeSkype: 28,
    UDPWeheApp.WhatsApp: 29, UDPWeheApp.Probe2WhatsApp: 29, UDPWeheApp.IncProbeWhatsApp: 29
}
