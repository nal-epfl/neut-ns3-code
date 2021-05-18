//
// Created by nal on 05.09.20.
//

#include "TraceReplayClientServer.h"
#include "TraceReplayClientHelper.h"
#include "ns3/applications-module.h"

int TraceReplayClientServer::_appCount = 0;

TraceReplayClientServer::TraceReplayClientServer() {
    _appCount++; // increase every time a traceReplayClientSever is created
}

void TraceReplayClientServer::SetClient(const Ptr<Node> &client) {
    _client = client;
}

void TraceReplayClientServer::SetServer(const Ptr<Node> &server) {
    _server = server;
}

void TraceReplayClientServer::SetTracesPath(string clientTracesPath, string serverTracePath) {
    _clientTracePath = clientTracesPath;
    _serverTracePath = serverTracePath;
}

void TraceReplayClientServer::SetProtocol(const string &protocol) {
    _protocol = protocol;
}

void TraceReplayClientServer::SetTrafficsTos(int clientTrafficTos, int serverTrafficTos) {
    _clientTrafficTos = clientTrafficTos;
    _serverTrafficTos = serverTrafficTos;
}

void TraceReplayClientServer::EnableCwndMonitor(string saveFilename) {
    _enableCwndMonitor = true;
    _cwndSaveFilename = saveFilename;
}

void TraceReplayClientServer::Setup(ns3::Time startTime, ns3::Time endTime) {
    // helper variables
    Ipv4Address serverAddress = _server->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
    Ipv4Address clientAddress = _client->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
    int sinkPort = 3000 + _appCount;

    // create sink at server
    InetSocketAddress sinkAddressServer = InetSocketAddress(serverAddress, sinkPort);
    sinkAddressServer.SetTos(_clientTrafficTos); // This is for traffic differentiation
    PacketSinkHelper sinkHelperServer(_protocol, sinkAddressServer);
    ApplicationContainer sinkAppServer = sinkHelperServer.Install(_server);
    sinkAppServer.Start(startTime);
    sinkAppServer.Stop(endTime);

    // create the replay application at client
    TraceReplayClientHelper replayHelperClient(sinkAddressServer);
    replayHelperClient.SetAttribute("Protocol", StringValue(_protocol));
    replayHelperClient.SetAttribute("TraceFile", StringValue(_clientTracePath));
    if(_enableCwndMonitor) {
        replayHelperClient.SetAttribute("EnableCwndMonitor", BooleanValue(_enableCwndMonitor));
        replayHelperClient.SetAttribute("CongAlgoFolder", StringValue(_cwndSaveFilename));
    }
    ApplicationContainer replayAppClient = replayHelperClient.Install(_client);
    replayAppClient.Start(startTime);
    replayAppClient.Stop(endTime);

//    // create sink at client
//    InetSocketAddress sinkAddressClient = InetSocketAddress(clientAddress, sinkPort);
//    sinkAddressClient.SetTos(_serverTrafficTos); // This is for traffic differentiation
//    PacketSinkHelper sinkHelperClient(_protocol, sinkAddressClient);
//    ApplicationContainer sinkAppClient = sinkHelperClient.Install(_client);
//    sinkAppClient.Start(startTime);
//    sinkAppClient.Stop(endTime);
//
//    // create the replay application at server
//    TraceReplayClientHelper replayHelperServer(sinkAddressClient);
//    replayHelperServer.SetAttribute("Protocol", StringValue(_protocol));
//    replayHelperServer.SetAttribute("TraceFile", StringValue(_serverTracePath));
//    ApplicationContainer replayAppServer = replayHelperServer.Install(_server);
//    replayAppServer.Start(startTime);
//    replayAppServer.Stop(endTime);

}

