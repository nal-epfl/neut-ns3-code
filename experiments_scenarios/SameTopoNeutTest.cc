//
// Created by nal on 20.01.22.
//
// Network topology
//
//                            +--- d0
//      s0 ---+-- r0 ___ r1 --+--- d1
//                            +--- d2
//
// - All links are P2P
// - Wehe applications runs along d0->s0, d1->s0 back-to-back or simultaneously
// - App 1 and 3 along path d0->s0   &&   App 2 and 4 along path d1->s0
// - Background is CAIDA traces
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
#include "../traffic_generator_module/trace_replay/MultipleReplayClients.h"
#include "../traffic_generator_module/poisson/PoissonClientHelper.h"
#include "../traffic_generator_module/measurement_replay/MeasurReplayClientHelper.h"
#include "../traffic_generator_module/infinite_tcp/InfiniteTCPClientHelper.h"
#include "../traffic_differentiation_module/CbQueueDisc.h"
#include "../traffic_generator_module/wehe_cs/WeheCS.h"

using namespace ns3;
using namespace std;
using namespace std::chrono;
namespace fs = std::filesystem;

#define PCAP_FLAG 0

/*****************************************************************************/
int run_same_topo_neut_test(int argc, char **argv) {
    auto start = high_resolution_clock::now();

/* ################################################ READ AND PREPARE PARAMETERS (START) ################################################ */
    /*** Defining Inputs: Variables read from arguments ***/
    string commonLinkRate = "180Mbps"; // the transmission rate of the common link
    float duration = 120.; // for how long measurement traffic should run
    string resultsFolderName; // the output folder to save files generated by the monitors
    int isTCP = 0; // 0 to run wehe app as UDP --- 1 to run it as TCP
    string tcpProtocol = "ns3::TcpCubic"; // in case of TCP the congestion control algorithm to use
    uint32_t appType = 0; // the type of measurement application
    string appDataRate = "20Mbps";
    uint32_t pktSize = 256; // size of probe packets
    double lambda = 0.001; // rate for constand and lambda for poisson
    string replayTrace = "empty"; // specific traffic to send along the measurement paths
    string backgroundDir = "empty"; // specifiy which background traffic to use in our experiments
    int isNeutral = 0; // 0 to Run a neutral appType
    double policingRate = 4; // the rate at which tokens in the token bucket are generated
    double burstLength = 0.1; // the lnegth of the burst parameter of the token bucket
    int throttleUdp = 0; // 0 -> do not throttle, 1 -> otherwise
    string linksDelaysStr = "empty"; // you can pass the propagation delays of the non-common links
    string linksDataRatesStr = "empty"; // you can pass the link bandwidth of the non-common links

    CommandLine cmd;
    cmd.AddValue("linkRate", "the rate of the common link", commonLinkRate);
    cmd.AddValue("duration", "the duration of the the simulation", duration);
    cmd.AddValue("resultsFolderName", "the name of the folder to save results to", resultsFolderName);
    cmd.AddValue("appProtocol", "the protocol used by the wehe app: 0=udp, 1=tcp", isTCP);
    cmd.AddValue("TCPProtocol", "the tcp congestion control protocol", tcpProtocol);
    cmd.AddValue("appType", "The measurement application to run", appType);
    cmd.AddValue("pktSize", "pktSize", pktSize);
    cmd.AddValue("lambda", "lambda", lambda);
    cmd.AddValue("replayTrace", "file to replay by the Client", replayTrace);
    cmd.AddValue("appDataRate", "The application rate of generating data", appDataRate);
    cmd.AddValue("backgroundDir", "directory for the background traces to use as cross traffic", backgroundDir);
    cmd.AddValue("neutral", "to enable neutral bottleneck behaviour (0 means neutral)", isNeutral);
    cmd.AddValue("policingRate", "rate used in case of policing (in Mbps) ", policingRate);
    cmd.AddValue("policingBurstLength", "duration of burst (in sec)", burstLength);
    cmd.AddValue("throttleUdp", "0 to throttle udp traffic, 1 otherwise", throttleUdp);
    cmd.AddValue("nonCommonlinksDelays", "the propagation delay of the 5 non-common links", linksDelaysStr);
    cmd.AddValue("nonCommonlinksDataRates", "the bandwidth of the 5 non-common links", linksDataRatesStr);
    cmd.Parse(argc, argv);
    /*** end of defining inputs ***/

    srand(ns3::RngSeedManager::GetSeed());

    /*** Time Parameters ***/
    bool runBackToBack = true;
    uint16_t controlTestId = 0, suspectedTestId = 1;
    Time warmupTime = Seconds(10);
    Time testsStartTime[2], testsEndTime[2];
    testsStartTime[controlTestId] = warmupTime;
    testsEndTime[controlTestId] = testsStartTime[controlTestId] + Seconds(duration);
    testsStartTime[suspectedTestId] = runBackToBack ? warmupTime + testsEndTime[controlTestId] : testsStartTime[controlTestId];
    testsEndTime[suspectedTestId] = testsStartTime[suspectedTestId] + Seconds(duration);

    /*** Input-Output parameters ***/
    string resultsPath = (string) (getenv("PWD")) + "/results/" + resultsFolderName;
    string dataPath = (string) (getenv("PWD")) + "/data/";

    /*** Topology Parameters ***/
    uint32_t nbApps = 4;
    uint32_t nbServers = nbApps/2 + 1; // the +1 is for the last path which carries only back traffic

    /*** Traffic Parameters ***/
    string appProtocol = (isTCP == 1) ? "ns3::TcpSocketFactory" : "ns3::UdpSocketFactory";
    uint32_t rcvBufSize = 131072, sndBufSize = 131072;
    uint32_t mss = 1228, mtu = 1500;
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpCubic"));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(mss));
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(rcvBufSize));
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(sndBufSize));
    Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(1));
    Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(MilliSeconds(200)));
    Config::SetDefault("ns3::TcpL4Protocol::RecoveryType", TypeIdValue(TcpClassicRecovery::GetTypeId()));
    Config::SetDefault ("ns3::TcpSocketState::EnablePacing", BooleanValue (true));

    /*** Interpret isNeutral Case ***/
    bool isPolicerShared = false, isPolicerIndependent = false;
    bool policerOnCommonLink = false, policerOnNonCommonForApp3 = false, policerOnNonCommonForApp4 = false;
    if (isNeutral == 1 || isNeutral == -1) {
        isPolicerShared = true, policerOnCommonLink = true;
    }
    else if(isNeutral == 2 || isNeutral == -2) {
        isPolicerIndependent = true, policerOnCommonLink = true;
    }
    else if(isNeutral == 3 || isNeutral == -3) {
        isPolicerShared = true, policerOnNonCommonForApp3 = true, policerOnNonCommonForApp4 = true;
    }
    else if(isNeutral == 4 || isNeutral == -4) {
        isPolicerIndependent = true, policerOnNonCommonForApp3 = true, policerOnNonCommonForApp4 = true;
    }
    else if(isNeutral == 5 || isNeutral == -5) {
        isPolicerShared = true, policerOnNonCommonForApp4 = true;
    }

