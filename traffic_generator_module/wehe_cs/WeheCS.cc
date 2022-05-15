//
// Created by nal on 07.02.21.
//

#include "WeheCS.h"

#include <utility>
#include "WeheClient.h"
#include "WeheServer.h"

uint32_t WeheCS::APPS_COUNT = 0;


WeheCS::WeheCS(Ptr<Node> client, Ptr<Node> server, string protocol) {
    _appId = ++APPS_COUNT;
    _clientNode = client;
    _serverNode = server;
    _protocol = std::move(protocol);
    SetPort(3000 + _appId);
}

void WeheCS::SetTos(int tos) {
    _trafficTos = tos;
}

void WeheCS::SetResultsFolder(string resultsFolder) {
    _resultsFolder = std::move(resultsFolder);
}

void WeheCS::EnableCwndMonitor() {
    _enableCwndMonitor = true;
}

void WeheCS::LoadTrace(const string& traceFile) {
    ifstream traceInput(traceFile);
    string line;
    while(getline(traceInput, line)) {
        vector<string> pkt_attributes = HelperMethods::SplitStr(line, ',');

        uint32_t frameNb = stoi(pkt_attributes[0]);
        ns3::Time timestamp = Seconds(stod(pkt_attributes[1]));
        uint32_t payload_size = stoi(pkt_attributes[2]);
        AppSide appSide = pkt_attributes[3] == "client" ? CLIENT : SERVER;

        if(appSide == CLIENT) {
            _traceItems.push_back({frameNb, timestamp, payload_size, appSide, 0});
        }
        else if(appSide == SERVER) {
            _traceItems.push_back({frameNb, timestamp, payload_size, appSide, 0});
        }
    }
    traceInput.close();
}

void WeheCS::StartApplication(ns3::Time startTime) {
    Ipv4Address address = _serverNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
    int port = GetPort();
    InetSocketAddress serverSocketAddress = InetSocketAddress(address, port);
    serverSocketAddress.SetTos(_trafficTos);

    if(_protocol == "ns3::TcpSocketFactory") {
        _serverApp = new TCPWeheServer(_appId, _serverNode, serverSocketAddress);
    }
    else if(_protocol == "ns3::UdpSocketFactory"){
        _serverApp = new UDPWeheServer(_appId, _serverNode, serverSocketAddress);
    }
    _serverApp->LoadTrace(_traceItems);
    _serverApp->SetResultsFolder(_resultsFolder);
    _serverApp->SetTos(_trafficTos);
    if(_enableCwndMonitor) { _serverApp->EnableCwndMonitor(); }
    _serverApp->StartApplication();


    if(_protocol == "ns3::TcpSocketFactory") {
        _clientApp = new TCPWeheClient(_appId, _clientNode, serverSocketAddress);
    }
    else if(_protocol == "ns3::UdpSocketFactory"){
        _clientApp = new UDPWeheClient(_appId, _clientNode, serverSocketAddress);
    }
    _clientApp->LoadTrace(_traceItems);
    _clientApp->SetResultsFolder(_resultsFolder);
    _clientApp->SetTos(_trafficTos);
    Simulator::Schedule(startTime, &WeheClient::StartApplication, _clientApp);
}

void WeheCS::StopApplication(ns3::Time endTime) {
    Simulator::Schedule(endTime, &WeheClient::StopApplication, _clientApp);
    Simulator::Schedule(endTime, &WeheServer::StopApplication, _serverApp);
}

uint16_t WeheCS::GetPort() {
    return _serverPort;
}

void WeheCS::SetPort(uint16_t port) {
    _serverPort = port;
}

WeheCS* WeheCS::CreateWeheCS(Ptr<Node> client, Ptr<Node> server, const string &trace, bool isTCP, uint8_t tos, const string &resultsPath) {
    WeheCS* weheCS = new WeheCS(client, server, ((isTCP == 1) ? "ns3::TcpSocketFactory" : "ns3::UdpSocketFactory"));
    weheCS->SetTos(tos);
    weheCS->SetResultsFolder(resultsPath);
    weheCS->LoadTrace(trace);
    if(isTCP == 1) weheCS->EnableCwndMonitor();
    return weheCS;
}




