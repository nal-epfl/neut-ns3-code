//
// Created by nal on 19.04.21.
// Network topology
//
//                            +___ d0
//      s0 ___+__ r0 --- r1 __+___ d1
//                            + ...
//                            +___ dn-1
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

#include "../traffic_differentiation_module/TokenBucket.h"

#include "../traffic_generator_module/wehe_cs/WeheCS.h"
#include "../traffic_generator_module/trace_replay/MultipleReplayClients.h"

using namespace ns3;
using namespace std;
using namespace std::chrono;

NS_LOG_COMPONENT_DEFINE("NeutTestWehe");

#define PCAP_FLAG 0
#define PACKET_MONITOR_FLAG 1

/*****************************************************************************/
// This function is to handle when ns3 fails to save current work
int nbWeheApp = 4;
string resultsPath;
PacketMonitor* bottleneckPktMonitorDown;
vector<PacketMonitor*> pathPktsMonitorsDown;
vector<CwndMonitor*> cwndMonitors;
void CleanTerminate () {
    cerr << "terminate handler called\n";
    bottleneckPktMonitorDown->SaveRecordedPacketsCompact(resultsPath + "/bottleneck_packets_down.csv");
    for(int i = 0; i < nbWeheApp; i++) {
        pathPktsMonitorsDown[i]->SaveRecordedPacketsCompact(resultsPath + "/path" + to_string(i) + "_packets_down.csv");
    }

    for(int i = 0; i < nbWeheApp; i++) {
        cwndMonitors[i]->SaveCwndChanges();
        cwndMonitors[i]->SaveCongStateChanges();
        cwndMonitors[i]->SaveRtoChanges();
        cwndMonitors[i]->SaveRttChanges();
    }
}

/*****************************************************************************/
int run_neut_test_wehe(int argc, char **argv) {
    // temp to be fixed later
//    int nbWeheApp = 4;
    //////////////////////////
    auto start = high_resolution_clock::now();

    LogComponentEnable ("NeutTestWehe", LOG_LEVEL_INFO);
//    LogComponentEnable ("TcpSocketBase", LOG_LEVEL_ALL);
//    LogComponentEnable ("TcpRxBuffer", LOG_LEVEL_ALL);
    NS_LOG_INFO("N Wehe applications with CAIDA background is running");

    /*** Variables read from arguments ***/
    float duration = 120.;
    string resultsFolderName;
    string btlkLinkRate = "180Mbps";
    int weheProtocol = 0;
    string tcpProtocol = "ns3::TcpCubic";
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
    cout << "the common link transmission rate = " << btlkLinkRate << endl;


    /*** Simulation Parameters ***/
    Time simStartTime = Seconds(0.);
    Time warmupTime = Seconds(10);
    Time simEndTime = warmupTime + Seconds(duration) + Seconds(5);
    resultsPath = (string)(getenv("PWD")) + "/results" + resultsFolderName;
//    string resultsPath = (string)(getenv("PWD")) + "/results" + resultsFolderName;
    string dataPath = (string)(getenv("PWD")) + "/data";


    /*** Topology Parameters ***/
    int nbDsts = nbWeheApp + 1;

    /*** Traffic Parameters ***/
    string weheAppProtocol = (weheProtocol == 1) ? "ns3::TcpSocketFactory" : "ns3::UdpSocketFactory";
    cout << weheAppProtocol << endl;

    uint32_t rcvBufSize = 2e9;
    uint32_t sndBufSize = 2e9;
    uint32_t mss = 1228;
    uint32_t mtu = 1500; //1600;
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue (tcpProtocol));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (mss));
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (rcvBufSize));
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue (sndBufSize));
    Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue (1));
