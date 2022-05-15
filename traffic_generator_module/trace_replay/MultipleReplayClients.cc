//
// Created by nal on 08.02.21.
//

#include "ns3/applications-module.h"

#include "../../helper_classes/HelperMethods.h"
#include <random>

#include "TraceReplayClientHelper.h"
#include "TraceReplayServerHelper.h"
#include "MultipleReplayClients.h"

uint32_t MultipleReplayClients::SOCKET_COUNT = 0;

MultipleReplayClients::MultipleReplayClients(Ptr<Node> client, Ptr<Node> server) : _client(client), _server(server) {}

void MultipleReplayClients::RunAllTraces(const string& tracesPath, uint32_t nbTCPFlows, uint32_t nbUDPFlows, uint8_t tos) {
    for(uint32_t i = 0; i < nbUDPFlows; i++) {
        string tracePath = tracesPath + "/UDP/trace_" + to_string(i) + ".csv";
        RunSingleTrace(tracePath, "ns3::UdpSocketFactory", tos);
    }
    for(uint32_t i = 0; i < nbTCPFlows; i++) {
        string tracePath = tracesPath + "/TCP/trace_" + to_string(i) + ".csv";
        RunSingleTrace(tracePath, "ns3::TcpSocketFactory", tos);
    }
}

void MultipleReplayClients::RunAllTraces(const string& tracesPath, uint8_t tos) {
    uint32_t nbTCPFlows = HelperMethods::GetSubDirCount(tracesPath + "/TCP");
    uint32_t nbUDPFlows = HelperMethods::GetSubDirCount(tracesPath + "/UDP");
    this->RunAllTraces(tracesPath, nbTCPFlows, nbUDPFlows, tos);
}

void MultipleReplayClients::RunSpecificTraces(const vector<string>& tcpTracesPath, const vector<string>& udpTracesPath, uint8_t tos) {
    for(const string& tracePath: udpTracesPath) {
        RunSingleTrace(tracePath, "ns3::UdpSocketFactory", tos);
    }
    for(const string& tracePath: tcpTracesPath) {
        RunSingleTrace(tracePath, "ns3::TcpSocketFactory", tos);
    }
}

void MultipleReplayClients::RunTracesWithRandomThrottledTCPFlows(const string& tracesPath, double throttledProb, uint8_t thottledTos) {
    uint32_t nbTCPFlows = HelperMethods::GetSubDirCount(tracesPath + "/TCP");
    vector<string> tcpTracesPathNeutral, tcpTracesPathThrottled;

    random_device rd;
    mt19937 mt(rd());
    uniform_real_distribution<double> dist(0.0, 1.0);

    uint32_t countTCP4 = 0;
    for(uint32_t i = 0; i < nbTCPFlows; i++) {
        string tracePath = tracesPath + "/TCP/trace_" + to_string(i) + ".csv";
        if (dist(mt) < throttledProb) {
            countTCP4++;
            tcpTracesPathThrottled.push_back(tracePath);
        }
        else {
            tcpTracesPathNeutral.push_back(tracePath);
        };
    }
    this->RunSpecificTraces(tcpTracesPathNeutral, {tracesPath + "/UDP/trace_0.csv"}, 0);
    this->RunSpecificTraces(tcpTracesPathThrottled, {}, thottledTos);
}

void MultipleReplayClients::RunTracesWithRandomThrottledUDPFlows(const string& tracesPath, double throttledProb, uint8_t thottledTos) {
    uint32_t nbTCPFlows = HelperMethods::GetSubDirCount(tracesPath + "/TCP");
    vector<string> tcpTracesPathNeutral, udpTracesPathThrottled;

    random_device rd;
    mt19937 mt(rd());
    uniform_real_distribution<double> dist(0.0, 1.0);

    uint32_t countTCP4 = 0;
    for(uint32_t i = 0; i < nbTCPFlows; i++) {
        string tracePath = tracesPath + "/TCP/trace_" + to_string(i) + ".csv";
        if (dist(mt) < throttledProb) {
            countTCP4++;
            udpTracesPathThrottled.push_back(tracePath);
        }
        else {
            tcpTracesPathNeutral.push_back(tracePath);
        };
    }
    this->RunSpecificTraces(tcpTracesPathNeutral, {tracesPath + "/UDP/trace_0.csv"}, 0);
    this->RunSpecificTraces({},udpTracesPathThrottled,  thottledTos);
}

void MultipleReplayClients::RunSingleTrace(string tracePath, string protocol, uint8_t tos = 0) {
    uint32_t traceId = ++SOCKET_COUNT;

    Ipv4Address serverAddress = _server->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
    int sinkPort = 4000 + traceId;

    // create sink at server
    InetSocketAddress sinkAddressServer = InetSocketAddress(serverAddress, sinkPort);
    TraceReplayServerHelper replayHelperServer(sinkAddressServer);
    replayHelperServer.SetAttribute("Protocol", StringValue(protocol));
    ApplicationContainer replayAppServer = replayHelperServer.Install(_server);
    replayAppServer.Start(Simulator::Now());

    // for tos
    sinkAddressServer.SetTos(tos);

    // create the replay application at client
    TraceReplayClientHelper replayHelperClient(sinkAddressServer);
    replayHelperClient.SetAttribute("Protocol", StringValue(protocol));
    replayHelperClient.SetAttribute("TraceFile", StringValue(tracePath));
    ApplicationContainer replayAppClient = replayHelperClient.Install(_client);
    replayAppClient.Start(Simulator::Now());
}
