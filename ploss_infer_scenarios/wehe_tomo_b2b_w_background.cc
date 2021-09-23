//
// Created by nal on 03.02.21.
//
// Network topology
//
//      s0 ___+               +___ d0
//      s1 ___+__ r0 --- r1 __+___ d1
//      s3 ___+               +___ d3
//
// - All links are P2P
// - Wehe data traces between s0->d0, s1->d1
// - Background UDP flow with Poisson Pareto Bursts between s3-d3

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
#include "../monitors_module/LossMonitor.h"

#include "../traffic_generator_module/trace_replay/TraceReplayClientServer.h"
#include "../traffic_generator_module/ppb/PPBBidirectional.h"
#include "../traffic_differentiation_module/TokenBucket.h"


using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("WeheB2BWithPpbNoise");

#define PCAP_FLAG 0
#define PACKET_MONITOR_FLAG 1
#define POLICING_FLAG 0

/*****************************************************************************/
int run_wehe_tomo_b2b_w_background(int argc, char **argv) {

    LogComponentEnable ("WeheB2BWithPpbNoise", LOG_LEVEL_INFO);
    NS_LOG_INFO("Wehe b2b traces with PPB Noise is running");


    /*** Variables read from arguments ***/
    float duration = 120.;
    string resultsFolderName;
    string btlkLinkRate = "180Mbps";
    int weheProtocol = 0;
    string tcpProtocol = "ns3::TcpNewReno";
    CommandLine cmd;
    cmd.AddValue("duration", "the duration of the the simulation", duration);
    cmd.AddValue("resultsFolderName", " the name of the folder to save results to", resultsFolderName);
    cmd.AddValue("LinkRate", " the rate of the bottleneck link", btlkLinkRate);
    cmd.AddValue("WeheAppProtocol", " the protocol used by the wehe app: 0=udp, 1=tcp", weheProtocol);
    cmd.AddValue("TCPProtocol", " the tcp congestion control protocol", tcpProtocol);
    cmd.Parse(argc, argv);

    cout << "tcp protocol used: " << tcpProtocol << endl;


    /*** Simulation Parameters ***/
    Time simStartTime = Seconds(0.);
    Time warmupTime = Seconds(25);
    Time simEndTime = warmupTime + 2*Seconds(duration) + Seconds(10);
    string resultsPath = (string)(getenv("PWD")) + "/results" + resultsFolderName;
    string dataPath = (string)(getenv("PWD")) + "/data";


    /*** Topology Parameters ***/
    int nbPaths = 3;
    int nbDsts = nbPaths;
    int nbSrcs = nbPaths;

    // should change here the RTT to 0.035
    string queueSize = to_string(int(0.035 * (DataRate(btlkLinkRate).GetBitRate() * 0.125))) + "B"; // RTT * link_rate

    /*** Traffic Parameters ***/
    string trafficProtocol = (weheProtocol == 1) ? "ns3::TcpSocketFactory" : "ns3::UdpSocketFactory";
    cout << trafficProtocol << endl;

    if(weheProtocol == 1) {
        uint32_t rcvBufSize = 2e9;
        uint32_t mss = 1228;
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue (tcpProtocol));
        Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (mss));
        Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (rcvBufSize));
        Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue (rcvBufSize));
//        Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(Seconds(0)));
    }


    /*** Create the dumbbell topology ***/
    NS_LOG_INFO("Create Topology.");
    NodeContainer srcNodes; srcNodes.Create(nbSrcs);
    NodeContainer routers; routers.Create(2);
    NodeContainer dstNodes; dstNodes.Create(nbDsts);

    InternetStackHelper internetStackHelper;
    internetStackHelper.Install(srcNodes);
    internetStackHelper.Install(routers);
    internetStackHelper.Install(dstNodes);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
    p2p.SetDeviceAttribute("Mtu", UintegerValue(2000));
    p2p.SetChannelAttribute("Delay", StringValue("10ms"));
    p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1p"));
    NetDeviceContainer channels_srcs_r0[nbSrcs];
    for(int i = 0; i < nbSrcs; i++) channels_srcs_r0[i] = p2p.Install(srcNodes.Get(i), routers.Get(0));
    NetDeviceContainer channels_r1_dsts[nbDsts];