//    Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue (true));
    Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue (MilliSeconds(200)));
    Config::SetDefault("ns3::TcpL4Protocol::RecoveryType", TypeIdValue (TcpClassicRecovery::GetTypeId()));


    /*** Create the trident topology ***/
    NS_LOG_INFO("Create Topology.");
    NodeContainer routers; routers.Create(2);
    NodeContainer dstNodes; dstNodes.Create(nbDsts);

    InternetStackHelper internetStackHelper;
    internetStackHelper.Install(routers);
    internetStackHelper.Install(dstNodes);

    string defaultDataRate = "10Gbps";
    string defaultDataRate2 = "1000Mbps";
    string defaultLinkDelay = "5ms";
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(defaultDataRate));
    p2p.SetChannelAttribute("Delay", StringValue(defaultLinkDelay));
    p2p.SetDeviceAttribute("Mtu", UintegerValue(mtu));
    p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1p"));

    NetDeviceContainer channels_r1_dsts[nbDsts];
    string dataRates[] = {defaultDataRate2, defaultDataRate2, defaultDataRate2, defaultDataRate2, defaultDataRate};
    if(scenario == 2) {
        dataRates[0] = "350Mbps"; dataRates[1] = "350Mbps";
        dataRates[2] = "350Mbps"; dataRates[3] = "350Mbps";
    }
    else if(scenario == 3) {
        dataRates[0] = "270Mbps"; dataRates[1] = "210Mbps";
        dataRates[2] = "220Mbps"; dataRates[3] = "210Mbps";
    }
    else if(scenario == 4) {
        dataRates[0] = "300Mbps"; dataRates[1] = "240Mbps";
        dataRates[2] = "240Mbps"; dataRates[3] = "240Mbps";
    }
    else if(scenario == 5) { // not needed
        dataRates[0] = "350Mbps"; dataRates[1] = "350Mbps";
        dataRates[2] = "240Mbps"; dataRates[3] = "240Mbps";
    }
    else if(scenario == 6) {
//        dataRates[0] = "350Mbps"; dataRates[1] = "350Mbps";
        dataRates[2] = "220Mbps"; dataRates[3] = "210Mbps";
    }
    else if(scenario == 7) {
        dataRates[0] = "255Mbps"; dataRates[1] = "210Mbps";
        dataRates[2] = "220Mbps"; dataRates[3] = "210Mbps";
    }
    string delays[] = {defaultLinkDelay, defaultLinkDelay, defaultLinkDelay, defaultLinkDelay, defaultLinkDelay};
    for(int i = 0; i < nbDsts; i++) {
        // to create scenarios on the non-common links
        p2p.SetDeviceAttribute("DataRate", StringValue(dataRates[i]));
        p2p.SetChannelAttribute("Delay", StringValue(delays[i]));
        channels_r1_dsts[i] = p2p.Install( dstNodes.Get(i), routers.Get(1));
        // set the queues to fifo queueing discipline
        TrafficControlHelper tch;
        string queueSize = to_string(int(0.025 * (DataRate(dataRates[i]).GetBitRate() * 0.125))) + "B"; // RTT * link_rate
        tch.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", StringValue(queueSize));
        tch.Install(channels_r1_dsts[i]);
    }


    // Parameters for the bottleneck channel
    PointToPointHelper p2pRouters;
    p2pRouters.SetDeviceAttribute("DataRate", StringValue(btlkLinkRate));
    p2pRouters.SetChannelAttribute("Delay", StringValue(defaultLinkDelay));
    p2pRouters.SetDeviceAttribute("Mtu", UintegerValue(mtu));
    p2pRouters.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1p"));
    NetDeviceContainer channel_r0_r1 = p2pRouters.Install(routers.Get(0), routers.Get(1));


    // Modify the traffic control layer module of the router 0 net device to implement policing
    TrafficControlHelper tch;
    string queueSize = to_string(int(0.025 * (DataRate(btlkLinkRate).GetBitRate() * 0.125))) + "B"; // RTT * link_rate
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
    Ipv4InterfaceContainer addresses_dsts_r0[nbDsts];
    Ipv4Address dstAddresses[nbDsts];
    for(int i = 0; i < nbDsts; i++) {
        ipv4.SetBase(("10.1." + to_string(++nbSubnets) + ".0").c_str(), "255.255.255.0");
        addresses_dsts_r0[i] = ipv4.Assign(channels_r1_dsts[i]);
        dstAddresses[i] = addresses_dsts_r0[i].GetAddress(0);
    }


    Ipv4GlobalRoutingHelper::PopulateRoutingTables();


    uint16_t dstPorts[nbWeheApp];
    /*** Create Wehe Traffic ***/
    int trafficClass[] = {0, 0, 4, 8};
    for(int i = 0; i < nbWeheApp; i++) {
        WeheCS* weheCS = new WeheCS(routers.Get(0), dstNodes.Get(i), weheAppProtocol);
        weheCS->SetTos(trafficClass[i]);
        weheCS->SetResultsFolder(resultsPath);
        weheCS->LoadTrace(dataPath + "/weheCS_trace");
        if(weheProtocol == 1) weheCS->EnableCwndMonitor();
        weheCS->StartApplication(warmupTime);
        weheCS->StopApplication(simEndTime);
        dstPorts[i] = weheCS->GetPort();
    }

    /*** Create Background Traffic ***/
