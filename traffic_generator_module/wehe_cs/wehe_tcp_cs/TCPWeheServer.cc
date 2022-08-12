//
// Created by nal on 25.04.21.
//

#include "TCPWeheServer.h"

#include <utility>

TCPWeheServer::TCPWeheServer(string appTag, const Ptr<Node>& server, InetSocketAddress serverAddress) :
        _appTag(std::move(appTag)), _server(server), _serverAddress(serverAddress){
    _sendEvent = EventId ();
}

void TCPWeheServer::LoadTrace(vector<WeheTraceItem> &traceItems) {
    uint32_t preBytesRx = 0;
    for(const auto& item : traceItems) {
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

void TCPWeheServer::StartApplication() {

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

    // to enable pacing for the measurement traffic
    Ptr<TcpSocketBase> tcpSocket = _socket->GetObject<TcpSocketBase>();
    tcpSocket->SetPacingStatus(true);

    // This is added to avoid socket overflow
    _socket->SetSendCallback (MakeCallback (&TCPWeheServer::ResumeResponse, this));

    // part for monitoring the congestion window
    if (_enableCwndMonitor) {
        string outputFolder = _resultsFolder + "/cong_algo_info_" + _appTag + "/sender/";
        _cwndMonitor = new CwndMonitor(_socket, outputFolder);
    }

    _startTime = Simulator::Now();
}

void TCPWeheServer::StopApplication() {
    if (appStopped) { return; }
    appStopped = true;

    // make sure all objects are freed
    _socket->Dispose();
    _lSocket->Dispose();
    Simulator::Cancel (_sendEvent);

    // part for monitoring the congestion window
    if (_cwndMonitor) {
        _cwndMonitor->SaveCwndChanges();
        _cwndMonitor->SaveRtoChanges();
        _cwndMonitor->SaveRttChanges();
        _cwndMonitor->SaveCongStateChanges();
    }

    // save recorded receive events
    ofstream outfile;
    outfile.open(_resultsFolder + "/server_" + _appTag + "_bytes_rx.csv");
    for (auto& event: _rxEvents) { outfile << event.bytesRx << ", " << event.rxTime << endl; }
    outfile.close();
}

bool TCPWeheServer::Send(const WeheTraceItem& item) {
    SeqTsHeader seqTs;
    seqTs.SetSeq (item.frameNb);
    uint32_t segmentSize = item.payloadSize > (8 + 4) ? item.payloadSize - (8 + 4) : item.payloadSize; // 8+4 : the size of the seqTs header
    Ptr<Packet> p = Create<Packet>(segmentSize);
    if(item.payloadSize > (8 + 4)) { p->AddHeader(seqTs); }

    if ((_socket->Send (p)) < 0) {
//        cout << "Error while sending " << payloadSize << " bytes to " << _socket->GetErrno()
//             << " at time " << ns3::Now() << endl;
        return false;
    }

    if(item.payloadSize == 0) { StopApplication(); }
    return true;
}

void TCPWeheServer::Recv(Ptr<Socket> socket) {
    Ptr<Packet> packet = socket->Recv();
    if(packet->GetSize() == 0) {return;}

    _rxEvents.push_back({packet->GetSize(), (Simulator::Now()).GetSeconds()});
    CheckForNextResponse(packet->GetSize());
}

void TCPWeheServer::CheckForNextResponse(uint32_t nbBytesRx) {
    _nbBytesRx += nbBytesRx;

    // TCP case: I received all the previous required bytes
    if(_nbBytesRx >= _traceItems[_traceItemIdx].preBytesRx) {
        sendingResponse = true;
        _nbBytesRx = 0;
        _sendEvent = Simulator::Schedule(Seconds(0.0), &TCPWeheServer::ScheduleNextResponse, this);
    }

}

void TCPWeheServer::ScheduleNextResponse() {
    if(!sendingResponse) { return; }
    if(_traceItemIdx >= _traceItems.size()) {return;}

    do {
        WeheTraceItem item = _traceItems[_traceItemIdx];

        // comment this if the server should have no time pacing and should send as the network conditions allow
        ns3::Time relativeTime = Simulator::Now() - _startTime;
        if (item.timestamp > relativeTime) {
            _sendEvent = Simulator::Schedule(item.timestamp - relativeTime, &TCPWeheServer::ScheduleNextResponse, this);
            return;
        }

        if(!Send(item)) { return; }

        _traceItemIdx++;
    } while(_traceItemIdx < _traceItems.size() && _traceItems[_traceItemIdx].preBytesRx == 0);
    sendingResponse = false;
}

// This is just a wrap-up function
void TCPWeheServer::ResumeResponse(Ptr<Socket> localSocket, uint32_t txSpace) {
    _sendEvent = Simulator::Schedule(Seconds(0.0), &TCPWeheServer::ScheduleNextResponse, this);
}

void TCPWeheServer::SetDscp(int tos) { }

