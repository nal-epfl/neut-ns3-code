//
// Created by nal on 25.04.21.
//

#include "UDPWeheClient.h"

#include <utility>

UDPWeheClient::UDPWeheClient(string appTag, Ptr<Node> &client, InetSocketAddress &serverAddress) :
        _appTag(std::move(appTag)), _client(client), _serverAddress(serverAddress){

}

void UDPWeheClient::LoadTrace(vector<WeheTraceItem> &traceItems) {
    for(auto item : traceItems) {
        if(item.appSide == CLIENT) {
            _traceItems.push_back({item.frameNb, item.timestamp, item.payloadSize, CLIENT, 0});
        }
    }
    WeheTraceItem lastAppItem = traceItems.back();
    _traceItems.push_back({lastAppItem.frameNb + 1, lastAppItem.timestamp, 0, CLIENT, 0});

    _traceItemIdx = 0;
}

void UDPWeheClient::SetResultsFolder(string resultsFolder) {
    _resultsFolder = resultsFolder;
}

void UDPWeheClient::SetDscp(int tos) {
    _trafficTos = tos;
}

void UDPWeheClient::EnableCwndMonitor() {}

void UDPWeheClient::StartApplication() {

    if (!_socket) {
        _socket = Socket::CreateSocket (_client, TypeId::LookupByName ("ns3::UdpSocketFactory"));
        if (_socket->Bind () == -1) {
            NS_FATAL_ERROR ("Failed to bind socket");
        }
        _socket->Connect(_serverAddress);
    }

    _socket->SetRecvCallback (MakeCallback (&UDPWeheClient::Recv, this));
    _socket->SetAllowBroadcast (true);

    _startTime = Simulator::Now();

    ScheduleNextSendingEvents();
}

void UDPWeheClient::StopApplication() {
    _socket->Dispose();

    ofstream outfile;
    outfile.open(_resultsFolder + "/client_" + _appTag + "_bytes_rx.csv");
    for (auto& event: _rxEvents) { outfile << event.bytesRx << ", " << event.rxTime << endl; }
    outfile.close();
}

void UDPWeheClient::Send(uint32_t payloadSize, uint32_t seqNb) {
    Ptr<Packet> p = Create<Packet> (payloadSize);

    if ((_socket->Send (p)) < 0) {
        cout << "Error while sending " << payloadSize << " bytes to " << _serverAddress
             << " at time " << ns3::Now() << endl;
    }

    if(payloadSize == 0) {StopApplication();}
}

void UDPWeheClient::Recv(Ptr<Socket> socket) {
    Ptr<Packet> packet = socket->Recv();
    if(packet->GetSize() == 0) {return;}

    _rxEvents.push_back({packet->GetSize(), (Simulator::Now()-_startTime).GetSeconds()});
}

void UDPWeheClient::ScheduleNextSendingEvents() {
    if(_traceItemIdx >= _traceItems.size()) {return;}
    do {
        WeheTraceItem item = _traceItems[_traceItemIdx];

        ns3::Time relativeTime = Simulator::Now() - _startTime;
        ns3::Time remainingTime = (item.timestamp > relativeTime) ? item.timestamp - relativeTime : Seconds(0);
        Simulator::Schedule(remainingTime, &UDPWeheClient::Send, this, item.payloadSize, _traceItemIdx);

        _traceItemIdx++;
    } while(_traceItemIdx < _traceItems.size());
}



