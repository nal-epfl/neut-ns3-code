//
// Created by nal on 25.04.21.
//

#include "TCPWeheClient.h"



TCPWeheClient::TCPWeheClient(uint32_t appId, Ptr<Node> &client, InetSocketAddress &serverAddress) :
        _appId(appId), _client(client), _serverAddress(serverAddress) {
}

void TCPWeheClient::LoadTrace(vector<WeheTraceItem> &traceItems) {
    uint32_t preBytesRx = 0;
    for(auto item : traceItems) {
        if(item.appSide == CLIENT) {
            _traceItems.push_back({item.frameNb, item.timestamp, item.payloadSize, CLIENT, preBytesRx});
            preBytesRx = 0;
        }
        else if(item.appSide == SERVER) {
            preBytesRx += item.payloadSize;
        }
    }
    WeheTraceItem lastAppItem = traceItems.back();
    _traceItems.push_back({lastAppItem.frameNb + 1, lastAppItem.timestamp, 0, CLIENT, preBytesRx});

    _traceItemIdx = 0; _nbBytesRx = 0;
}

void TCPWeheClient::SetResultsFolder(string resultsFolder) {
    _resultsFolder = resultsFolder;
}

void TCPWeheClient::EnableCwndMonitor() {
    _enableCwndMonitor = true;
}

void TCPWeheClient::StartApplication() {

    if (!_socket) {
        _socket = Socket::CreateSocket (_client, TypeId::LookupByName ("ns3::TcpSocketFactory" ));
        if (_socket->Bind () == -1) {
            NS_FATAL_ERROR ("Failed to bind socket");
        }
        _socket->Connect(_serverAddress);
    }

    _socket->SetAttribute("RcvBufSize", UintegerValue(131072));
    _socket->SetAttribute("SndBufSize", UintegerValue(131072));
    _socket->SetRecvCallback (MakeCallback (&TCPWeheClient::Recv, this));
    _socket->SetAllowBroadcast (true);

    // part for monitoring the congestion window
    if (_enableCwndMonitor) {
        string outputFolder = _resultsFolder + "/cong_algo_info_" + to_string(_appId) + "/client/";
        _cwndMonitor = new CwndMonitor(_socket, outputFolder);
    }

    _startTime = Simulator::Now();

    DecideOnNextSend(0);
}

void TCPWeheClient::StopApplication() {
    _socket->Dispose();

    // part for monitoring the congestion window
    if (_enableCwndMonitor) {
        _cwndMonitor->SaveCwndChanges();
        _cwndMonitor->SaveRtoChanges();
        _cwndMonitor->SaveRttChanges();
        _cwndMonitor->SaveCongStateChanges();
    }

    ofstream outfile;
    outfile.open(_resultsFolder + "/client_app" + to_string(_appId) + "_bytes_rx.csv");
    for (auto& event: _rxEvents) { outfile << event.bytesRx << ", " << event.rxTime << endl; }
    outfile.close();
}

void TCPWeheClient::Send(uint32_t payloadSize) {
    Ptr<Packet> p = Create<Packet> (payloadSize);

    if ((_socket->Send (p)) < 0) {
        cout << "Error while sending " << payloadSize << " bytes to " << _serverAddress
             << " at time " << ns3::Now() << endl;
    }

//    cout << "client sends data " << seqNb << " with payload size " << payloadSize << endl;

    if(payloadSize == 0) {StopApplication();}
}

void TCPWeheClient::Recv(Ptr<Socket> socket) {
    Ptr<Packet> packet = socket->Recv();
    if(packet->GetSize() == 0) {return;}

//    cout << "client recv " << packet->GetSize() << " at time " <<  (Simulator::Now()-_startTime).GetSeconds() << endl;

    _rxEvents.push_back({packet->GetSize(), (Simulator::Now()-_startTime).GetSeconds()});
    DecideOnNextSend(packet->GetSize());
}

void TCPWeheClient::DecideOnNextSend(uint32_t nbBytesRx) {
    _nbBytesRx += nbBytesRx;

    // TCP case: I received all the previous required bytes
    if(_nbBytesRx >= _traceItems[_traceItemIdx].preBytesRx) {
        ScheduleNextSendingEvents();
    }
}

void TCPWeheClient::ScheduleNextSendingEvents() {
    if(_traceItemIdx >= _traceItems.size()) {return;}
    do {
        WeheTraceItem item = _traceItems[_traceItemIdx];

        // keep this time pacing on the client requests side
//        ns3::Time relativeTime = Simulator::Now() - _startTime;
//        ns3::Time remainingTime = (item.timestamp > relativeTime) ? item.timestamp - relativeTime : Seconds(0);
//        Simulator::Schedule(remainingTime, &TCPWeheClient::Send, this, item.payloadSize);

        Send(item.payloadSize);

        _traceItemIdx++;
    } while(_traceItemIdx < _traceItems.size() && _traceItems[_traceItemIdx].preBytesRx == 0);
    _nbBytesRx = 0;
}
