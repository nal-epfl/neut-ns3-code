//
// Created by zeinab on 7/26/22.
//
//
// Network topology
//
//      s0 ---+-- r0 ___ r1 --+--- d0
//
// - All links are P2P
// - Measurement applications runs along d0->s0
// - No Background
//

#include <iostream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"

#include "ns3/traffic-control-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"

#include "ns3/file-helper.h"

#include "../monitors_module/PacketMonitor.h"
#include "../traffic_generator_module/background_replay/BackgroundReplay.h"
#include "../traffic_generator_module/packet_probes/ProbesSenderHelper.h"
#include "../traffic_generator_module/measurement_replay/MeasurementReplaySenderHelper.h"
#include "../traffic_generator_module/infinite_tcp/InfiniteTCPSenderHelper.h"
#include "../traffic_differentiation_module/CbQueueDisc.h"
#include "../traffic_generator_module/wehe_cs/WeheCS.h"

using namespace ns3;
using namespace std;
using namespace helper_methods;
using namespace std::chrono;
namespace fs = std::filesystem;

#define PCAP_FLAG 0

/*****************************************************************************/
[[maybe_unused]] int run_policer_configuration_testing(int argc, char **argv) {
    auto start = high_resolution_clock::now();

/* ################################################ READ AND PREPARE PARAMETERS (START) ################################################ */
    /*** Defining Inputs: Variables read from arguments ***/
    string commonLinkRate = "180Mbps";              // common link bandwidth
    string nonCommonLinksDelaysStr = "empty";       // non-common links propagation delays; empty to use default values
    string nonCommonLinksDataRatesStr = "empty";    //  non-common link bandwidth; empty to use default values
    string resultsFolderName;                       // the output folder to save files generated by the monitors
    int isTCP = 0;                                  // 0 to run measurements as UDP --- 1 to run measurements as TCP
    string tcpProtocol = "ns3::TcpCubic";           // congestion control algorithm of the TCP Measurement traffic
    uint32_t appType = 0;                           // the measurement application type
    string appDataRate = "20Mbps";                  // in case of infinite tcp application: the rate of generating data
    uint32_t pktSize = 256;                         // the probe packets size
    double lambda = 0.001;                          // in case of packet_probes/constant application: the inter-arrival time includes probs
    string replayTrace = "empty";                   // in case of measurement replay application: the trace to replay
    float testDuration = 120.;                      // duration to run the measurements that may be policed
    string backgroundDir = "empty";                 // directory for the background traces to use as cross traffic
    int isNeutral = 0;                              // 0 to run a neutral scenario --- 1 for policing
    string policerLocation = "c";                   // 'c' for common link --- 'nci' for noncommon link of path i --- 'nc' for all noncommon links
    int policerType = 0;                            // 0 for shared policer --- 1 for per-flow policer
    double policingRate = 4;                        // the rate of generating tokens in the token bucket
    double burstLength = 0.1;                       // the burst length parameter of the token bucket
    double throttlingPctOfBack = 0.3;               // percentage of background to throttle with the measurement traffic
    string overflowEventsTrace = "";                // file that contains times in which policer experienced overflow events

    CommandLine cmd;
    cmd.AddValue("commonLinkRate", "common link bandwidth", commonLinkRate);
    cmd.AddValue("nonCommonLinksDelays", "non-common links propagation delays; empty to use default values", nonCommonLinksDelaysStr);
    cmd.AddValue("nonCommonLinksDataRates", "non-common link bandwidth; empty to use default values", nonCommonLinksDataRatesStr);
    cmd.AddValue("resultsFolderName", "the output folder to save files generated by the monitors", resultsFolderName);
    cmd.AddValue("appType", "the measurement application type", appType);
    cmd.AddValue("controlTestDuration", "duration to run the control measurements", testDuration);
    cmd.AddValue("suspectedTestDuration", "duration to run the measurements that may be policed", testDuration);
    cmd.AddValue("measurementsTransportProtocol", "0 to run measurements as UDP --- 1 to run measurements as TCP", isTCP);
    cmd.AddValue("TCPProtocol", "congestion control algorithm of the TCP Measurement traffic", tcpProtocol);
    cmd.AddValue("appDataRate", "in case of infinite tcp application: the rate of generating data", appDataRate);
    cmd.AddValue("pktSize", "the probe packets size", pktSize);
    cmd.AddValue("lambda", "in case of packet_probes/constant application: the inter-arrival time includes probs", lambda);
    cmd.AddValue("replayTrace", "in case of measurement replay application: the trace to replay", replayTrace);
    cmd.AddValue("backgroundDir", "directory for the background traces to use as cross traffic", backgroundDir);
    cmd.AddValue("isNeutral", "0 to run a neutral scenario --- 1 for policing", isNeutral);
    cmd.AddValue("policingRate", "throttling rate used in case of policing (in Mbps) ", policingRate);
    cmd.AddValue("policingBurstLength", "allowed burst length in case of policing (in sec)", burstLength);
    cmd.AddValue("policerLocation", "'c' for common link --- 'nci' for noncommon link of path i --- 'nc' for all noncommon links", policerLocation);
    cmd.AddValue("policerType", "'0 for shared policer --- 1 for per-flow policer", policerType);
    cmd.AddValue("backThrottledPct", "percentage of background to throttle with the measurement traffic", throttlingPctOfBack);
    cmd.AddValue("overflowEventsTrace", "file that contains times in which policer experienced overflow events", overflowEventsTrace);
    cmd.Parse(argc, argv);
    /*** end of defining inputs ***/

    srand(ns3::RngSeedManager::GetSeed());

    /*** Time Parameters ***/
    Time testStartTime = Seconds(10);
    Time testEndTime = testStartTime + Seconds(testDuration);

    /*** Input-Output parameters ***/
    string resultsPath = (string) (getenv("PWD")) + "/results/" + resultsFolderName;
    string dataPath = (string) (getenv("PWD")) + "/data/";

    /*** Topology Parameters ***/
    uint32_t nbApps = 1;
    uint32_t nbServers = nbApps;

    /*** Traffic classifiers on which to throttle packets ***/
    TrafficClassifier dscpsClassifier = TrafficClassifier({
        new Dscps2QueueBand(0, {0}), new Dscps2QueueBand(1, {1, 3}), new Dscps2QueueBand(2, {2})});

    /*** Traffic Parameters ***/
    uint32_t rcvBufSize = 131072, sndBufSize = 131072;
    uint32_t mss = 1228, mtu = 1500;
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpCubic"));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(mss));
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(rcvBufSize));
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(sndBufSize));
    Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(1));
    Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(MilliSeconds(200)));
    Config::SetDefault("ns3::TcpL4Protocol::RecoveryType", TypeIdValue(TcpClassicRecovery::GetTypeId()));
    Config::SetDefault("ns3::TcpSocketState::EnablePacing", BooleanValue (true));