/* ################################################## READ AND PREPARE PARAMETERS (END) ################################################# */


/* ################################################ BUILD AND CONFIGURE TOPOLOGY (START) ################################################ */
    /*** Create the topology ***/
    NodeContainer routers; routers.Create(2); // router[0] is actually the mobile client
    NodeContainer serverNodes; serverNodes.Create(nbServers);

    InternetStackHelper internetStackHelper;
    internetStackHelper.Install(routers);
    internetStackHelper.Install(serverNodes);

    string defaultNonCommonLinkRate = "1Gbps", defaultLinkDelay = "5ms";
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("Mtu", UintegerValue(mtu));
    p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1p"));

    NetDeviceContainer channels_r1_servers[nbServers];
    vector<string> nonCommonLinkDelays(nbServers, defaultLinkDelay);
    if (linksDelaysStr != "empty") { nonCommonLinkDelays = HelperMethods::SplitStr(linksDelaysStr, ','); }

    vector<string> nonCommonLinkRates(nbServers, defaultNonCommonLinkRate);
    if (linksDataRatesStr != "empty") { nonCommonLinkRates = HelperMethods::SplitStr(linksDataRatesStr, ','); }

    for (uint32_t i = 0; i < nbServers; i++) {
        p2p.SetDeviceAttribute("DataRate", StringValue(nonCommonLinkRates[i]));
        p2p.SetChannelAttribute("Delay", StringValue(nonCommonLinkDelays[i]));
        channels_r1_servers[i] = p2p.Install(serverNodes.Get(i), routers.Get(1));

        string queueSize = HelperMethods::ComputeQueueSize(
                nonCommonLinkRates[i], {nonCommonLinkDelays[i], defaultLinkDelay});

        // set the queues to fifo queueing discipline
        TrafficControlHelper tch;
        tch.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", StringValue(queueSize));
        tch.Install(channels_r1_servers[i]);
        // For the policers on non-common links
        if ((i == 0 && policerOnNonCommonForApp3) || (i == 1 && policerOnNonCommonForApp4)) {
            double rate = policingRate; //(i == 1) ? 15 : policingRate;
            TrafficControlHelper policerTch = CbQueueDisc::GenerateDisc1FifoNPolicers(
                    queueSize, {0, 4, 8}, rate, burstLength, resultsPath + "/noncommon_link_" + to_string(i));

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
    string queueSize = HelperMethods::ComputeQueueSize(commonLinkRate, {defaultLinkDelay, defaultLinkDelay});
    tch.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", StringValue(queueSize));
    tch.Install(channel_r0_r1);
    if (policerOnCommonLink) {
        TrafficControlHelper policerTch = CbQueueDisc::GenerateDisc1FifoNPolicers(
                queueSize, {0, 4, 8}, policingRate, burstLength, resultsPath + "/common_link");

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
    Ipv4Address clientIP = HelperMethods::GetNodeIP(client, 1);
    vector<Ptr<Node>> appsServer;

    vector<AppKey> appsKey;
    int trafficClass[] = {0, 0, 0, 0};
    if (isNeutral > 0) {
        trafficClass[2] = 4;
        trafficClass[3] = isPolicerIndependent? 8 : 4;
    }
    uint32_t testId[] = {controlTestId, controlTestId, suspectedTestId, suspectedTestId};
    /*** Create Application Traffic ***/
    for (uint32_t i = 0; i < nbApps; i++) {
        appsServer.push_back(serverNodes.Get(i % 2));
        appsKey.emplace_back(HelperMethods::GetNodeIP(appsServer[i], 1), clientIP, 0, 3001 + i);

        // exceptional case of WeheCS (Wehe Client-Server)
        if (appType == 5) {
            WeheCS* weheCS = WeheCS::CreateWeheCS(client, appsServer[i], dataPath + "/weheCS_trace", isTCP, trafficClass[i], resultsPath);
            weheCS->StartApplication(testsStartTime[testId[i]]);
            weheCS->StopApplication(testsEndTime[testId[i]]);
            continue;
        }

        // create the application at destination
        PacketSinkHelper sinkAppHelper(appProtocol, InetSocketAddress(Ipv4Address::GetAny(), appsKey[i].GetDstPort()));
        ApplicationContainer sinkApp = sinkAppHelper.Install(client);
        sinkApp.Start(testsStartTime[testId[i]]);
        sinkApp.Stop(testsEndTime[testId[i]]);

        // create the client sending traffic
        InetSocketAddress sinkAddress = InetSocketAddress(appsKey[i].GetDstIp(), appsKey[i].GetDstPort());
        sinkAddress.SetTos(trafficClass[i]); // used for policing to set the traffic type
        ApplicationContainer app;
        if (appType == 1) {
            app = PoissonClientHelper::CreateConstantProbeApplication(
                    sinkAddress, isTCP, lambda, pktSize, resultsPath, appsServer[i]);
        }
        else if (appType == 2) {
            app = PoissonClientHelper::CreatePoissonApplication(
                    sinkAddress, isTCP, lambda, pktSize, resultsPath, appsServer[i]);
        }
        else if (appType == 3) {
            app = MeasurReplayClientHelper::CreateMeasurementReplayApplication(
                    sinkAddress, isTCP, dataPath + replayTrace, resultsPath, appsServer[i]);
        }
        else if (appType == 4) {
            app = InfiniteTCPClientHelper::CreateInfiniteTcpApplication(
                    sinkAddress, tcpProtocol, pktSize, resultsPath, appsServer[i], appDataRate);
        }
        app.Start(testsStartTime[testId[i]]);
        app.Stop(testsEndTime[testId[i]]);
    }

    /*** Create Cross Traffic On Paths 1 & 2 ***/
    auto *backP0 = new MultipleReplayClients(appsServer[0], client);
    double throttledProbP0 = (isPolicerShared) ? 0.3 : 0;
    string tracesPathP0 = dataPath + backgroundDir + "/link0";
    if (fs::exists(tracesPathP0)) {
        backP0->RunTracesWithRandomThrottledTCPFlows(tracesPathP0, throttledProbP0, 4);
    } else {
        cout << "requested Background Directory does not exist" << endl;
    }

    auto *backP1 = new MultipleReplayClients(appsServer[1], client);
    double throttledProbP1 = (isPolicerShared) ? 0.3 : 0;
    string tracesPathP1 = dataPath + backgroundDir + "/link1";
    if (fs::exists(tracesPathP1)) {
        backP1->RunTracesWithRandomThrottledTCPFlows(tracesPathP1, throttledProbP1, 4);
    } else {
        cout << "requested Background Directory does not exist" << endl;
    }

/* ########################################### SETUP THE APPLICATIONS AND BACKGROUND (END) ########################################### */


/* ############################################## RUN SIMULATION AND MONITORING (START) ############################################## */
#if PCAP_FLAG /*** Record Pcap files for channels ***/
    AsciiTraceHelper ascii;
//    p2p.EnableAsciiAll(ascii.CreateFileStream(resultsPath + "/tracing.tr"));
    p2pRouters.EnablePcapAll(resultsPath + "/pcap_files");
//    p2pRouters.EnablePcap(resultsPath + "/router-link", 0, 1);
#endif

    /*** Attach a Packet Monitor ***/
    auto *commonLinkMonitor = new PacketMonitor(
            testsStartTime[controlTestId], testsEndTime[suspectedTestId], routerR->GetId(), client->GetId(), "commonLink"
    );
    vector<PacketMonitor *> appsMonitors;
    vector<PacketMonitor *> nonCommonLinkMonitors;
    for (uint32_t i = 0; i < nbApps; i++) {
        commonLinkMonitor->AddAppKey(appsKey[i]);

        auto *appMonitor = new PacketMonitor(testsStartTime[testId[i]], testsEndTime[testId[i]], appsServer[i]->GetId(), client->GetId(), "app" + to_string(i));
        appMonitor->AddAppKey(appsKey[i]);
        appsMonitors.push_back(appMonitor);

        auto *nonCommonLinkMonitor = new PacketMonitor(testsStartTime[testId[i]], testsEndTime[testId[i]], appsServer[i]->GetId(), routerR->GetId(), "noncommonLink" + to_string(i));
        nonCommonLinkMonitor->AddAppKey(appsKey[i]);
        nonCommonLinkMonitors.push_back(nonCommonLinkMonitor);
    }

    /*** Run simulation ***/
    cout << "Start Simulation" << endl;
    Simulator::Stop(testsEndTime[suspectedTestId] + warmupTime);
    Simulator::Run();
    Simulator::Destroy();

    commonLinkMonitor->SaveRecordedPacketsCompact(resultsPath + "/common_link_packets.csv");
    for (uint32_t i = 0; i < nbApps; i++) {
        appsMonitors[i]->SaveRecordedPacketsCompact(resultsPath + "/app" + to_string(i) + "_packets.csv");
        nonCommonLinkMonitors[i]->SaveRecordedPacketsCompact(resultsPath + "/app" + to_string(i) + "_noncommon_link_packets.csv");
    }
/* ############################################## RUN SIMULATION AND MONITORING (END) ############################################## */

    auto stop = high_resolution_clock::now();
    cout << "Total execution time = " << duration_cast<microseconds>(stop - start).count() << " microsecond" << endl;

    return 0;
}




