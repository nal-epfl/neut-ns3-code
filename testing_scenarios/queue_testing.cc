//
// Created by nal on 30.09.20.
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
#include "../monitors_module/LossMonitor.h"

#include "../traffic_generator_module/trace_replay/TraceReplayClientServer.h"
#include "../traffic_generator_module/ppb/PPBBidirectional.h"


using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("QueueTesting");

#define APP_LOG_FLAG 0
#define PCAP_FLAG 0
#define PACKET_MONITOR_FLAG 0
#define LOSS_MONITOR_FLAG 1
#define POLICING_FLAG 0

/*****************************************************************************/
//ofstream pktsEnqueueOutfile;
//int enCount = 0;
//void enqueueTracing(ns3::Ptr<ns3::Packet const> packet) {
//    pktsEnqueueOutfile << enCount++ << " " << (ns3::Now().GetNanoSeconds() / 1e9) << " " << packet->ToString() << endl;
//}
//ofstream queueOutfile;
//ofstream enqueueOutfile;

//void trace_queue(unsigned int oldSize, unsigned int newSize) {
//    queueOutfile << oldSize << "," << newSize << "," << (Simulator::Now()).GetNanoSeconds() << endl;
//    if(oldSize < newSize) {
//        enqueueOutfile << oldSize << "," << newSize << "," << (Simulator::Now()).GetNanoSeconds() << endl;
//    }
//}
//
//int sent_pkts = 0;
//
//void RecordIpv4PacketSent(string context, Ptr<Packet const> packet, Ptr<Ipv4> ipv4, uint32_t interface) {
//    sent_pkts++;
//}
//
//int received_pkts = 0;
//void RecordIpv4PacketReceived(string context, Ptr<Packet const> packet, Ptr<Ipv4> ipv4, uint32_t interface) {
//    received_pkts++;
//}

int run_queue_testing(int argc, char **argv) {

    LogComponentEnable ("QueueTesting", LOG_LEVEL_INFO);
    NS_LOG_INFO("Test queue setup is running");
//    LogComponentEnable("QueueDisc", LOG_LEVEL_ALL);

#if APP_LOG_FLAG
    LogComponentEnable("UdpClient", LOG_LEVEL_INFO);
    LogComponentEnable("UdpServer", LOG_LEVEL_INFO);
#endif

    /*** Variables read from arguments ***/
    float duration = 120.;
    string resultsFolderName = "";
    CommandLine cmd;
    cmd.AddValue("duration", "the duration of the the simulation", duration);
    cmd.AddValue("resultsFolderName", " the name of the folder to save results to", resultsFolderName);
    cmd.Parse(argc, argv);


    /*** Simulation Parameters ***/
    Time simStartTime = Seconds(0.);
    Time warmupTime = Seconds(0);
    Time simEndTime = warmupTime + Seconds(duration) + Seconds(10);
    string resultsPath = (string)(getenv("PWD")) + "/results" + resultsFolderName;
    string dataPath = (string)(getenv("PWD")) + "/data";


    /*** Topology Parameters ***/
//    string linkPropagationDelay = "10ms";
//    string linkRate = "15Mbps";
    int nbPaths = 2;
    int nbDsts = nbPaths;
    int nbSrcs = nbPaths;


    /*** Traffic Parameters ***/
    string trafficProtocol = "ns3::UdpSocketFactory";
//    auto *upPPBParams = new PPBSettings("0.05Mb/s", 2857, 0.3, 0.8);
//    auto *downPPBParams = new PPBSettings("0.05Mb/s", 2857, 0.3, 0.8);


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
    p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("43750000B"));
    NetDeviceContainer channels_srcs_r0[nbSrcs];
    for(int i = 0; i < nbSrcs; i++) channels_srcs_r0[i] = p2p.Install(srcNodes.Get(i), routers.Get(0));
    NetDeviceContainer channels_r1_dsts[nbDsts];
    for(int i = 0; i < nbDsts; i++) channels_r1_dsts[i] = p2p.Install( dstNodes.Get(i), routers.Get(1));

    // Parameters for the bottleneck channel
    string linkRate = "150Mbps";
    string queueSize = to_string(int(0.035 * (DataRate(linkRate).GetBitRate() * 0.125))) + "B"; // RTT * link_rate
    cout << "the queue size = " << queueSize << endl;
    PointToPointHelper p2pRouters;
    p2pRouters.SetDeviceAttribute("DataRate", StringValue(linkRate));
    p2pRouters.SetDeviceAttribute("Mtu", UintegerValue(2000));
    p2pRouters.SetChannelAttribute("Delay", StringValue("10ms"));
    p2pRouters.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1p"));
    NetDeviceContainer channel_r0_r1 = p2pRouters.Install(routers.Get(0), routers.Get(1));


    // Modify the traffic control layer module of the node 0 net device to implement policing
    TrafficControlHelper tch;
