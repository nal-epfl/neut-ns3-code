//
// Created by nal on 25.04.21.
//

#include "UDPWeheServer.h"

#include <utility>


UDPWeheServer::UDPWeheServer(uint32_t appId, Ptr<Node> server, InetSocketAddress serverAddress) :
        _appId(appId), _server(server), _serverAddress(serverAddress) {
}

void UDPWeheServer::LoadTrace(vector<WeheTraceItem> &traceItems) {
    for(auto item : traceItems) {
        if(item.appSide == SERVER) {
            _traceItems.push_back({item.frameNb, item.timestamp, item.payloadSize, SERVER, 0});
        }
    }
    WeheTraceItem lastAppItem = traceItems.back();
    _traceItems.push_back({lastAppItem.frameNb + 1, lastAppItem.timestamp, 0, SERVER, 0});

    _traceItemIdx = 0;
}

void UDPWeheServer::SetResultsFolder(string resultsFolder) {
    _resultsFolder = std::move(resultsFolder);
}

void UDPWeheServer::SetTos(int tos) {
    _trafficTos = tos;
}

void UDPWeheServer::EnableCwndMonitor() {}

void UDPWeheServer::StartApplication(void) {

    if(!_lSocket) {
        _lSocket = Socket::CreateSocket (_server, TypeId::LookupByName ("ns3::UdpSocketFactory"));
        if (_lSocket->Bind(_serverAddress) == -1) {
            NS_FATAL_ERROR ("Failed to bind socket");
        }
        _lSocket->Listen ();

        _lSocket->SetRecvCallback (MakeCallback (&UDPWeheServer::HandleUDPAccept, this));
    }

}

void UDPWeheServer::HandleUDPAccept(Ptr<Socket> socket) {
    Address from; socket->RecvFrom(from); socket->Connect(from);
    socket->SetIpTos(_trafficTos);
    SetupConnection(socket);
    ScheduleNextSendingEvents();
}

void UDPWeheServer::SetupConnection(Ptr<Socket> socket) {
    _socket = socket;
    _socket->SetRecvCallback (MakeCallback (&UDPWeheServer::Recv, this));
    _startTime = Simulator::Now();
}


void UDPWeheServer::StopApplication() {
    ofstream outfile;
    outfile.open(_resultsFolder + "/server_app" + to_string(_appId) + "_bytes_rx.csv");
    for (auto& event: _rxEvents) { outfile << event.bytesRx << ", " << event.rxTime << endl; }
    outfile.close();
}

void UDPWeheServer::Send(uint32_t payloadSize, uint32_t seqNb) {
    Ptr<Packet> p = Create<Packet> (payloadSize);

    if ((_socket->Send (p)) < 0) {
        cout << "Error while sending " << payloadSize << " bytes to " << _socket->GetErrno()
             << " at time " << ns3::Now() << endl;
    }

    if(payloadSize == 0) {StopApplication();}
}

void UDPWeheServer::Recv(Ptr<Socket> socket) {
    Ptr<Packet> packet = socket->Recv();
    if(packet->GetSize() == 0) {return;}

    _rxEvents.push_back({packet->GetSize(), (Simulator::Now()-_startTime).GetSeconds()});
}

void UDPWeheServer::ScheduleNextSendingEvents() {
    if(_traceItemIdx >= _traceItems.size()) {return;}
    do {
        WeheTraceItem item = _traceItems[_traceItemIdx];

        ns3::Time relativeTime = Simulator::Now() - _startTime;
        ns3::Time remainingTime = (item.timestamp > relativeTime) ? item.timestamp - relativeTime : Seconds(0);
        Simulator::Schedule(remainingTime, &UDPWeheServer::Send, this, item.payloadSize, _traceItemIdx);

        _traceItemIdx++;
    } while(_traceItemIdx < _traceItems.size());
}

