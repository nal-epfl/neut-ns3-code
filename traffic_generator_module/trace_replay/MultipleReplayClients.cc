//
// Created by nal on 08.02.21.
//

#include "ns3/applications-module.h"

#include "TraceReplayClientHelper.h"
#include "TraceReplayServerHelper.h"
#include "MultipleReplayClients.h"

uint32_t MultipleReplayClients::SOCKET_COUNT = 0;

MultipleReplayClients::MultipleReplayClients(Ptr<Node> client, Ptr<Node> server) : _client(client), _server(server) {}

void MultipleReplayClients::RunAllTraces(string tracesPath, uint32_t nbTCPFlows, uint32_t nbUDPFlows) {
    for(uint32_t i = 0; i < nbUDPFlows; i++) {
        string tracePath = tracesPath + "/UDP/trace_" + to_string(i) + ".csv";
        RunSingleTrace(tracePath, "ns3::UdpSocketFactory", 0);
    }
    for(uint32_t i = 0; i < nbTCPFlows; i++) {
        string tracePath = tracesPath + "/TCP/trace_" + to_string(i) + ".csv";
//        uint8_t tos = (i >=800 && i < 1100) ? 4 : 0;
        RunSingleTrace(tracePath, "ns3::TcpSocketFactory", 0);
    }
}

void MultipleReplayClients::RunSpecificTraces(vector<string> tcpTracesPath, vector<string> udpTracesPath, uint8_t tos) {
    for(const string& tracePath: udpTracesPath) {
        RunSingleTrace(tracePath, "ns3::UdpSocketFactory", 0);
    }
    for(const string& tracePath: tcpTracesPath) {
        RunSingleTrace(tracePath, "ns3::TcpSocketFactory", tos);
    }
}

void MultipleReplayClients::RunSingleTrace(string tracePath, string protocol, uint8_t tos = 0) {
    uint32_t traceId = ++SOCKET_COUNT;

    Ipv4Address serverAddress = _server->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
    int sinkPort = 4000 + traceId;

    // create sink at server
//    InetSocketAddress sinkAddressServer = InetSocketAddress(serverAddress, sinkPort);
//    PacketSinkHelper sinkHelperServer(protocol, sinkAddressServer);
//    ApplicationContainer sinkAppServer = sinkHelperServer.Install(_server);
//    sinkAppServer.Start(Simulator::Now());
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