//    string CaidaTracesPath = dataPath + "/chicago_2010_back_traffic_6min_byprefix";
//
//    uint32_t nbTcpTraces = 303;
////    // Scenario new1 Traffic on all paths
//    MultipleReplayClients* back1 = new MultipleReplayClients(dstNodes.Get(0), routers.Get(0));
//    MultipleReplayClients* back2 = new MultipleReplayClients(dstNodes.Get(1), routers.Get(0));
//    MultipleReplayClients* back3 = new MultipleReplayClients(dstNodes.Get(2), routers.Get(0));
//    MultipleReplayClients* back4 = new MultipleReplayClients(dstNodes.Get(3), routers.Get(0));
//    MultipleReplayClients* back5 = new MultipleReplayClients(dstNodes.Get(nbDsts-1), routers.Get(0));
//    vector<string> p1TcpTraces, p2TcpTraces, p3TcpTraces, p4TcpTraces, p5TcpTraces;
//    for(uint32_t i = 0; i < nbTcpTraces; i++) {
//        string tracePath = CaidaTracesPath + "/TCP/trace_" + to_string(i) + ".csv";
//
//        if(i >= 0 && i < 12) { p1TcpTraces.push_back(tracePath); }
//        else if(i >= 12 && i < 17) { p2TcpTraces.push_back(tracePath); }
//        else if(i >= 16 && i < 50) { p3TcpTraces.push_back(tracePath); }
//        else if(i >= 50 && i < 200) { p4TcpTraces.push_back(tracePath); }
//        else { p5TcpTraces.push_back(tracePath); }
//    }
//    if(scenario == 0) {
//        back5->RunSpecificTraces(p1TcpTraces, {}, 0);
//        back5->RunSpecificTraces(p2TcpTraces, {}, 0);
//        back5->RunSpecificTraces(p3TcpTraces, {}, 0);
//        back5->RunSpecificTraces(p4TcpTraces, {}, 0);
//    }
//    else if(scenario == 1 || scenario == 2 || scenario == 3) {
//        back1->RunSpecificTraces(p1TcpTraces, {}, 0);
//        back2->RunSpecificTraces(p2TcpTraces, {}, 0);
//        back3->RunSpecificTraces(p3TcpTraces, {}, 0);
//        back4->RunSpecificTraces(p4TcpTraces, {}, 0);
//    }
//    back5->RunSpecificTraces(p5TcpTraces, {CaidaTracesPath + "/UDP/trace_0.csv"}, 0);

    // scenario 2 where we send a CAIDA traffic on each link
    // Scenario new1 Traffic on all paths
    MultipleReplayClients* back1 = new MultipleReplayClients(dstNodes.Get(0), routers.Get(0));
    back1->RunAllTraces(dataPath + "/chicago_2010_links_back_traffic_0/chicago_2010_back_traffic_1min_link1", 899, 1);
    MultipleReplayClients* back2 = new MultipleReplayClients(dstNodes.Get(1), routers.Get(0));
    back2->RunAllTraces(dataPath + "/chicago_2010_links_back_traffic_0/chicago_2010_back_traffic_1min_link2", 773, 1);
    MultipleReplayClients* back3 = new MultipleReplayClients(dstNodes.Get(2), routers.Get(0));
    back3->RunAllTraces(dataPath + "/chicago_2010_links_back_traffic_0/chicago_2010_back_traffic_1min_link3", 945, 1);
    MultipleReplayClients* back4 = new MultipleReplayClients(dstNodes.Get(3), routers.Get(0));
    back4->RunAllTraces(dataPath + "/chicago_2010_links_back_traffic_0/chicago_2010_back_traffic_1min_link4", 781, 1);


