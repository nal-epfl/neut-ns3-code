//
// Created by nal on 17.02.21.
// Network topology
//
//      s0 ___+               +___ d0
//      s1 ___+__ r0 --- r1 __+___ d1
//        ... +               + ...
//    sn-1 ___+               +___ dn-1
//
// - All links are P2P
// - TCP flows along s0->d0, s1->d1, sn-2->dn-2
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

#include "../traffic_differentiation_module/CbQueueDisc.h"

#include "../traffic_generator_module/wehe_cs/WeheCS.h"
#include "../traffic_generator_module/trace_replay/MultipleReplayClients.h"

using namespace ns3;
using namespace std;
using namespace std::chrono;

NS_LOG_COMPONENT_DEFINE("NTCPwithCAIDA");

#define PCAP_FLAG 0
#define PACKET_MONITOR_FLAG 1
#define CWND_MONITOR 1

/*****************************************************************************/
// This function is to handle when ns3 fails to save current work
//int nbWeheApp = 4;
//string resultsPath;
//PacketMonitor* bottleneckPktMonitorDown;
//vector<PacketMonitor*> pathPktsMonitorsDown;
//vector<CwndMonitor*> cwndMonitors;
//void CleanTerminate () {
//    cerr << "terminate handler called\n";
//    bottleneckPktMonitorDown->SaveRecordedPacketsFor1Path(resultsPath + "/bottleneck_packets_down.csv");
//    for(int i = 0; i < nbWeheApp; i++) {
//        pathPktsMonitorsDown[i]->SaveRecordedPacketsFor1Path(resultsPath + "/path" + to_string(i) + "_packets_down.csv");
//    }
//
//    for(int i = 0; i < nbWeheApp; i++) {
//        cwndMonitors[i]->SaveCwndChanges();
//        cwndMonitors[i]->SaveCongStateChanges();
//        cwndMonitors[i]->SaveRtoChanges();
//        cwndMonitors[i]->SaveRttChanges();
//    }
//}

/*****************************************************************************/
int run_TcpCwnd_test(int argc, char **argv) {
    // temp to be fixed later
    int nbTcpFlows = 4;
    //////////////////////////
    auto start = high_resolution_clock::now();

    LogComponentEnable ("NTCPwithCAIDA", LOG_LEVEL_INFO);
//    LogComponentEnable ("PacketMetadata", LOG_LEVEL_ALL);
    NS_LOG_INFO("N TCP flows with CAIDA background is running");

    /*** Variables read from arguments ***/
    float duration = 120.;
    string resultsFolderName;
    string btlkLinkRate = "180Mbps";
    int weheProtocol = 0;
    string tcpProtocol = "ns3::TcpNewReno";
    int isNeutral = 1;
    double policingRate = 4;
    double burstLength = 0.1; // in sec
    string nonCommonRate = "10Gbps";
    uint32_t scenario = 0; // 0-> all back on path 5, 1-> distributed back
    string tcpRate = "2Mbps";

    CommandLine cmd;
    cmd.AddValue("duration", "the duration of the the simulation", duration);
    cmd.AddValue("resultsFolderName", "the name of the folder to save results to", resultsFolderName);
    cmd.AddValue("linkRate", "the rate of the bottleneck link", btlkLinkRate);
    cmd.AddValue("weheAppProtocol", "the protocol used by the wehe app: 0=udp, 1=tcp", weheProtocol);
    cmd.AddValue("TCPProtocol", "the tcp congestion control protocol", tcpProtocol);
    cmd.AddValue("neutral", "to enable neutral bottleneck behaviour (0 means enable policing)", isNeutral);
    cmd.AddValue("policingRate", "rate used in case of policing (in Mbps) ", policingRate);
    cmd.AddValue("policingBurstLength", "duration of burst (in sec)", burstLength);
    cmd.AddValue("nonCommonRate", "the rate of the non-common link", nonCommonRate);
    cmd.AddValue("scenario", "0 means perFlow_policer, 2 means path3_policer, 3 means different_policers", scenario);
    cmd.AddValue("tcpRate", "the rate of the tcp measurement flows", tcpRate);
    cmd.Parse(argc, argv);

    srand(ns3::RngSeedManager::GetSeed());
    cout << "the seed for this run is " << ns3::RngSeedManager::GetSeed() << endl;


    /*** Simulation Parameters ***/
    Time simStartTime = Seconds(0.);
    Time warmupTime = Seconds(15);
    Time simEndTime = warmupTime + Seconds(duration) + Seconds(5);
    string resultsPath = (string)(getenv("PWD")) + "/results" + resultsFolderName;
//    resultsPath = (string)(getenv("PWD")) + "/results" + resultsFolderName;
    string dataPath = (string)(getenv("PWD")) + "/data";


    /*** Topology Parameters ***/
    int nbDsts = nbTcpFlows + 1;
//    int nbSrcs = 1;

    /*** Traffic Parameters ***/
    string weheAppProtocol = (weheProtocol == 1) ? "ns3::TcpSocketFactory" : "ns3::UdpSocketFactory";
    cout << weheAppProtocol << endl;

    uint32_t rcvBufSize = 2e9;
    uint32_t mss = 1442;
    uint32_t mtu = 1550; //1600;
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue (tcpProtocol));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (mss));
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (rcvBufSize));
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue (rcvBufSize));
//    Config::SetDefault("ns3::TcpSocketBase::ReTxThreshold", UintegerValue (2e9));
//  Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(Seconds(0)));


    /*** Create the dumbbell topology ***/
    NS_LOG_INFO("Create Topology.");
    NodeContainer routers; routers.Create(2);
    NodeContainer dstNodes; dstNodes.Create(nbDsts);

    InternetStackHelper internetStackHelper;
    internetStackHelper.Install(routers);
    internetStackHelper.Install(dstNodes);


    string defaultDataRate = "10Gbps";
    string defaultDataRate2 = "100Mbps";
    string defaultLinkDelay = "10ms";
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(defaultDataRate));
    p2p.SetChannelAttribute("Delay", StringValue("10ms"));
    p2p.SetDeviceAttribute("Mtu", UintegerValue(mtu));
    p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1p"));

    NetDeviceContainer channels_r1_dsts[nbDsts];
    string dataRates[] = {defaultDataRate2, defaultDataRate2, defaultDataRate2, defaultDataRate2, defaultDataRate};
    if(scenario == 3) {
        dataRates[2] = "42Mbps"; dataRates[3] = "58Mbps";
    }
    else if(scenario == 4) {
        dataRates[0] = "50Mbps"; dataRates[1] = "44Mbps";
        dataRates[2] = "44Mbps"; dataRates[3] = "60Mbps";
    }