#if POLICING_FLAG
    string policingRate = "0.2Mb/s";
    TokenBucket policerForTos4(4, 2000, policingRate);
    tch.SetRootQueueDisc("ns3::CbPolicingQueueDisc", "MaxSize", StringValue ("1p"), "TokenBucket", TokenBucketValue(policerForTos4));
#else
    tch.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", StringValue(queueSize));
#endif
    tch.Install(channel_r0_r1);
//    pktsEnqueueOutfile.open(resultsPath + "/enqueue_traces.txt");
//    Simulator::Schedule(warmupTime, []() {
//        Config::ConnectWithoutContext("/NodeList/3/DeviceList/3/$ns3::PointToPointNetDevice/MacTx", MakeCallback(&enqueueTracing));
//    }
//    Simulator::Schedule(warmupTime + Seconds(duration), []() {
//        Config::DisconnectWithoutContext("/NodeList/3/DeviceList/3/#ns3::PointToPointNetDevice/MacTx", MakeCallback(&enqueueTracing));
//    });

//    queueOutfile.open(resultsPath + "/btlk_down_queue_size_log.csv");
//    enqueueOutfile.open(resultsPath + "/btlk_down_queue_size_on_enqueue_log.csv");
//    Simulator::Schedule(warmupTime, []() {
//        Config::ConnectWithoutContext("/NodeList/3/DeviceList/3/$ns3::PointToPointNetDevice/TxQueue/BytesInQueue", MakeCallback(&trace_queue));
//    });

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
    TraceReplayClientServer* replayClientServer = new TraceReplayClientServer();
    replayClientServer->SetClient(srcNodes.Get(0));
    replayClientServer->SetServer(dstNodes.Get(0));
    replayClientServer->SetProtocol(trafficProtocol);
    replayClientServer->SetTracesPath(dataPath + "/", dataPath + "/");
    replayClientServer->Setup(warmupTime, simEndTime);

    /*** Create Background Traffic ***/
//    PPBBidirectional* backgroundApp = new PPBBidirectional();
//    backgroundApp->SetUpPPBParams(upPPBParams);
//    backgroundApp->SetDownPPBParams(downPPBParams);
//    backgroundApp->SetHosts(srcNodes.Get(nbSrcs-1), dstNodes.Get(nbDsts-1));
//    backgroundApp->Setup(simStartTime, simEndTime);

    TraceReplayClientServer* backReplayClientServer = new TraceReplayClientServer();
    backReplayClientServer->SetClient(srcNodes.Get(nbSrcs-1));
    backReplayClientServer->SetServer(dstNodes.Get(nbDsts-1));
    backReplayClientServer->SetProtocol(trafficProtocol);
    backReplayClientServer->SetTracesPath(dataPath + "/", dataPath + "/control_trace");
    backReplayClientServer->Setup(warmupTime, simEndTime);

#if PCAP_FLAG /*** Record Pcap files for channels ***/
    AsciiTraceHelper ascii;
    p2p.EnableAsciiAll(ascii.CreateFileStream(resultsPath + "/tracing.tr"));
    p2p.EnablePcapAll(resultsPath + "/dumbbell-topo", false);

#endif


#if PACKET_MONITOR_FLAG
    uint32_t srcIds[nbSrcs]; for(int i = 0; i < nbSrcs; i++) { srcIds[i] = srcNodes.Get(i)->GetId(); }
    uint32_t routersIds[2] = {routers.Get(0)->GetId(), routers.Get(1)->GetId()};
    uint32_t dstIds[nbDsts]; for(int i = 0; i < nbDsts; i++) { dstIds[i] = dstNodes.Get(i)->GetId(); }

//    PacketMonitor* bottleneckPktMonitorUp = new PacketMonitor(warmupTime, Seconds(duration), routersIds[0], routersIds[1], "bottleneckUp");
//    for(int i = 0; i < nbDsts; i++) bottleneckPktMonitorUp->AddDestination(dstAddresses[i]);
//
//    vector<PacketMonitor*> pathMonitorsUp;
//    for(int i = 0; i < nbPaths; i++) {
//        PacketMonitor* pathMonitor = new PacketMonitor(warmupTime, Seconds(duration), srcIds[i], dstIds[i],  "path" + to_string(i) + "Up");
//        pathMonitor->AddDestination(dstAddresses[i]);
//        pathMonitorsUp.push_back(pathMonitor);
//    }

    PacketMonitor* bottleneckPktMonitorDown = new PacketMonitor(warmupTime, Seconds(duration), routersIds[1], routersIds[0], "bottleneckDown");
    for(int i = 0; i < nbSrcs; i++) bottleneckPktMonitorDown->AddDestination(srcAddresses[i]);

    vector<PacketMonitor*> pathMonitorsDown;
    for(int i = 0; i < nbPaths; i++) {
        PacketMonitor* pathMonitor = new PacketMonitor(warmupTime, Seconds(duration), dstIds[i], srcIds[i],  "path" + to_string(i) + "Down");
        pathMonitor->AddAppKey(srcAddresses[i]);
        pathMonitorsDown.push_back(pathMonitor);
    }