/* ################################################## READ AND PREPARE PARAMETERS (END) ################################################# */


/* ################################################ BUILD AND CONFIGURE TOPOLOGY (START) ################################################ */
    /*** Create the topology ***/
    NodeContainer routers; routers.Create(2); // router[0] is actually the mobile client
    NodeContainer serverNodes; serverNodes.Create(nbServers);

    InternetStackHelper internetStackHelper;
    internetStackHelper.Install(routers);
    internetStackHelper.Install(serverNodes);

    string defaultNonCommonLinkRate = "1Gbps", defaultLinkDelay = "2ms";
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("Mtu", UintegerValue(mtu));
    p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1p"));

    NetDeviceContainer channels_r1_servers[nbServers];

    vector<string> nonCommonLinkDelays(nbServers, defaultLinkDelay);
    if (nonCommonLinksDelaysStr != "empty") { nonCommonLinkDelays = SplitStr(nonCommonLinksDelaysStr, ','); }

    vector<string> nonCommonLinkRates(nbServers, defaultNonCommonLinkRate);
    if (nonCommonLinksDataRatesStr != "empty") { nonCommonLinkRates = SplitStr(nonCommonLinksDataRatesStr, ','); }

    for (uint32_t i = 0; i < nbServers; i++) {
        p2p.SetDeviceAttribute("DataRate", StringValue(nonCommonLinkRates[i]));
        p2p.SetChannelAttribute("Delay", StringValue(nonCommonLinkDelays[i]));
        channels_r1_servers[i] = p2p.Install(serverNodes.Get(i), routers.Get(1));

        string queueSize = ComputeQueueSize(
                nonCommonLinkRates[i], {nonCommonLinkDelays[i], defaultLinkDelay});

        // set the queues to fifo queueing discipline
        TrafficControlHelper tch;
        tch.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", StringValue(queueSize));
        tch.Install(channels_r1_servers[i]);

        // check if you need to install a policer instead of fifo
        if ((isNeutral != 0) && DoesPolicerLocationMatch("nc" + to_string(i), policerLocation)) {
            double rate = policingRate;
            TrafficControlHelper policerTch = CbQueueDisc::GenerateDisc1FifoNPolicers(
                    queueSize, dscpsClassifier, rate, burstLength,
                    15000, resultsPath + "/noncommon_link_" + to_string(i));

            const Ptr<NetDevice> &netDevice = serverNodes.Get(i)->GetDevice(1);
            tch.Uninstall(netDevice);
            policerTch.Install(netDevice);
        }
    }

    // Parameters for the common channel
    PointToPointHelper p2pRouters;
    p2pRouters.SetDeviceAttribute("DataRate", StringValue(commonLinkRate));
    p2pRouters.SetChannelAttribute("Delay", StringValue(defaultLinkDelay));
    p2pRouters.SetDeviceAttribute("Mtu", UintegerValue(mtu));
    p2pRouters.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1p"));
    NetDeviceContainer channel_r0_r1 = p2pRouters.Install(routers.Get(0), routers.Get(1));

    // Modify the traffic control layer module of the router 0 net device to implement policing
    TrafficControlHelper tch;
    string queueSize = ComputeQueueSize(commonLinkRate, {
            *max_element(begin(nonCommonLinkDelays), end(nonCommonLinkDelays)), defaultLinkDelay});
    tch.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", StringValue(queueSize));
    tch.Install(channel_r0_r1);
    if ((isNeutral != 0) && DoesPolicerLocationMatch("c", policerLocation)) {
        TrafficControlHelper policerTch = CbQueueDisc::GenerateDisc1FifoNPolicers(
                queueSize, dscpsClassifier, policingRate, burstLength, 15000, resultsPath + "/common_link");

        const Ptr<NetDevice> &netDevice = routers.Get(1)->GetDevice(routers.Get(1)->GetNDevices() - 1);
        tch.Uninstall(netDevice);
        policerTch.Install(netDevice);
    }


    Ipv4AddressHelper ipv4;
    uint16_t nbSubnets = 0;
    ipv4.SetBase(("10.1." + to_string(++nbSubnets) + ".0").c_str(), "255.255.255.0");
    Ipv4InterfaceContainer addresses_r0_r1 = ipv4.Assign(channel_r0_r1);
    Ipv4InterfaceContainer addresses_dsts_r0[nbServers];
    for (uint32_t i = 0; i < nbServers; i++) {
        ipv4.SetBase(("10.1." + to_string(++nbSubnets) + ".0").c_str(), "255.255.255.0");
        addresses_dsts_r0[i] = ipv4.Assign(channels_r1_servers[i]);
    }
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
/* ############################################### BUILD AND CONFIGURE TOPOLOGY (END) ################################################## */