//    string dataRates[] = {"50Mbps", "44Mbps", "44Mbps", "60Mbps", defaultDataRate};
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
    p2pRouters.SetDeviceAttribute("Mtu", UintegerValue(mtu));
    p2pRouters.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1p"));
    NetDeviceContainer channel_r0_r1 = p2pRouters.Install(routers.Get(0), routers.Get(1));


    // Modify the traffic control layer module of the router 0 net device to implement policing
    TrafficControlHelper tch;
    // should change here the RTT to 0.035
    string queueSize = to_string(int(0.035 * (DataRate(btlkLinkRate).GetBitRate() * 0.125))) + "B"; // RTT * link_rate
    if(isNeutral == 0) {
        cout << "we have policing with rate " << policingRate << "Mbps";

        int burst = floor(policingRate * burstLength * 125000);// in byte
        cout << ", and burst duration " << burstLength << " sec, giving burst = " << burst << " Byte." << endl;

        uint16_t handle = tch.SetRootQueueDisc("ns3::CbQueueDisc", "MaxSize", StringValue(queueSize),
                                               "TosMap", TosMapValue(TosMap{0, 4, 8}));

        TrafficControlHelper::ClassIdList cid = tch.AddQueueDiscClasses (handle, 3, "ns3::QueueDiscClass");
        tch.AddChildQueueDisc (handle, cid[0], "ns3::FifoQueueDisc", "MaxSize", StringValue(queueSize));
        tch.AddChildQueueDisc (handle, cid[1], "ns3::TbfQueueDiscChild",
                               "Burst", UintegerValue (burst),
                               "Mtu", UintegerValue (mtu),
                               "Rate", DataRateValue (DataRate (to_string(policingRate) + "Mbps")),
                               "PeakRate", DataRateValue (DataRate ("0bps")));
        tch.AddChildQueueDisc (handle, cid[2], "ns3::TbfQueueDiscChild",
                               "Burst", UintegerValue (burst),
                               "Mtu", UintegerValue (mtu),
                               "Rate", DataRateValue (DataRate (to_string(policingRate) + "Mbps")),
                               "PeakRate", DataRateValue (DataRate ("0bps")));
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
    Ipv4InterfaceContainer addresses_dsts_r0[nbDsts];
    Ipv4Address dstAddresses[nbDsts];
    for(int i = 0; i < nbDsts; i++) {
        ipv4.SetBase(("10.1." + to_string(++nbSubnets) + ".0").c_str(), "255.255.255.0");
        addresses_dsts_r0[i] = ipv4.Assign(channels_r1_dsts[i]);
        dstAddresses[i] = addresses_dsts_r0[i].GetAddress(0);
    }


    Ipv4GlobalRoutingHelper::PopulateRoutingTables();


    uint16_t sinkPorts[nbTcpFlows];
    Ptr<Socket> appSockets[nbTcpFlows];
    /*** Create Wehe Traffic ***/
    int trafficClass[] = {0, 0, 4, 8};
    for(int i = 0; i < nbTcpFlows; i++) {
        // Create a packet sink on r0 to receive packets
        uint16_t port = 3000 + (i+1);
        sinkPorts[i] = port;
        PacketSinkHelper sink(weheAppProtocol, InetSocketAddress(Ipv4Address::GetAny (), port));
        ApplicationContainer sinkApp = sink.Install(routers.Get(0));
        sinkApp.Start(warmupTime);
        sinkApp.Stop(simEndTime);



        InetSocketAddress sinkAddress = InetSocketAddress(addresses_r0_r1.GetAddress(0), port);
        sinkAddress.SetTos(trafficClass[i]);
        OnOffHelper client(weheAppProtocol, sinkAddress);
        client.SetAttribute ("PacketSize", UintegerValue (1228));
        client.SetAttribute ("DataRate", DataRateValue (DataRate(tcpRate)));
        client.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
        client.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=" + to_string(duration) +"]"));
        client.SetAttribute("EnableSeqTsSizeHeader", BooleanValue(true));
        ApplicationContainer clientApp = client.Install(dstNodes.Get(i));
        clientApp.Start(warmupTime);
        clientApp.Stop(simEndTime);

    }

    /*** Create Background Traffic ***/
    string CaidaTracesPath = dataPath + "/chicago_2010_back_traffic_6min_byprefix";

    uint32_t nbTcpTraces = 303;
    // Scenario new1 Traffic on all paths
    MultipleReplayClients* back1 = new MultipleReplayClients(dstNodes.Get(0), routers.Get(0));
    MultipleReplayClients* back2 = new MultipleReplayClients(dstNodes.Get(1), routers.Get(0));
    MultipleReplayClients* back3 = new MultipleReplayClients(dstNodes.Get(2), routers.Get(0));
    MultipleReplayClients* back4 = new MultipleReplayClients(dstNodes.Get(3), routers.Get(0));
    MultipleReplayClients* back5 = new MultipleReplayClients(dstNodes.Get(nbDsts-1), routers.Get(0));