#endif

#if LOSS_MONITOR_FLAG
    uint32_t srcIds[nbSrcs]; for(int i = 0; i < nbSrcs; i++) { srcIds[i] = srcNodes.Get(i)->GetId(); }
    uint32_t routersIds[2] = {routers.Get(0)->GetId(), routers.Get(1)->GetId()};
    uint32_t dstIds[nbDsts]; for(int i = 0; i < nbDsts; i++) { dstIds[i] = dstNodes.Get(i)->GetId(); }

//    LossMonitor* bottleneckPktMonitorUp = new LossMonitor(warmupTime, simEndTime, routersIds[0], routersIds[1], "bottleneckUp");
//    for(int i = 0; i < nbDsts; i++) bottleneckPktMonitorUp->AddAppKey(dstAddresses[i]);
//
//    vector<LossMonitor*> pathMonitorsUp;
//    for(int i = 0; i < nbPaths; i++) {
//        LossMonitor* pathMonitor = new LossMonitor(warmupTime, Seconds(duration), srcIds[i], dstIds[i],  "path" + to_string(i) + "Up");
//        pathMonitor->AddAppKey(dstAddresses[i]);
//        pathMonitorsUp.push_back(pathMonitor);
//    }
//
    LossMonitor* bottleneckPktMonitorDown = new LossMonitor(warmupTime, simEndTime, routersIds[1], routersIds[0], "bottleneckDown");
    for(int i = 0; i < nbSrcs; i++) bottleneckPktMonitorDown->AddDestination(srcAddresses[i]);

    vector<LossMonitor*> pathMonitorsDown;
    for(int i = 0; i < nbPaths; i++) {
        LossMonitor* pathMonitor = new LossMonitor(warmupTime, Seconds(duration), dstIds[i], srcIds[i],  "path" + to_string(i) + "Down");
        pathMonitor->AddDestination(srcAddresses[i]);
        pathMonitorsDown.push_back(pathMonitor);
    }
#endif

#if POLICING_FLAG
    TrafficMonitor bottleneckClass1Monitor(intervalTime, "bottleneckC1");
    bottleneckClass1Monitor.AddDestination(sink1Address);
    bottleneckClass1Monitor.Install(0, 1);

    TrafficMonitor bottleneckClass2Monitor(intervalTime, "bottleneckC2");
    bottleneckClass2Monitor.AddAppKey(sink2Address);
    bottleneckClass2Monitor.Install(0, 1);
#endif
//
//    Config::Connect("/NodeList/" + to_string(routersIds[1]) + "/$ns3::Ipv4L3Protocol/Tx", MakeCallback(&RecordIpv4PacketSent));
//    Config::Connect("/NodeList/" + to_string(routersIds[0]) + "/$ns3::Ipv4L3Protocol/Rx", MakeCallback(&RecordIpv4PacketReceived));


    /*** Run simulation ***/
    NS_LOG_INFO("Run Simulation.");
    Simulator::Stop(simEndTime);
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");


#if PACKET_MONITOR_FLAG
    //    bottleneckPktMonitorUp->SaveRecordedPacketsCompact(resultsPath + "/bottleneck_packets_up.csv");
    bottleneckPktMonitorDown->SaveRecordedPacketsCompact(resultsPath + "/bottleneck_packets_down.csv");
    for(int i = 0; i < nbPaths; i++) {
//        pathMonitorsUp[i]->SaveRecordedPacketsCompact(resultsPath + "/path" + to_string(i) + "_packets_up.csv");
        pathMonitorsDown[i]->SaveRecordedPacketsCompact(resultsPath + "/path" + to_string(i) + "_packets_down.csv");
    }
#endif

#if LOSS_MONITOR_FLAG
//    bottleneckPktMonitorUp->DisplayStats();
    bottleneckPktMonitorDown->DisplayStats();
    for(int i = 0; i < nbPaths; i++) {
//        pathMonitorsUpp[i]->DisplayStats();
        pathMonitorsDown[i]->DisplayStats();
    }
#endif


#if POLICING_FLAG
    bottleneckClass1Monitor.PrintLossRatios();
    bottleneckClass1Monitor.SaveLossRatiosToCSV(resultsPath + "/bottleneck_c1_loss_ratios.csv");
    bottleneckClass1Monitor.SaveAllStatsToCSV(resultsPath + "/bottleneck_c1_stats.csv");

    bottleneckClass2Monitor.PrintLossRatios();
    bottleneckClass2Monitor.SaveLossRatiosToCSV(resultsPath + "/bottleneck_c2_loss_ratios.csv");
    bottleneckClass2Monitor.SaveAllStatsToCSV(resultsPath + "/bottleneck_c2_stats.csv");
#endif
//
//    queueOutfile.close();
//    enqueueOutfile.close();

//    cout << sent_pkts << " packets are sent and " << received_pkts << " packets are received" << endl;

    return 0;
}

