//
// Created by nal on 09.02.21.
//
// Network topology
//
//      s0 ___+               +___ d0
//      s1 ___+__ r0 --- r1 __+___ d1
//        ... +               + ...
//    sn-1 ___+               +___ dn-1
//
// - All links are P2P
// - Wehe data traces between s0->d0, ..., sn-2->dn-2
// - Background CAIDA traffic

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

#include "../traffic_differentiation_module/TokenBucket.h"

#include "../traffic_generator_module/wehe_cs/WeheCS.h"
#include "../traffic_generator_module/trace_replay/MultipleReplayClients.h"

using namespace ns3;
using namespace std;
using namespace std::chrono;

NS_LOG_COMPONENT_DEFINE("NWeheCSwithCAIDA");

#define PCAP_FLAG 0
#define PACKET_MONITOR_FLAG 1

/*****************************************************************************/
int run_NWeheCS_w_CAIDA(int argc, char **argv) {
    auto start = high_resolution_clock::now();

    LogComponentEnable ("NWeheCSwithCAIDA", LOG_LEVEL_INFO);
    NS_LOG_INFO("N Wehe CS with CAIDA background is running");
//    LogComponentEnable("PacketMetadata", LOG_LEVEL_ALL);


    /*** Variables read from arguments ***/
    float duration = 120.;
    string resultsFolderName;
    string btlkLinkRate = "180Mbps";
    int weheProtocol = 0;
    string tcpProtocol = "ns3::TcpNewReno";
    int isNeutral = 1;
    double policingRate = 6;
    string nonCommonRate = "10Gbps";
    CommandLine cmd;
    cmd.AddValue("duration", "the duration of the the simulation", duration);
    cmd.AddValue("resultsFolderName", "the name of the folder to save results to", resultsFolderName);
    cmd.AddValue("linkRate", "the rate of the bottleneck link", btlkLinkRate);
    cmd.AddValue("weheAppProtocol", "the protocol used by the wehe app: 0=udp, 1=tcp", weheProtocol);
    cmd.AddValue("TCPProtocol", "the tcp congestion control protocol", tcpProtocol);
    cmd.AddValue("neutral", "to enable neutral bottleneck behaviour (0 means enable policing)", isNeutral);
    cmd.AddValue("policingRate", "rate used in case of policing (in Mbps) ", policingRate);
    cmd.AddValue("nonCommonRate", "the rate of the non-common link ", nonCommonRate);
    cmd.Parse(argc, argv);


    /*** Simulation Parameters ***/
    Time simStartTime = Seconds(0.);
    Time warmupTime = Seconds(15);
    Time simEndTime = warmupTime + Seconds(duration) + Seconds(10);
    string resultsPath = (string)(getenv("PWD")) + "/results" + resultsFolderName;
    string dataPath = (string)(getenv("PWD")) + "/data";


    /*** Topology Parameters ***/
    int nbWeheCS = 4;
    int nbDsts = nbWeheCS + 1;
//    int nbSrcs = 1;
    int trafficClass[] = {0, 0, 4, 4};

    /*** Traffic Parameters ***/
    string weheAppProtocol = (weheProtocol == 1) ? "ns3::TcpSocketFactory" : "ns3::UdpSocketFactory";
    cout << weheAppProtocol << endl;

    uint32_t rcvBufSize = 2e9;
    uint32_t mss = 1500;
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue (tcpProtocol));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (mss));
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (rcvBufSize));
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue (rcvBufSize));
//  Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(Seconds(0)));


    /*** Create the dumbbell topology ***/
    NS_LOG_INFO("Create Topology.");
//    NodeContainer srcNodes; srcNodes.Create(nbSrcs);
    NodeContainer routers; routers.Create(2);
    NodeContainer dstNodes; dstNodes.Create(nbDsts);

    InternetStackHelper internetStackHelper;
//    internetStackHelper.Install(srcNodes);
    internetStackHelper.Install(routers);
    internetStackHelper.Install(dstNodes);


    string defaultDataRate = "10Gbps";
    string defaultLinkDelay = "10ms";
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(defaultDataRate));
    p2p.SetChannelAttribute("Delay", StringValue("10ms"));
    p2p.SetDeviceAttribute("Mtu", UintegerValue(2000));
    p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1p"));