/* ########################################### SETUP THE APPLICATIONS AND BACKGROUND (START) ########################################### */
    // Some parameters to define the architecture
    Ptr<Node> routerR = routers.Get(1), client = routers.Get(0);
    Ipv4Address clientIP = GetNodeIP(client, 1);
    Ptr<Node> appsServer[nbApps];
    string appsTag[nbApps];
    vector<AppKey> appsKey;

    int trafficDscp[nbApps];
    for (uint32_t i = 0; i < nbApps; i++) { trafficDscp[i] = 1; }
    if (IsPolicerTypePerFlowPolicer(policerType)) { trafficDscp[nbApps - 1] = 2; }

    /*** Create Application Traffic ***/
    for (uint32_t i = 0; i < nbApps; i++) {
        uint32_t appPath = i % 2;
        appsTag[i] = "suspected_app" + to_string(appPath);
        appsServer[i] = serverNodes.Get(appPath);

        // only in case of WeheCS both client and server are created by the application.
        if (appType == 5) {
            WeheCS* weheCS = WeheCS::CreateWeheCS(
                    appsTag[i], client, appsServer[i], dataPath + "/" + replayTrace, isTCP, trafficDscp[i], resultsPath);
            weheCS->StartApplication(testStartTime);
            weheCS->StopApplication(testEndTime);
            appsKey.emplace_back(GetNodeIP(appsServer[i], 1), clientIP, weheCS->GetPort(), 0);
            continue;
        }

        appsKey.emplace_back(GetNodeIP(appsServer[i], 1), clientIP, 0, 3001 + i);

        // create the application at destination
        PacketSinkHelper sinkAppHelper(GetSocketFactory(isTCP), InetSocketAddress(Ipv4Address::GetAny(), appsKey[i].GetDstPort()));
        ApplicationContainer sinkApp = sinkAppHelper.Install(client);
        sinkApp.Start(testStartTime);
        sinkApp.Stop(testEndTime);

        // create the client sending traffic
        InetSocketAddress sinkAddress = InetSocketAddress(appsKey[i].GetDstIp(), appsKey[i].GetDstPort());
        sinkAddress.SetTos(Dscp2Tos(trafficDscp[i])); // used for policing to set the traffic type
        ApplicationContainer app;
        if (appType == 1) {
            app = ProbesSenderHelper::CreateConstantProbeApplication(
                    appsTag[i], sinkAddress, isTCP, lambda, pktSize, resultsPath, appsServer[i]);
        }
        else if (appType == 2) {
            app = ProbesSenderHelper::CreatePoissonApplication(
                    appsTag[i], sinkAddress, isTCP, lambda, pktSize, resultsPath, appsServer[i]);
        }
        else if (appType == 3) {
            app = MeasurementReplaySenderHelper::CreateMeasurementReplayApplication(
                    appsTag[i], sinkAddress, isTCP, dataPath + replayTrace, resultsPath,appsServer[i]);
        }
        else if (appType == 4) {
            app = InfiniteTCPSenderHelper::CreateInfiniteTcpApplication(
                    appsTag[i], sinkAddress, tcpProtocol, pktSize, resultsPath,appsServer[i], appDataRate);
        }
        app.Start(testStartTime);
        app.Stop(testEndTime);
    }
