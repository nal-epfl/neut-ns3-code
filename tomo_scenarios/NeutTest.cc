//
// Created by nal on 20.10.21.
//
// Network topology
//
//                            +--- d0
//      s0 ---+-- r0 ___ r1 --+--- d1
//                            +--- d2
//
// - All links are P2P
// - Wehe applications runs along d0->s0, d1->s0, d2->s0, and d3->s0
// - Background is CAIDA traces

#include <iostream>
#include <string>
#include <chrono>

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
#include "../traffic_differentiation_module/TokenBucket.h"

using namespace ns3;
using namespace std;
using namespace std::chrono;

NS_LOG_COMPONENT_DEFINE("NeutTest");

#define PCAP_FLAG 0
#define PACKET_MONITOR_FLAG 1

int run_neut_test(int argc, char **argv) {
    auto start = high_resolution_clock::now();

    NS_LOG_INFO("Four Probing applications with CAIDA background is running");

    /*** Defining Inputs: Variables read from arguments ***/
    float duration = 120.; // for how long measurement traffic should run
    string resultsFolderName; // the output folder to save files generated by the monitors
    string commonLinkRate = "180Mbps"; // the transmission rate of the common link
    int isTCP = 0; // 0 to run wehe app as UDP --- 1 to run it as TCP
    string tcpProtocol = "ns3::TcpCubic"; // in case of TCP the congestion control algorithm to use
    uint32_t scenario = 0; // This is used to enable running different scenarios (e.g., congesion on non-common link)
    uint32_t pktSize = 256; // size of probe packets
    double lambda = 0.001; // rate for constand and lambda for poisson
    string replayTrace = ""; // specific traffic to send along the measurement paths
    int isNeutral = 1; // 1 to Run a neutral scenario  --- 0 to enable policing
    double policingRate = 4; // the rate at which tokens in the token bucket are generated
    double burstLength = 0.1; // the lnegth of the burst parameter of the token bucket

    CommandLine cmd;
    cmd.AddValue("duration", "the duration of the the simulation", duration);
    cmd.AddValue("resultsFolderName", "the name of the folder to save results to", resultsFolderName);
    cmd.AddValue("linkRate", "the rate of the common link", commonLinkRate);
    cmd.AddValue("appProtocol", "the protocol used by the wehe app: 0=udp, 1=tcp", isTCP);
    cmd.AddValue("TCPProtocol", "the tcp congestion control protocol", tcpProtocol);
    cmd.AddValue("scenario", "it is defined based on what is in the NeutTestWeheCleanCode file", scenario);
    cmd.AddValue("pktSize", "pktSize", pktSize);
    cmd.AddValue("lambda", "lambda", lambda);
    cmd.AddValue("replayTrace", "file to replay by the Client", replayTrace);
    cmd.AddValue("neutral", "to enable neutral bottleneck behaviour (0 means enable policing)", isNeutral);
    cmd.AddValue("policingRate", "rate used in case of policing (in Mbps) ", policingRate);
    cmd.AddValue("policingBurstLength", "duration of burst (in sec)", burstLength);
    cmd.Parse(argc, argv);
    /*** end of defining inputs ***/

    srand(ns3::RngSeedManager::GetSeed());

    cout << "tcp procotol is " << tcpProtocol << endl;

    /*** Time Parameters ***/
    Time simStartTime = Seconds(0.);
    Time warmupTime = Seconds(10); // time before start sending measurement traffic to eliminate transient period
    Time simEndTime = warmupTime + Seconds(duration) + Seconds(5);

    /*** Input-Output parameters ***/
    string resultsPath = (string)(getenv("PWD")) + "/results" + resultsFolderName;
    string dataPath = (string)(getenv("PWD")) + "/data";

    /*** Topology Parameters ***/
    uint32_t nbApps = 4;
    uint32_t nbServers = nbApps + 1; // the +1 is for the last path which carries only back traffic (could be eliminated)


    /*** Traffic Parameters ***/
    string appProtocol = (isTCP == 1) ? "ns3::TcpSocketFactory" : "ns3::UdpSocketFactory";

    uint32_t rcvBufSize = 2e9;
    uint32_t sndBufSize = 2e9;
    uint32_t mss = 1228;
    uint32_t mtu = 1500;
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue (tcpProtocol));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (mss));
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (rcvBufSize));
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue (sndBufSize));
    Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue (1));
    Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue (MilliSeconds(200)));
    Config::SetDefault("ns3::TcpL4Protocol::RecoveryType", TypeIdValue (TcpClassicRecovery::GetTypeId()));


    /*** Create the topology ***/
    NS_LOG_INFO("Create Topology.");
    NodeContainer routers; routers.Create(2); // router[0] is actually the mobile client
    NodeContainer serverNodes; serverNodes.Create(nbServers);

    InternetStackHelper internetStackHelper;
    internetStackHelper.Install(routers);
    internetStackHelper.Install(serverNodes);

    string defaultCommonDataRate = "10Gbps";
    string defaultNonCommonDataRate = "1Gbps";
    string defaultLinkDelay = "5ms";
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("Mtu", UintegerValue(mtu));
    p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1p"));

    NetDeviceContainer channels_r1_servers[nbServers];
    string delays[] = {defaultLinkDelay, defaultLinkDelay, defaultLinkDelay, defaultLinkDelay, defaultLinkDelay};
    string dataRates[] = {defaultNonCommonDataRate, defaultNonCommonDataRate, defaultNonCommonDataRate, defaultNonCommonDataRate, defaultCommonDataRate};

    for(uint32_t i = 0; i < nbServers; i++) {
        p2p.SetDeviceAttribute("DataRate", StringValue(dataRates[i]));
        p2p.SetChannelAttribute("Delay", StringValue(delays[i]));
        channels_r1_servers[i] = p2p.Install(serverNodes.Get(i), routers.Get(1));

        // set the queues to fifo queueing discipline
        TrafficControlHelper tch;
        string queueSize = to_string(int(0.025 * (DataRate(dataRates[i]).GetBitRate() * 0.125))) + "B"; // RTT * link_rate
        tch.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", StringValue(queueSize));
        tch.Install(channels_r1_servers[i]);
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
    string queueSize = to_string(int(0.025 * (DataRate(commonLinkRate).GetBitRate() * 0.125))) + "B"; // RTT * link_rate
    if(isNeutral == 0) {
        cout << "we have policing with rate " << policingRate << "Mbps";

        int burst = floor(policingRate * burstLength * 125000);// in byte
        cout << ", and burst duration " << burstLength << " sec, giving burst = " << burst << " Byte." << endl;

        TokenBucket policerForTos4(4, burst, to_string(policingRate) + "Mbps");
        TokenBucket policerForTos8(8, burst, to_string(policingRate) + "Mbps");
        tch.SetRootQueueDisc("ns3::CbPolicingQueueDisc", "MaxSize", StringValue(queueSize), "TokenBucket",
                             TokenBucketValue(policerForTos4), "TokenBucket1", TokenBucketValue(policerForTos8));
    }
    else {
        cout << "queue size: " << queueSize << endl;
        tch.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", StringValue(queueSize));
    }
    tch.Install(channel_r0_r1);


    Ipv4AddressHelper ipv4;
    uint16_t nbSubnets = 0;
    ipv4.SetBase(("10.1." + to_string(++nbSubnets) + ".0").c_str(), "255.255.255.0");
    Ipv4InterfaceContainer addresses_r0_r1 = ipv4.Assign(channel_r0_r1);
    Ipv4InterfaceContainer addresses_dsts_r0[nbServers];
    Ipv4Address dstAddresses[nbServers];
    for(uint32_t i = 0; i < nbServers; i++) {
        ipv4.SetBase(("10.1." + to_string(++nbSubnets) + ".0").c_str(), "255.255.255.0");
        addresses_dsts_r0[i] = ipv4.Assign(channels_r1_servers[i]);
        dstAddresses[i] = addresses_dsts_r0[i].GetAddress(0);
    }


    Ipv4GlobalRoutingHelper::PopulateRoutingTables();


    uint16_t appPorts[nbApps];
    int trafficClass[] = {0, 0, 4, 8};
    /*** Create Wehe Traffic ***/
    for(uint32_t i = 0; i < nbApps; i++) {
        // create the application at destination
        uint32_t sinkPort = 3001 + i;
        PacketSinkHelper sinkAppHelper(appProtocol, InetSocketAddress(Ipv4Address::GetAny (), sinkPort));
        ApplicationContainer sinkApp = sinkAppHelper.Install(routers.Get(0));
        sinkApp.Start(warmupTime);
        sinkApp.Stop(simEndTime);

        // create the client sending poisson
        InetSocketAddress sinkAddress = InetSocketAddress(addresses_r0_r1.GetAddress(0), sinkPort);
        sinkAddress.SetTos(trafficClass[i]); // used for policing to set the traffic type

        ApplicationContainer app;

        if (scenario == 1 || scenario == 2) {
            PoissonClientHelper poissonClientHelper(sinkAddress);
            poissonClientHelper.SetAttribute("Protocol", StringValue(appProtocol));
            if (scenario == 1) {
                poissonClientHelper.SetAttribute("Interval", StringValue(
                        "ns3::ConstantRandomVariable[Constant=" + to_string(lambda) + "]"));
            } else if (scenario == 2) {
                poissonClientHelper.SetAttribute("Interval", StringValue(
                        "ns3::ExponentialRandomVariable[Mean=" + to_string(lambda) + "]"));
            }
            poissonClientHelper.SetAttribute("PacketSize", UintegerValue(pktSize));
            if (isTCP == 1) {
                poissonClientHelper.SetAttribute("EnableCwndMonitor", BooleanValue(true));
                poissonClientHelper.SetAttribute("ResultsFolder", StringValue(resultsPath));
            }
            app = poissonClientHelper.Install(serverNodes.Get(i));
        }
        else if (scenario == 3) {
            MeasurReplayClientHelper replayClientHelper(sinkAddress);
            replayClientHelper.SetAttribute("Protocol", StringValue(appProtocol));
            replayClientHelper.SetAttribute("TraceFile", StringValue(dataPath + "/" + replayTrace));
            if(isTCP == 1) {
                replayClientHelper.SetAttribute("EnableCwndMonitor", BooleanValue(true));
                replayClientHelper.SetAttribute("CongAlgoFolder", StringValue(resultsPath));
            }
            app = replayClientHelper.Install(serverNodes.Get(i));
        }


        app.Start(warmupTime);
        app.Stop(warmupTime + Seconds(duration));

        // for the packet monitoring
        appPorts[i] = sinkPort;
    }

    /*** Create Background Traffic ***/
    MultipleReplayClients* back = new MultipleReplayClients(serverNodes.Get(nbServers-1), routers.Get(0));
    back->RunAllTraces(dataPath + "/chicago_2010_back_traffic_10min", 3173, 1, 4);
//    back->RunAllTraces(dataPath + "/chicago_2010_links_back_traffic_0/chicago_2010_back_traffic_1min_link2", 773, 1, 4);


#if PCAP_FLAG /*** Record Pcap files for channels ***/
    AsciiTraceHelper ascii;
//    p2p.EnableAsciiAll(ascii.CreateFileStream(resultsPath + "/tracing.tr"));
    p2pRouters.EnablePcapAll(resultsPath + "/pcap_files");
//    p2pRouters.EnablePcap(resultsPath + "/router-link", 0, 1);
#endif

    uint32_t routersIds[2] = {routers.Get(0)->GetId(), routers.Get(1)->GetId()};
    uint32_t dstIds[nbServers]; for(uint32_t i = 0; i < nbServers; i++) { dstIds[i] = serverNodes.Get(i)->GetId(); }

#if PACKET_MONITOR_FLAG
    PacketMonitor* bottleneckPktMonitorDown = new PacketMonitor(warmupTime, Seconds(duration), routersIds[1], routersIds[0], "bottleneckDown");
    for(uint32_t i = 0; i < nbApps; i++)
        bottleneckPktMonitorDown->AddAppKey(dstAddresses[i], addresses_r0_r1.GetAddress(0), 0, 0);

    vector<PacketMonitor*> pathPktsMonitorsDown;
    for(uint32_t i = 0; i < nbApps; i++) {
        PacketMonitor* pathMonitor = new PacketMonitor(warmupTime, Seconds(duration), dstIds[i], routersIds[0],  "path" + to_string(i) + "Down");
        pathMonitor->AddAppKey(dstAddresses[i], addresses_r0_r1.GetAddress(0), 0, appPorts[i]);
        pathPktsMonitorsDown.push_back(pathMonitor);
    }

#endif

    /*** Run simulation ***/
    NS_LOG_INFO("Run Simulation.");
    Simulator::Stop(simEndTime);
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");


#if PACKET_MONITOR_FLAG
    bottleneckPktMonitorDown->SaveRecordedPacketsCompact(resultsPath + "/bottleneck_packets_down.csv");
    for(uint32_t i = 0; i < nbApps; i++) {
        pathPktsMonitorsDown[i]->SaveRecordedPacketsCompact(resultsPath + "/path" + to_string(i) + "_packets_down.csv");
    }
#endif

    auto stop = high_resolution_clock::now();
    cout << "Total execution time = " << duration_cast<microseconds>(stop - start).count() << " microsecond" << endl;

    return 0;
}