//    NetDeviceContainer channels_srcs_r0[nbSrcs];
//    for(int i = 0; i < nbSrcs; i++) {
//        channels_srcs_r0[i] = p2p.Install(srcNodes.Get(i), routers.Get(0));
//        // set the queues to fifo queueing discipline
//        TrafficControlHelper tch;
//        string queueSize = to_string(int(0.035 * (DataRate(defaultDataRate).GetBitRate() * 0.125))) + "B"; // RTT * link_rate
//        tch.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", StringValue(queueSize));
//        tch.Install(channels_srcs_r0[i]);
//    }

    NetDeviceContainer channels_r1_dsts[nbDsts];
    string dataRates[] = {defaultDataRate, defaultDataRate, nonCommonRate, nonCommonRate, defaultDataRate};
    string delays[] = {defaultLinkDelay, defaultLinkDelay, defaultLinkDelay, defaultLinkDelay, defaultLinkDelay};
    for(int i = 0; i < nbDsts; i++) {
        // to create scenarios on the non-common links
        p2p.SetDeviceAttribute("DataRate", StringValue(dataRates[i]));
        p2p.SetChannelAttribute("Delay", StringValue(delays[i]));
        channels_r1_dsts[i] = p2p.Install( dstNodes.Get(i), routers.Get(1));
        // set the queues to fifo queueing discipline
        TrafficControlHelper tch;
        string queueSize = to_string(int(0.035 * (DataRate(dataRates[i]).GetBitRate() * 0.125))) + "B"; // RTT * link_rate
        tch.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", StringValue(queueSize));
        tch.Install(channels_r1_dsts[i]);
    }




    // Parameters for the bottleneck channel
    PointToPointHelper p2pRouters;
    p2pRouters.SetDeviceAttribute("DataRate", StringValue(btlkLinkRate));
    p2pRouters.SetChannelAttribute("Delay", StringValue("10ms"));
    p2pRouters.SetDeviceAttribute("Mtu", UintegerValue(2000));
    p2pRouters.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1p"));
    NetDeviceContainer channel_r0_r1 = p2pRouters.Install(routers.Get(0), routers.Get(1));


    // Modify the traffic control layer module of the router 0 net device to implement policing
    TrafficControlHelper tch;
    // should change here the RTT to 0.035
    string queueSize = to_string(int(0.035 * (DataRate(btlkLinkRate).GetBitRate() * 0.125))) + "B"; // RTT * link_rate
    if(isNeutral == 0) {
        cout << "we have policing with rate " << policingRate << "Mbps";

        double burstLength = 1; // in sec
        int burst = floor(policingRate * burstLength * 125000);// in byte
        cout << ", and burst duration " << burstLength << " sec, giving burst = " << burst << " Byte." << endl;

        TokenBucket policerForTos4(4, burst, to_string(policingRate) + "Mbps");
        tch.SetRootQueueDisc("ns3::CbPolicingQueueDisc", "MaxSize", StringValue(queueSize), "TokenBucket",
                             TokenBucketValue(policerForTos4));
    }
    else {
        cout << "queue size: " << queueSize << endl;
        tch.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", StringValue(queueSize));
    }
    tch.Install(channel_r0_r1);


    Ipv4AddressHelper ipv4;
    uint16_t nbSubnets = 0;
