//
// Created by nal on 22.12.20.
//
// Network topology
//
//       r0 --- r1
//
// - Single Infinite TCP flow form r0 to r1

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

#include "../traffic_differentiation_module/CbQueueDisc.h"


using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("1PathTCP");

#define PCAP_FLAG 1
#define PACKET_MONITOR_FLAG 1
#define LOSS_MONITOR_FLAG 0
#define POLICING_FLAG 1

/*****************************************************************************/
int run_1path_tcp(int argc, char **argv) {

    LogComponentEnable ("1PathTCP", LOG_LEVEL_INFO);
    NS_LOG_INFO("TCP Flow is running on single path");

    /*** Variables read from arguments ***/
    float duration = 120.;
    string resultsFolderName;
    string btlkLinkRate = "180Mbps";
    CommandLine cmd;
    cmd.AddValue("duration", "the duration of the the simulation", duration);
    cmd.AddValue("resultsFolderName", " the name of the folder to save results to", resultsFolderName);
    cmd.AddValue("LinkRate", " the rate of the bottleneck link", btlkLinkRate);
    cmd.Parse(argc, argv);


    /*** Simulation Parameters ***/
    Time simStartTime = Seconds(0.);
    Time simEndTime = Seconds(duration) + Seconds(10);
    string resultsPath = (string)(getenv("PWD")) + "/results" + resultsFolderName;
    string dataPath = (string)(getenv("PWD")) + "/data";
    cout << "simulation end time " << simEndTime << endl;


    // should change here the RTT to 0.02
    string queueSize = to_string(int(0.035 * (DataRate(btlkLinkRate).GetBitRate() * 0.125))) + "B"; // RTT * link_rate

    /*** Traffic Parameters ***/
    string trafficProtocol =  "ns3::TcpSocketFactory";

    uint32_t rcvBufSize = 2e9;
    uint32_t mss = 1228;
    uint32_t mtu = 1500;
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (mss));
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (rcvBufSize));
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue (rcvBufSize));


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
    double rate = 5;
    string policingRate = to_string(rate) + "Mbps";
    int burst = 3125;//rate * 125000;
    cout << "the burst size of the token bucket " << burst << endl;

    uint16_t handle = tch.SetRootQueueDisc("ns3::CbQueueDisc", "MaxSize", StringValue(queueSize),
                                           "TosMap", TosMapValue(TosMap{0, 4}));

    TrafficControlHelper::ClassIdList cid = tch.AddQueueDiscClasses (handle, 2, "ns3::QueueDiscClass");
    tch.AddChildQueueDisc (handle, cid[0], "ns3::FifoQueueDisc", "MaxSize", StringValue(queueSize));
    tch.AddChildQueueDisc (handle, cid[1], "ns3::TbfQueueDiscChild",
                           "Burst", UintegerValue (burst),
                           "Mtu", UintegerValue (mtu),
                           "Rate", DataRateValue (DataRate (policingRate)),
                           "PeakRate", DataRateValue (DataRate ("0bps")));
#else
    cout << "queue size: " << queueSize << endl;
    tch.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", StringValue(queueSize));
#endif
    tch.Install(channel_r0_r1);


    Ipv4AddressHelper ipv4;
    ipv4.SetBase(("10.1.1.0"), "255.255.255.0");
    Ipv4InterfaceContainer addresses_r0_r1 = ipv4.Assign(channel_r0_r1);


    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    /*** Create Measurement Traffic ***/
    PacketSinkHelper sink(trafficProtocol, InetSocketAddress(Ipv4Address::GetAny (), 3001));
    ApplicationContainer sinkApp = sink.Install(routers.Get(1));
    sinkApp.Start(simStartTime);

    OnOffHelper client(trafficProtocol, InetSocketAddress(routers.Get(1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 3001));
    client.SetAttribute ("PacketSize", UintegerValue (1228));
    client.SetAttribute ("DataRate", DataRateValue (DataRate("3.5Mbps")));
    ApplicationContainer clientApp = client.Install(routers.Get(0));
    clientApp.Start(simStartTime);


#if PCAP_FLAG /*** Record Pcap files for channels ***/
    AsciiTraceHelper ascii;
//    p2pRouters.EnableAsciiAll(ascii.CreateFileStream(resultsPath + "/tracing.tr"));
    p2pRouters.EnablePcapAll(resultsPath + "/router-link");

    AnimationInterface::SetConstantPosition(routers.Get(0), 0.0, 20.0);
    AnimationInterface::SetConstantPosition(routers.Get(1), 20.0, 20.0);

    AnimationInterface anim(resultsPath + "/routers-anim.xml");
    anim.EnablePacketMetadata(true);
    anim.SetStartTime(simStartTime);
#endif

    uint32_t routersIds[2] = {routers.Get(0)->GetId(), routers.Get(1)->GetId()};

#if PACKET_MONITOR_FLAG
    PacketMonitor* bottleneckPktMonitorUp = new PacketMonitor(simStartTime, Seconds(duration), routersIds[0], routersIds[1], "bottleneckUp");
    bottleneckPktMonitorUp->AddAppKey(addresses_r0_r1.GetAddress(0), addresses_r0_r1.GetAddress(1));

    PacketMonitor* bottleneckPktMonitorDown = new PacketMonitor(simStartTime, Seconds(duration), routersIds[1], routersIds[0], "bottleneckDown");
    bottleneckPktMonitorDown->AddAppKey(addresses_r0_r1.GetAddress(1), addresses_r0_r1.GetAddress(0));
#endif

    /*** Run simulation ***/
    NS_LOG_INFO("Run Simulation.");
    Simulator::Stop(simEndTime);
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");


#if PACKET_MONITOR_FLAG
    bottleneckPktMonitorUp->SaveRecordedPacketsCompact(resultsPath + "/bottleneck_packets_up.csv");
    bottleneckPktMonitorDown->SaveRecordedPacketsCompact(resultsPath + "/bottleneck_packets_down.csv");
#endif

    return 0;
}
