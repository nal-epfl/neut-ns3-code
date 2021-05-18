//
// Created by nal on 08.09.20.
//
// Network topology
//
//    n0 -- n1 -- n2
//
// - UDP flow with Poisson Pareto Bursts form n0 to n2

#include <iostream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"

#include "../monitors_module/PacketMonitor.h"
#include "../monitors_module/LossMonitor.h"

#include "../traffic_generator_module/ppb/PPBClientHelper.h"
#include "../traffic_generator_module/poisson_bursts/PBClientHelper.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("1PathPPBUdp");

#define APP_LOG_FLAG 0
#define PCAP_FLAG 0
#define PACKET_MONITOR_FLAG 1
#define LOSS_MONITOR_FLAG 0


/*****************************************************************************/

int run_1path_ppb_udp(int argc, char **argv) {

    LogComponentEnable("1PathPPBUdp", LOG_LEVEL_INFO);
    NS_LOG_INFO("Single path with PPB UDP flows is running");
//    LogComponentEnable("PointToPointNetDevice", LOG_LEVEL_ALL);

#if APP_LOG_FLAG
    LogComponentEnable("UdpClient", LOG_LEVEL_INFO);
    LogComponentEnable("UdpServer", LOG_LEVEL_INFO);
#endif

//    LogComponentEnable("PPBPApplication", LOG_LEVEL_INFO);

    /*** Variables read from arguments ***/
    float duration = 120.;
    string resultsFolderName = "";
    CommandLine cmd;
    cmd.AddValue("duration", "the duration of the the simulation", duration);
    cmd.AddValue("resultsFolderName", " the name of the folder to save results to", resultsFolderName);
    cmd.Parse(argc, argv);


    /*** Specifications for the topology and experiment ***/
    string linkPropagationDelay = "10ms";
    int mtu = 2000;
    string linkRate = "550Mbps";
    string queueSize = to_string(int(0.035 * (DataRate(linkRate).GetBitRate() * 0.125))) + "B"; // RTT * link_rate
    cout << "queue capacity: " << queueSize << endl;

    uint16_t sinkPort = 4000;
    Time startTime = Seconds(0.);
    Time transientPeriod = Seconds(200);
    Time endTime = transientPeriod + Seconds(duration) + Seconds(10);
    string resultsPath = (string) (getenv("PWD")) + "/results" + resultsFolderName;


    /*** Traffic Parameters ***/
    string trafficProtocol = "ns3::UdpSocketFactory";
    string burstIntensity("0.05Mb/s");
    double meanBurstArrivals = 1540; // bursts per sec
    double meanBurstTimeLength = 2; //sec
    double hurstParameter = 0.8;


    /*** Create the dumbbell topology ***/
    NS_LOG_INFO("Create Topology.");
    NodeContainer nodeContainer;
    nodeContainer.Create(3);

    InternetStackHelper internetStackHelper;
    internetStackHelper.Install(nodeContainer);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(linkRate));
    p2p.SetDeviceAttribute("Mtu", UintegerValue(mtu));
    p2p.SetChannelAttribute("Delay", StringValue(linkPropagationDelay));
    p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1p"));
    NetDeviceContainer channel_0_1 = p2p.Install(nodeContainer.Get(0), nodeContainer.Get(1));
    NetDeviceContainer channel_1_2 = p2p.Install(nodeContainer.Get(1), nodeContainer.Get(2));


    // Modify the traffic control layer module of the node 0 net device to implement policing
    TrafficControlHelper tch;
    tch.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", StringValue(queueSize));
    tch.Install(channel_0_1);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer addresses_0_1 = ipv4.Assign(channel_0_1);
    ipv4.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer addresses_1_2 = ipv4.Assign(channel_1_2);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();


    /*** Create PPB UDP Flows ***/
    NS_LOG_INFO("Create Traffic.");

    for(int i = 0; i < 1; i++) {

        Address sinkAddress = Address(
                InetSocketAddress(nodeContainer.Get(2)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), sinkPort+i));
        Ptr<Socket> sinkSocket = Socket::CreateSocket(nodeContainer.Get(2), UdpSocketFactory::GetTypeId());
        sinkSocket->Bind(sinkAddress);

        PPBClientHelper ppbpHelper(trafficProtocol, sinkAddress);
        ppbpHelper.SetAttribute("BurstIntensity", DataRateValue(burstIntensity));
        ppbpHelper.SetAttribute("MeanBurstArrivals", DoubleValue(meanBurstArrivals));
        ppbpHelper.SetAttribute("MeanBurstTimeLength", DoubleValue(meanBurstTimeLength));
        ppbpHelper.SetAttribute("H", DoubleValue(hurstParameter));
        ApplicationContainer ppbpApp = ppbpHelper.Install(nodeContainer.Get(0));
        ppbpApp.Start(startTime);
        ppbpApp.Stop(endTime);
    }

    Address sinkAddress = Address(
            InetSocketAddress(nodeContainer.Get(2)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), sinkPort+10));
    Ptr<Socket> sinkSocket = Socket::CreateSocket(nodeContainer.Get(2), UdpSocketFactory::GetTypeId());
    sinkSocket->Bind(sinkAddress);