#if PCAP_FLAG /*** Record Pcap files for channels ***/
    AsciiTraceHelper ascii;
//    p2p.EnableAsciiAll(ascii.CreateFileStream(resultsPath + "/tracing.tr"));
    p2pRouters.EnablePcapAll(resultsPath + "/pcap_files");
//    p2pRouters.EnablePcap(resultsPath + "/router-link", 0, 1);
#endif

    uint32_t routersIds[2] = {routers.Get(0)->GetId(), routers.Get(1)->GetId()};
    uint32_t dstIds[nbDsts]; for(int i = 0; i < nbDsts; i++) { dstIds[i] = dstNodes.Get(i)->GetId(); }

#if PACKET_MONITOR_FLAG
    bottleneckPktMonitorDown = new PacketMonitor(warmupTime, Seconds(duration), routersIds[1], routersIds[0], "bottleneckDown");
//    PacketMonitor* bottleneckPktMonitorDown = new PacketMonitor(warmupTime, Seconds(duration), routersIds[1], routersIds[0], "bottleneckDown");
    for(int i = 0; i < nbWeheApp; i++)
        bottleneckPktMonitorDown->AddAppKey(dstAddresses[i], addresses_r0_r1.GetAddress(0), dstPorts[i]);

//    vector<PacketMonitor*> pathPktsMonitorsDown;
    for(int i = 0; i < nbWeheApp; i++) {
        PacketMonitor* pathMonitor = new PacketMonitor(warmupTime, Seconds(duration), dstIds[i], routersIds[0],  "path" + to_string(i) + "Down");
        pathMonitor->AddAppKey(dstAddresses[i], addresses_r0_r1.GetAddress(0), dstPorts[i]);
        pathPktsMonitorsDown.push_back(pathMonitor);
    }

//    vector<PacketMonitor*> pathPktsMonitorsUp;
//    for(int i = 0; i < nbWeheApp; i++) {
//        PacketMonitor* pathMonitor = new PacketMonitor(warmupTime, Seconds(duration), routersIds[0], dstIds[i],  "path" + to_string(i) + "Up");
//        pathMonitor->AddAppKey(addresses_r0_r1.GetAddress(0), dstAddresses[i], 49153+i);
//        pathPktsMonitorsUp.push_back(pathMonitor);
//    }
#endif

    std::set_terminate(CleanTerminate);


    /*** Run simulation ***/
    NS_LOG_INFO("Run Simulation.");
    Simulator::Stop(simEndTime);
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");


#if PACKET_MONITOR_FLAG
    bottleneckPktMonitorDown->SaveRecordedPacketsCompact(resultsPath + "/bottleneck_packets_down.csv");
    for(int i = 0; i < nbWeheApp; i++) {
        pathPktsMonitorsDown[i]->SaveRecordedPacketsCompact(resultsPath + "/path" + to_string(i) + "_packets_down.csv");
//        pathPktsMonitorsUp[i]->SaveRecordedPacketsCompact(resultsPath + "/path" + to_string(i) + "_packets_up.csv");
//        pathPktsMonitorsDown[i]->SaveRecordedPacketsToCSV(resultsPath + "/path" + to_string(i) + "_packets_down.csv");
    }
#endif

    auto stop = high_resolution_clock::now();
    cout << "Total execution time = " << duration_cast<microseconds>(stop - start).count() << " microsecond" << endl;

    return 0;
}



