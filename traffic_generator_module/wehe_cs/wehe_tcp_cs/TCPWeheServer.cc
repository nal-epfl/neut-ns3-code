//
// Created by nal on 25.04.21.
//

#include "TCPWeheServer.h"

TCPWeheServer::TCPWeheServer(uint32_t appId, Ptr<Node> server, InetSocketAddress serverAddress) :
        _appId(appId), _server(server), _serverAddress(serverAddress){
}

void TCPWeheServer::LoadTrace(vector<WeheTraceItem> &traceItems) {
    uint32_t preBytesRx = 0;
    for(auto item : traceItems) {
        if(item.appSide == SERVER) {
            _traceItems.push_back({item.frameNb, item.timestamp, item.payloadSize, SERVER, preBytesRx});
            preBytesRx = 0;
        }
        else if(item.appSide == CLIENT) {
            preBytesRx += item.payloadSize;
        }
    }
    WeheTraceItem lastAppItem = traceItems.back();
    _traceItems.push_back({lastAppItem.frameNb + 1, lastAppItem.timestamp, 0, SERVER, preBytesRx});

    _traceItemIdx = 0; _nbBytesRx = 0;
}

void TCPWeheServer::SetResultsFolder(string resultsFolder) {
    _resultsFolder = resultsFolder;
}

void TCPWeheServer::EnableCwndMonitor() {
    _enableCwndMonitor = true;
}

void TCPWeheServer::StartApplication(void) {

    if(!_lSocket) {
        _lSocket = Socket::CreateSocket (_server, TypeId::LookupByName ("ns3::TcpSocketFactory" ));
        if (_lSocket->Bind(_serverAddress) == -1) {
            NS_FATAL_ERROR ("Failed to bind socket");
        }
        _lSocket->Listen ();

        _lSocket->SetAcceptCallback( MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
                    MakeCallback(&TCPWeheServer::HandleTCPAccept, this));
    }

}

void TCPWeheServer::HandleTCPAccept (Ptr<Socket> socket, const Address& from) {
    cout << "Received a connection from " << from << endl;
    SetupConnection(socket);
    CheckForNextResponse(0);
}

void TCPWeheServer::SetupConnection(Ptr<Socket> socket) {
    _socket = socket;
    _socket->SetAttribute("RcvBufSize", UintegerValue(131072));
    _socket->SetAttribute("SndBufSize", UintegerValue(131072));
    _socket->SetRecvCallback (MakeCallback (&TCPWeheServer::Recv, this));
    _socket->SetSendCallback (MakeCallback (&TCPWeheServer::ResumeResponse, this));

    // part for monitoring the congestion window
    if (_enableCwndMonitor) {
        string outputFolder = _resultsFolder + "/cong_algo_info_" + to_string(_appId) + "/server/";
        _cwndMonitor = new CwndMonitor(_socket, outputFolder);
    }

    _startTime = Simulator::Now();
}

void TCPWeheServer::StopApplication() {
    // part for monitoring the congestion window
    if (_enableCwndMonitor) {
        _cwndMonitor->SaveCwndChanges();
        _cwndMonitor->SaveRtoChanges();
        _cwndMonitor->SaveRttChanges();
        _cwndMonitor->SaveCongStateChanges();
    }

    ofstream outfile;
    outfile.open(_resultsFolder + "/server_app" + to_string(_appId) + "_bytes_rx.csv");
    for (auto& event: _rxEvents) { outfile << event.bytesRx << ", " << event.rxTime << endl; }
    outfile.close();
}

bool TCPWeheServer::Send(uint32_t payloadSize) {
    SeqTsHeader seqTs;
    seqTs.SetSeq (_traceItemIdx);
    uint32_t segmentSize = payloadSize > (8+4) ? payloadSize - (8+4) : payloadSize; // 8+4 : the size of the seqTs header
    Ptr<Packet> p = Create<Packet>(segmentSize);
    if(payloadSize > (8+4)) { p->AddHeader(seqTs); }

    if ((_socket->Send (p)) < 0) {
//        cout << "Error while sending " << payloadSize << " bytes to " << _socket->GetErrno()
//             << " at time " << ns3::Now() << endl;
        return false;
    }

    if(payloadSize == 0) {StopApplication();}
    return true;
}

void TCPWeheServer::Recv(Ptr<Socket> socket) {
    Ptr<Packet> packet = socket->Recv();
    if(packet->GetSize() == 0) {return;}

    _rxEvents.push_back({packet->GetSize(), (Simulator::Now()-_startTime).GetSeconds()});
    CheckForNextResponse(packet->GetSize());
}

void TCPWeheServer::CheckForNextResponse(uint32_t nbBytesRx) {
    _nbBytesRx += nbBytesRx;

    // TCP case: I received all the previous required bytes
    if(_nbBytesRx >= _traceItems[_traceItemIdx].preBytesRx) {
        sendingResponse = true;
        _nbBytesRx = 0;
        ScheduleNextResponse();
    }

}

void TCPWeheServer::ScheduleNextResponse() {
    if(!sendingResponse) { return; }
    if(_traceItemIdx >= _traceItems.size()) {return;}

    do {
        WeheTraceItem item = _traceItems[_traceItemIdx];

        // The server should have no time pacing and should send as the network conditions allow
        ns3::Time relativeTime = Simulator::Now() - _startTime;
        if (item.timestamp > relativeTime) {
            Simulator::Schedule(item.timestamp - relativeTime, &TCPWeheServer::ScheduleNextResponse, this);
            return;
        }

        bool isSend = Send(item.payloadSize);
        if(!isSend) { return; }

        _traceItemIdx++;
    } while(_traceItemIdx < _traceItems.size() && _traceItems[_traceItemIdx].preBytesRx == 0);
    sendingResponse = false;
}

// This is just a wrap-up function
void TCPWeheServer::ResumeResponse(Ptr<Socket> localSocket, uint32_t txSpace) {
    ScheduleNextResponse();
}

void TCPWeheServer::SetTos(int tos) { }