//    PBClientHelper burstClientHelper(trafficProtocol, sinkAddress);
//    burstClientHelper.SetAttribute("BurstIntensity", DataRateValue(DataRate("120Mbps")));
//    burstClientHelper.SetAttribute("MeanBurstArrivals", DoubleValue(1));
//    burstClientHelper.SetAttribute("MeanBurstTimeLength", DoubleValue(0.03));
//    ApplicationContainer burstApp = burstClientHelper.Install(nodeContainer.Get(0));
//    burstApp.Start(startTime);
//    burstApp.Stop(endTime);


    PPBClientHelper burstClientHelper(trafficProtocol, sinkAddress);
    burstClientHelper.SetAttribute("BurstIntensity", DataRateValue(DataRate("150Mbps")));
    burstClientHelper.SetAttribute("MeanBurstArrivals", DoubleValue(0.8));
    burstClientHelper.SetAttribute("MeanBurstTimeLength", DoubleValue(0.03));
    burstClientHelper.SetAttribute("H", DoubleValue(hurstParameter));
    ApplicationContainer burstApp = burstClientHelper.Install(nodeContainer.Get(0));
    burstApp.Start(startTime);
    burstApp.Stop(endTime);


#if PCAP_FLAG /*** Record Pcap files for channels ***/
    AsciiTraceHelper ascii;
    p2p.EnableAsciiAll(ascii.CreateFileStream(resultsPath + "dumbbell-topo.tr"));
    p2p.EnablePcapAll(resultsPath + "dumbbell-topo", false);
#endif

#if PACKET_MONITOR_FLAG /*** Monitor the paths ***/
    PacketMonitor* pathPktMonitor = new PacketMonitor(transientPeriod, Seconds(duration), nodeContainer.Get(0)->GetId(), nodeContainer.Get(2)->GetId(), "path");
    pathPktMonitor->AddAppKey(nodeContainer.Get(0)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(),
                              nodeContainer.Get(2)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 0);
#endif

#if LOSS_MONITOR_FLAG /*** Monitor the paths ***/
    LossMonitor* pathLossMonitor = new LossMonitor(transientPeriod, Seconds(duration), nodeContainer.Get(0)->GetId(), nodeContainer.Get(2)->GetId(), "path");
    pathLossMonitor->AddDestination(nodeContainer.Get(2)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal());
#endif


    /*** Run simulation ***/
    NS_LOG_INFO("Run Simulation.");
    Simulator::Stop(endTime);
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");


#if PACKET_MONITOR_FLAG
    pathPktMonitor->SaveRecordedPacketsCompact(resultsPath + "/path_packets_up.csv");
#endif

#if LOSS_MONITOR_FLAG
    pathLossMonitor->DisplayStats();
#endif

    return 0;
}