//    back->RunAllTraces(CaidaTracesPath, 1300, 1);
    vector<string> p1TcpTraces, p2TcpTraces, p3TcpTraces, p4TcpTraces, p5TcpTraces;
    for(uint32_t i = 0; i < nbTcpTraces; i++) {
        string tracePath = CaidaTracesPath + "/TCP/trace_" + to_string(i) + ".csv";

        if(i >= 0 && i < 12) { p1TcpTraces.push_back(tracePath); }
        else if(i >= 12 && i < 17) { p2TcpTraces.push_back(tracePath); }
        else if(i >= 16 && i < 50) { p3TcpTraces.push_back(tracePath); }
        else if(i >= 50 && i < 200) { p4TcpTraces.push_back(tracePath); }
        else { p5TcpTraces.push_back(tracePath); }
    }
    if(scenario == 0) {
        back5->RunSpecificTraces(p1TcpTraces, {}, 0);
        back5->RunSpecificTraces(p2TcpTraces, {}, 0);
        back5->RunSpecificTraces(p3TcpTraces, {}, 0);
        back5->RunSpecificTraces(p4TcpTraces, {}, 0);
    }
    else if(scenario == 1 || scenario == 3 || scenario == 4) {
        back1->RunSpecificTraces(p1TcpTraces, {}, 0);
        back2->RunSpecificTraces(p2TcpTraces, {}, 0);
        back3->RunSpecificTraces(p3TcpTraces, {}, 0);
        back4->RunSpecificTraces(p4TcpTraces, {}, 0);
    }
    back5->RunSpecificTraces(p5TcpTraces, {CaidaTracesPath + "/UDP/trace_0.csv"}, 0);


//    // Scenario 1: All on path 5
//    MultipleReplayClients* back = new MultipleReplayClients(dstNodes.Get(nbDsts-1), routers.Get(0));
////    back->RunAllTraces(CaidaTracesPath, 1300, 1);
//    vector<string> TcpTracePathsTos0, TcpTracePathsTos4;
//    for(uint32_t i = 0; i < nbTcpTraces; i++) {
//        string tracePath = CaidaTracesPath + "/TCP/trace_" + to_string(i) + ".csv";
//        if((i > 0 && i < 200) || (i >= 1000 && i < 1230)) { TcpTracePathsTos4.push_back(tracePath); }
//        else { TcpTracePathsTos0.push_back(tracePath); }
//    }
//    back->RunSpecificTraces(TcpTracePathsTos0, {CaidaTracesPath + "/UDP/trace_0.csv"}, 0);
//    back->RunSpecificTraces(TcpTracePathsTos4, {}, 4);

    // Scenario 2: On different paths

