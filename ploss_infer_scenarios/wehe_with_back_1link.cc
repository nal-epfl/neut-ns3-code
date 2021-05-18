//
// Created by nal on 26.10.20.
//
// Network topology
//
//      s0 ___+               +___ d0
//      s1 ___+__ r0 --- r1 __+___ d1
//        ... +               + ...
//    sn-1 ___+               +___ dn-1
//
// - n apps are running and the link between r0 and r1 is the only physical link
// - Wehe data traces between s0->d0, s1->d1, sn-2->dn-2
// - Background UDP flow with Poisson Pareto Bursts between s2-d2

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
#include "../monitors_module/CwndMonitor.h"

#include "../traffic_generator_module/trace_replay/TraceReplayClientServer.h"
#include "../traffic_generator_module/ppb/PPBBidirectional.h"

#include "../traffic_differentiation_module/TokenBucket.h"


using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("WeheWithBack");

#define PCAP_FLAG 1
#define PACKET_MONITOR_FLAG 1
#define POLICING_FLAG 1

/*****************************************************************************/
int run_wehe_with_back_on_1link(int argc, char **argv) {

    LogComponentEnable ("WeheWithBack", LOG_LEVEL_INFO);
    NS_LOG_INFO("Wehe traces with Back is running on single physical link");
//    LogComponentEnable("TcpSocket", LOG_LEVEL_ALL);


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
    Time simEndTime = warmupTime + Seconds(duration) + Seconds(10);
    string resultsPath = (string)(getenv("PWD")) + "/results" + resultsFolderName;
    string dataPath = (string)(getenv("PWD")) + "/data";

    /*** Topology Parameters ***/
    int nbApps = 5;
    int trafficClass[] = {0, 0, 4, 4};

    // should change here the RTT to 0.02
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
    NodeContainer routers; routers.Create(2);

    InternetStackHelper internetStackHelper;
    internetStackHelper.Install(routers);


    // Parameters for the bottleneck channel
    PointToPointHelper p2pRouters;
    p2pRouters.SetDeviceAttribute("DataRate", StringValue(btlkLinkRate));
    p2pRouters.SetDeviceAttribute("Mtu", UintegerValue(2000));
    p2pRouters.SetChannelAttribute("Delay", StringValue("10ms"));
    p2pRouters.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1p"));
    NetDeviceContainer channel_r0_r1 = p2pRouters.Install(routers.Get(0), routers.Get(1));


    // Modify the traffic control layer module of the node 0 net device to implement policing
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
//    tch.SetRootQueueDisc("ns3::RedQueueDisc", "MaxSize", StringValue(queueSize));
    tch.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", StringValue(queueSize));
#endif
    tch.Install(channel_r0_r1);


    Ipv4AddressHelper ipv4;
    uint16_t nbSubnets = 0;
    ipv4.SetBase(("10.1." + to_string(++nbSubnets) + ".0").c_str(), "255.255.255.0");
    Ipv4InterfaceContainer addresses_r0_r1 = ipv4.Assign(channel_r0_r1);


    Ipv4GlobalRoutingHelper::PopulateRoutingTables();


    /*** Create Measurement Traffic ***/
    for(int i = 0; i < nbApps - 1; i++) {
        TraceReplayClientServer *replayClientServer = new TraceReplayClientServer();
        replayClientServer->SetClient(routers.Get(0));
        replayClientServer->SetServer(routers.Get(1));
        replayClientServer->SetProtocol(trafficProtocol);
        replayClientServer->SetTrafficsTos(trafficClass[i], trafficClass[i]);
        replayClientServer->SetTracesPath(dataPath + "/wehe_trace_server", dataPath + "/");
        if(weheProtocol == 1) replayClientServer->EnableCwndMonitor(resultsPath + "/cong_algo_info_" + to_string(i));
        replayClientServer->Setup(warmupTime, simEndTime-Seconds(1));
    }

    /*** Create Background Traffic ***/
    TraceReplayClientServer *replayClientServerBack = new TraceReplayClientServer();
    replayClientServerBack->SetClient(routers.Get(0));
    replayClientServerBack->SetServer(routers.Get(1));
    replayClientServerBack->SetProtocol("ns3::TcpSocketFactory");
    replayClientServerBack->SetTrafficsTos(0, 0);
    replayClientServerBack->SetTracesPath(dataPath + "/ppb_back", dataPath + "/");
    replayClientServerBack->Setup(simStartTime, simEndTime);


#if PCAP_FLAG /*** Record Pcap files for channels ***/
    AsciiTraceHelper ascii;
//    p2pRouters.EnableAsciiAll(ascii.CreateFileStream(resultsPath + "/tracing.tr"));
    p2pRouters.EnablePcapAll(resultsPath + "/router-link");

    AnimationInterface::SetConstantPosition(routers.Get(0), 20.0, 0.0);
    AnimationInterface::SetConstantPosition(routers.Get(1), 20.0, 20.0);

    AnimationInterface anim(resultsPath + "/routers-anim.xml");
    anim.EnablePacketMetadata(true);
    anim.SetStartTime(Seconds(30));
#endif

    uint32_t routersIds[2] = {routers.Get(0)->GetId(), routers.Get(1)->GetId()};

#if PACKET_MONITOR_FLAG
    PacketMonitor* bottleneckPktMonitorUp = new PacketMonitor(warmupTime, Seconds(duration), routersIds[0], routersIds[1], "bottleneckUp");
    bottleneckPktMonitorUp->AddAppKey(addresses_r0_r1.GetAddress(0), addresses_r0_r1.GetAddress(1), 0);

    PacketMonitor* bottleneckPktMonitorDown = new PacketMonitor(warmupTime, Seconds(duration), routersIds[1], routersIds[0], "bottleneckDown");
    bottleneckPktMonitorDown->AddAppKey(addresses_r0_r1.GetAddress(1), addresses_r0_r1.GetAddress(0), 0);
#endif

//#if CWND_MONITOR
//    vector<CwndMonitor*> cwndMonitors;
//    for(int i = 0; i < nbApps - 1; i++) { cwndMonitors.push_back(new CwndMonitor(routersIds[0], i*2)); }
//#endif


    /*** Run simulation ***/
    NS_LOG_INFO("Run Simulation.");
    Simulator::Stop(simEndTime);
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");


#if PACKET_MONITOR_FLAG
    bottleneckPktMonitorUp->SaveRecordedPacketsFor1Path(resultsPath + "/bottleneck_packets_up.csv");
    bottleneckPktMonitorDown->SaveRecordedPacketsFor1Path(resultsPath + "/bottleneck_packets_down.csv");
#endif

//#if CWND_MONITOR
//    for(int i = 0; i < nbApps - 1; i++) {
//        cwndMonitors[i]->SaveCwndChanges(resultsPath + "/cwnd_changes_" + to_string(i) + ".csv");
////        cwndMonitors[i]->SaveCongStateChanges(resultsPath + "/cong_state_changes_" + to_string(i) + ".csv");

//#endif

    return 0;
}