//    string dataRates[] = {"10Gbps", "10Gbps", "10Gbps", "10Mbps", "10Gbps"};
    for(int i = 0; i < nbDsts; i++) {
//        p2p.SetDeviceAttribute("DataRate", StringValue(dataRates[i])); // to create congestion on a non-common link
        channels_r1_dsts[i] = p2p.Install( dstNodes.Get(i), routers.Get(1));
    }

    // Parameters for the bottleneck channel
    PointToPointHelper p2pRouters;
    p2pRouters.SetDeviceAttribute("DataRate", StringValue(btlkLinkRate));
    p2pRouters.SetDeviceAttribute("Mtu", UintegerValue(2000));
    p2pRouters.SetChannelAttribute("Delay", StringValue("10ms"));
    p2pRouters.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1p"));
    NetDeviceContainer channel_r0_r1 = p2pRouters.Install(routers.Get(0), routers.Get(1));


    // Modify the traffic control layer module of the router 0 net device to implement policing
    TrafficControlHelper tch;
#if POLICING_FLAG
    double rate = 8;
    string policingRate = to_string(rate) + "Mbps";
    cout << "we have policing with rate " << policingRate;

    double burstLength = 1; // in sec
    int burst = floor(rate * burstLength * 125000);// in byte
    cout << ", and burst duration " << burstLength << " sec, giving burst = " << burst << " Byte." << endl;

    TokenBucket policerForTos4(4, burst, policingRate);
    tch.SetRootQueueDisc("ns3::CbPolicingQueueDisc", "MaxSize", StringValue (queueSize), "TokenBucket", TokenBucketValue(policerForTos4));
#else
    cout << "queue size: " << queueSize << endl;
    tch.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", StringValue(queueSize));
#endif
    tch.Install(channel_r0_r1);


    Ipv4AddressHelper ipv4;
    uint16_t nbSubnets = 0;
    Ipv4InterfaceContainer addresses_srcs_r0[nbSrcs];
    Ipv4Address srcAddresses[nbSrcs];
    for(int i = 0; i < nbSrcs; i++) {
        ipv4.SetBase(("10.1." + to_string(++nbSubnets) + ".0").c_str(), "255.255.255.0");
        addresses_srcs_r0[i] = ipv4.Assign(channels_srcs_r0[i]);
        srcAddresses[i] = addresses_srcs_r0[i].GetAddress(0);
    }
    ipv4.SetBase(("10.1." + to_string(++nbSubnets) + ".0").c_str(), "255.255.255.0");
    Ipv4InterfaceContainer addresses_r0_r1 = ipv4.Assign(channel_r0_r1);
    Ipv4InterfaceContainer addresses_dsts_r0[nbDsts];
    Ipv4Address dstAddresses[nbDsts];
    for(int i = 0; i < nbDsts; i++) {
        ipv4.SetBase(("10.1." + to_string(++nbSubnets) + ".0").c_str(), "255.255.255.0");
        addresses_dsts_r0[i] = ipv4.Assign(channels_r1_dsts[i]);
        dstAddresses[i] = addresses_dsts_r0[i].GetAddress(0);
    }


    Ipv4GlobalRoutingHelper::PopulateRoutingTables();


    /*** Create Measurement Traffic ***/
    // for the first test with control traffic
    ns3::Time appStarTime = warmupTime;
    for(int i = 0; i < nbDsts - 1; i++) {
        TraceReplayClientServer *replayClientServer = new TraceReplayClientServer();
        replayClientServer->SetClient(srcNodes.Get(i));
        replayClientServer->SetServer(dstNodes.Get(i));
        replayClientServer->SetProtocol(trafficProtocol);
        replayClientServer->SetTrafficsTos(0, 0);
        replayClientServer->SetTracesPath(dataPath + "/wehe_trace_server", dataPath + "/");
        if(weheProtocol == 1) replayClientServer->EnableCwndMonitor(resultsPath + "/cong_algo_info_" + to_string(i));
        replayClientServer->Setup(appStarTime, appStarTime + Seconds(duration));
    }

    // for the second test with original traffic
    appStarTime = appStarTime + Seconds(duration+1);
    for(int i = 0; i < nbDsts - 1; i++) {
        TraceReplayClientServer *replayClientServer = new TraceReplayClientServer();
        replayClientServer->SetClient(srcNodes.Get(i));
        replayClientServer->SetServer(dstNodes.Get(i));
        replayClientServer->SetProtocol(trafficProtocol);
        replayClientServer->SetTrafficsTos(4, 4);
        replayClientServer->SetTracesPath(dataPath + "/wehe_trace_server", dataPath + "/");
        if(weheProtocol == 1) replayClientServer->EnableCwndMonitor(resultsPath + "/cong_algo_info_" + to_string(i+2));
        replayClientServer->Setup(appStarTime, appStarTime + Seconds(duration));
    }

    /*** Create Background Traffic ***/
    TraceReplayClientServer *replayClientServerBack = new TraceReplayClientServer();
    replayClientServerBack->SetClient(srcNodes.Get(nbPaths-1));
    replayClientServerBack->SetServer(dstNodes.Get(nbPaths-1));
    replayClientServerBack->SetProtocol("ns3::TcpSocketFactory");
    replayClientServerBack->SetTrafficsTos(0, 0);
    replayClientServerBack->SetTracesPath(dataPath + "/ppb_back_2", dataPath + "/");
    replayClientServerBack->Setup(simStartTime, simEndTime);