/* ########################################### SETUP THE APPLICATIONS AND BACKGROUND (END) ########################################### */


/* ############################################## RUN SIMULATION AND MONITORING (START) ############################################## */
#if PCAP_FLAG /*** Record Pcap files for channels ***/
    AsciiTraceHelper ascii;
//    p2p.EnableAsciiAll(ascii.CreateFileStream(resultsPath + "/tracing.tr"));
    p2pRouters.EnablePcapAll(resultsPath + "/pcap_files");
//    p2pRouters.EnablePcap(resultsPath + "/router-link", 0, 1);
#endif

    /*** Attach Packet Monitors ***/
    vector<PacketMonitor *> appsMonitors;
    for (uint32_t i = 0; i < nbApps; i++) {
        auto *pathMonitor = new PacketMonitor(
                testStartTime,  testEndTime, appsServer[i], client, "path_" + appsTag[i]);
        pathMonitor->AddAppKey(appsKey[i]);
        appsMonitors.push_back(pathMonitor);
    }

    /*** Run simulation ***/
    cout << "Start Simulation" << endl;
    Simulator::Stop(testEndTime + Seconds(2.0));
    Simulator::Run();
    Simulator::Destroy();

    for (uint32_t i = 0; i < nbApps; i++) {
        appsMonitors[i]->SavePacketRecords(resultsPath + "/path_" + appsTag[i] + "_packets.csv");
    }
/* ############################################## RUN SIMULATION AND MONITORING (END) ############################################## */

    auto stop = high_resolution_clock::now();
    cout << "Total execution time = " << duration_cast<microseconds>(stop - start).count() << " microsecond" << endl;

    return 0;
}