//    vector<string> p3TcpTracePaths, p4TcpTracePaths, p5TcpTracePaths;
//    for(uint32_t i = 0; i < nbTcpTraces; i++) {
//        string tracePath = CaidaTracesPath + "/TCP/trace_" + to_string(i) + ".csv";
//        if(i >= 10 && i < 40) { p3TcpTracePaths.push_back(tracePath); }
//        else if(i >= 40 && i < 150) { p4TcpTracePaths.push_back(tracePath); }
//        else { p5TcpTracePaths.push_back(tracePath); }
//    }
//
//    MultipleReplayClients* backP3 = new MultipleReplayClients(dstNodes.Get(nbDsts-3), routers.Get(0));
//    backP3->RunSpecificTraces(p3TcpTracePaths, {}, 0);
//
//    MultipleReplayClients* backP4 = new MultipleReplayClients(dstNodes.Get(nbDsts-2), routers.Get(0));
//    backP4->RunSpecificTraces(p4TcpTracePaths, {}, 0);
//
//    MultipleReplayClients* backP5 = new MultipleReplayClients(dstNodes.Get(nbDsts-1), routers.Get(0));
//    backP5->RunSpecificTraces(p5TcpTracePaths, {CaidaTracesPath + "/UDP/trace_0.csv"}, 0);


#if PCAP_FLAG /*** Record Pcap files for channels ***/
    AsciiTraceHelper ascii;
    p2p.EnableAsciiAll(ascii.CreateFileStream(resultsPath + "/tracing.tr"));
    p2pRouters.EnablePcapAll(resultsPath + "/router-link");
#endif

    uint32_t routersIds[2] = {routers.Get(0)->GetId(), routers.Get(1)->GetId()};
    uint32_t dstIds[nbDsts]; for(int i = 0; i < nbDsts; i++) { dstIds[i] = dstNodes.Get(i)->GetId(); }

#if PACKET_MONITOR_FLAG
    PacketMonitor* bottleneckPktMonitorDown = new PacketMonitor(warmupTime, Seconds(duration), routersIds[1], routersIds[0], "bottleneckDown");
//    bottleneckPktMonitorDown = new PacketMonitor(warmupTime, Seconds(duration), routersIds[1], routersIds[0], "bottleneckDown");
    for(int i = 0; i < nbTcpFlows; i++)
        bottleneckPktMonitorDown->AddAppKey(dstAddresses[i], addresses_r0_r1.GetAddress(0), 0, sinkPorts[i]);

    vector<PacketMonitor*> pathPktsMonitorsDown;
    for(int i = 0; i < nbTcpFlows; i++) {
        PacketMonitor* pathMonitor = new PacketMonitor(warmupTime, Seconds(duration), dstIds[i], routersIds[0],  "path" + to_string(i) + "Down");
        pathMonitor->AddAppKey(dstAddresses[i], addresses_r0_r1.GetAddress(0), 0, sinkPorts[i]);
        pathPktsMonitorsDown.push_back(pathMonitor);
    }
#endif

#if CWND_MONITOR
    vector<CwndMonitor*> cwndMonitors;
    for(int i = 0; i < nbTcpFlows; i++) {
        string outputFolder = resultsPath + "/cong_algo_info_" + to_string(i+1) + "/server/";
        cwndMonitors.push_back(new CwndMonitor(dstIds[i], 0, warmupTime+Seconds(0.1), outputFolder));
    }
#endif

//    std::set_terminate(CleanTerminate);


    /*** Run simulation ***/
    NS_LOG_INFO("Run Simulation.");
    Simulator::Stop(simEndTime);
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");


#if PACKET_MONITOR_FLAG
    bottleneckPktMonitorDown->SaveRecordedPacketsCompact(resultsPath + "/bottleneck_packets_down.csv");
    for(int i = 0; i < nbTcpFlows; i++) {
        pathPktsMonitorsDown[i]->SaveRecordedPacketsCompact(resultsPath + "/path" + to_string(i) + "_packets_down.csv");
    }
#endif

#if CWND_MONITOR
    for(int i = 0; i < nbTcpFlows; i++) {
        cwndMonitors[i]->SaveCwndChanges();
        cwndMonitors[i]->SaveCongStateChanges();
        cwndMonitors[i]->SaveRtoChanges();
        cwndMonitors[i]->SaveRttChanges();
    }
#endif

    auto stop = high_resolution_clock::now();
    cout << "Total execution time = " << duration_cast<microseconds>(stop - start).count() << " microsecond" << endl;

    return 0;
}