#if PCAP_FLAG /*** Record Pcap files for channels ***/
    AsciiTraceHelper ascii;
    p2p.EnableAsciiAll(ascii.CreateFileStream(resultsPath + "/tracing.tr"));
    p2pRouters.EnablePcapAll(resultsPath + "/router-link");
#endif

    uint32_t srcIds[nbSrcs]; for(int i = 0; i < nbSrcs; i++) { srcIds[i] = srcNodes.Get(i)->GetId(); }
    uint32_t routersIds[2] = {routers.Get(0)->GetId(), routers.Get(1)->GetId()};
    uint32_t dstIds[nbDsts]; for(int i = 0; i < nbDsts; i++) { dstIds[i] = dstNodes.Get(i)->GetId(); }

#if PACKET_MONITOR_FLAG
    PacketMonitor* bottleneckPktMonitorUp = new PacketMonitor(warmupTime, simEndTime - warmupTime, routersIds[0], routersIds[1], "bottleneckUp");
    for(int i = 0; i < nbPaths-1; i++) bottleneckPktMonitorUp->AddAppKey(srcAddresses[i], dstAddresses[i]);

    vector<PacketMonitor*> pathPktsMonitorsUp;
    for(int i = 0; i < nbPaths-1; i++) {
        PacketMonitor* pathMonitor = new PacketMonitor(warmupTime, simEndTime - warmupTime, srcIds[i], dstIds[i],  "path" + to_string(i) + "Up");
        pathMonitor->AddAppKey(srcAddresses[i], dstAddresses[i]);
        pathPktsMonitorsUp.push_back(pathMonitor);
    }

//    PacketMonitor* bottleneckPktMonitorDown = new PacketMonitor(warmupTime, Seconds(duration), routersIds[1], routersIds[0], "bottleneckDown");
//    for(int i = 0; i < nbSrcs; i++) bottleneckPktMonitorDown->AddAppKey(srcAddresses[i]);
//
//    vector<PacketMonitor*> pathPktsMonitorsDown;
//    for(int i = 0; i < nbPaths-1; i++) {
//        PacketMonitor* pathMonitor = new PacketMonitor(warmupTime, Seconds(duration), dstIds[i], srcIds[i],  "path" + to_string(i) + "Down");
//        pathMonitor->AddAppKey(srcAddresses[i]);
//        pathPktsMonitorsDown.push_back(pathMonitor);
//    }
#endif


    /*** Run simulation ***/
    NS_LOG_INFO("Run Simulation.");
    Simulator::Stop(simEndTime);
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");


#if PACKET_MONITOR_FLAG
    bottleneckPktMonitorUp->SaveRecordedPacketsCompact(resultsPath + "/bottleneck_packets_up.csv");
    for(int i = 0; i < nbPaths-1; i++) {
        pathPktsMonitorsUp[i]->SaveRecordedPacketsCompact(resultsPath + "/path" + to_string(i) + "_packets_up.csv");
    }

//    bottleneckPktMonitorDown->SaveRecordedPacketsCompact(resultsPath + "/bottleneck_packets_down.csv");
//    for(int i = 0; i < nbPaths-1; i++) {
//        pathPktsMonitorsDown[i]->SaveRecordedPacketsCompact(resultsPath + "/path" + to_string(i) + "_packets_down.csv");
//    }
#endif

    return 0;
}