//    Ipv4InterfaceContainer addresses_srcs_r0[nbSrcs];
//    Ipv4Address srcAddresses[nbSrcs];
//    for(int i = 0; i < nbSrcs; i++) {
//        ipv4.SetBase(("10.1." + to_string(++nbSubnets) + ".0").c_str(), "255.255.255.0");
//        addresses_srcs_r0[i] = ipv4.Assign(channels_srcs_r0[i]);
//        srcAddresses[i] = addresses_srcs_r0[i].GetAddress(0);
//    }
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


    /*** Create Wehe Traffic ***/
    for(int i = 0; i < nbWeheCS; i++) {
        WeheCS* weheCS = new WeheCS(routers.Get(0), dstNodes.Get(i), weheAppProtocol);
        weheCS->SetTos(trafficClass[i]);
        weheCS->SetResultsFolder(resultsPath);
        weheCS->LoadTrace(dataPath + "/weheCS_trace");
        if(weheProtocol == 1) weheCS->EnableCwndMonitor();
        weheCS->StartApplication(warmupTime);
        weheCS->StopApplication(simEndTime);
    }

    /*** Create Background Traffic ***/
    MultipleReplayClients* back = new MultipleReplayClients(dstNodes.Get(nbDsts-1), routers.Get(0));
    back->RunAllTraces(dataPath + "/chicago_2010_back_traffic_1", 5392, 1);


#if PCAP_FLAG /*** Record Pcap files for channels ***/
    AsciiTraceHelper ascii;
    p2p.EnableAsciiAll(ascii.CreateFileStream(resultsPath + "/tracing.tr"));
    p2pRouters.EnablePcapAll(resultsPath + "/router-link");
#endif

//    uint32_t srcIds[nbSrcs]; for(int i = 0; i < nbSrcs; i++) { srcIds[i] = srcNodes.Get(i)->GetId(); }
    uint32_t routersIds[2] = {routers.Get(0)->GetId(), routers.Get(1)->GetId()};
    uint32_t dstIds[nbDsts]; for(int i = 0; i < nbDsts; i++) { dstIds[i] = dstNodes.Get(i)->GetId(); }

#if PACKET_MONITOR_FLAG
//    PacketMonitor* bottleneckPktMonitorUp = new PacketMonitor(warmupTime, Seconds(duration), routersIds[0], routersIds[1], "bottleneckUp");
//    for(int i = 0; i < nbWeheCS; i++) bottleneckPktMonitorUp->AddAppKey(dstAddresses[i]);
//
//    vector<PacketMonitor*> pathPktsMonitorsUp;
//    for(int i = 0; i < nbWeheCS; i++) {
//        PacketMonitor* pathMonitor = new PacketMonitor(warmupTime, Seconds(duration), srcIds[i], dstIds[i],  "path" + to_string(i) + "Up");
//        pathMonitor->AddAppKey(dstAddresses[i]);
//        pathPktsMonitorsUp.push_back(pathMonitor);
//    }

    PacketMonitor* bottleneckPktMonitorDown = new PacketMonitor(warmupTime, simEndTime-warmupTime, routersIds[1], routersIds[0], "bottleneckDown");
    for(int i = 0; i < nbWeheCS; i++)
        bottleneckPktMonitorDown->AddAppKey(dstAddresses[i], addresses_r0_r1.GetAddress(0));

    vector<PacketMonitor*> pathPktsMonitorsDown;
    for(int i = 0; i < nbWeheCS; i++) {
        PacketMonitor* pathMonitor = new PacketMonitor(warmupTime, simEndTime-warmupTime, dstIds[i], routersIds[0],  "path" + to_string(i) + "Down");
        pathMonitor->AddAppKey(dstAddresses[i], addresses_r0_r1.GetAddress(0));
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
//    bottleneckPktMonitorUp->SaveRecordedPacketsFor1Path(resultsPath + "/bottleneck_packets_up.csv");
//    for(int i = 0; i < nbWeheCS - 1; i++) {
//        pathPktsMonitorsUp[i]->SaveRecordedPacketsFor1Path(resultsPath + "/path" + to_string(i) + "_packets_up.csv");
//    }

    bottleneckPktMonitorDown->SaveRecordedPacketsCompact(resultsPath + "/bottleneck_packets_down.csv");
    for(int i = 0; i < nbWeheCS; i++) {
        pathPktsMonitorsDown[i]->SaveRecordedPacketsCompact(resultsPath + "/path" + to_string(i) + "_packets_down.csv");
    }
#endif

    auto stop = high_resolution_clock::now();
    cout << "Total execution time = " << duration_cast<microseconds>(stop - start).count() << " microsecond" << endl;

    return 0;
}

