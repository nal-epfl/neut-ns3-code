//
// Created by nal on 25.10.21.
//

#include "MeasurReplayClient.h"


NS_LOG_COMPONENT_DEFINE ("MeasurReplayClient");

NS_OBJECT_ENSURE_REGISTERED (MeasurReplayClient);

int MeasurReplayClient::CLIENTS_COUNT = 0;

TypeId MeasurReplayClient::GetTypeId(void) {

    static TypeId tid = TypeId ("ns3::MeasurReplayClient")
            .SetParent<Application> ()
            .SetGroupName("Applications")
            .AddConstructor<MeasurReplayClient> ()
            .AddAttribute ("RemoteAddress",
                           "The destination Address of the outbound packets",
                           AddressValue (),
                           MakeAddressAccessor (&MeasurReplayClient::_peerAddress),
                           MakeAddressChecker ())
            .AddAttribute ("RemotePort", "The destination port of the outbound packets",
                           UintegerValue (100),
                           MakeUintegerAccessor (&MeasurReplayClient::_peerPort),
                           MakeUintegerChecker<uint16_t> ())
            .AddAttribute ("Protocol", "the name of the protocol to use to send traffic by the applications",
                           StringValue ("ns3::UdpSocketFactory"),
                           MakeStringAccessor (&MeasurReplayClient::_protocol),
                           MakeStringChecker())
            .AddAttribute ("TraceFile", "the name of the file to read the traces from",
                           StringValue (""),
                           MakeStringAccessor (&MeasurReplayClient::_traceFilename),
                           MakeStringChecker())
            .AddAttribute ("EnableCwndMonitor", "Enable monitoring the congestion window",
                           BooleanValue(false),
                           MakeBooleanAccessor(&MeasurReplayClient::_enableCwndMonitor),
                           MakeBooleanChecker())
            .AddAttribute ("ResultsFolder", "the folder path to save the congestion algorithm records to",
                           StringValue (""),
                           MakeStringAccessor (&MeasurReplayClient::_resultsFolder),
                           MakeStringChecker())
    ;
    return tid;
}

MeasurReplayClient::MeasurReplayClient() {
    NS_LOG_FUNCTION (this);
    _clientId = ++CLIENTS_COUNT;
    _traceItemIdx = 0;
    _socket = 0;
    _appPaused = false;
    _sendEvent = EventId ();
}

MeasurReplayClient::~MeasurReplayClient() {
    NS_LOG_FUNCTION (this);
}

void MeasurReplayClient::SetRemote(Address ip, uint16_t port) {
    NS_LOG_FUNCTION (this << ip << port);
    _peerAddress = ip;
    _peerPort = port;
}

void MeasurReplayClient::DoDispose(void) {
    NS_LOG_FUNCTION (this);
    Application::DoDispose();
}

void MeasurReplayClient::StartApplication(void) {
    NS_LOG_FUNCTION (this);

    if (_socket == 0)     {
        TypeId tid = TypeId::LookupByName (_protocol);
        _socket = Socket::CreateSocket (GetNode (), tid);
        if (Ipv4Address::IsMatchingType(_peerAddress) == true) {
            if (_socket->Bind () == -1) {
                NS_FATAL_ERROR ("Failed to bind socket");
            }
            _socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(_peerAddress), _peerPort));
        }
        else if (Ipv6Address::IsMatchingType(_peerAddress) == true) {
            if (_socket->Bind6 () == -1) {
                NS_FATAL_ERROR ("Failed to bind socket");
            }
            _socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom(_peerAddress), _peerPort));
        }
        else if (InetSocketAddress::IsMatchingType (_peerAddress) == true) {
            if (_socket->Bind () == -1) {
                NS_FATAL_ERROR ("Failed to bind socket");
            }
            _socket->Connect (_peerAddress);
        }
        else if (Inet6SocketAddress::IsMatchingType (_peerAddress) == true) {
            if (_socket->Bind6 () == -1) {
                NS_FATAL_ERROR ("Failed to bind socket");
            }
            _socket->Connect (_peerAddress);
        }
        else
        {
            NS_ASSERT_MSG (false, "Incompatible address type: " << _peerAddress);
        }
    }

    LoadTrace();
    _startTime = Simulator::Now();

    // adjust socket buffers
//    _socket->SetAttribute("RcvBufSize", UintegerValue(131072));
//    _socket->SetAttribute("SndBufSize", UintegerValue(131072));

    // part to change starts from here
    _socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    _socket->SetAllowBroadcast (true);
    Simulator::Schedule (Seconds (0.0), &MeasurReplayClient::ScheduleNextSend, this);

    // This is added to avoid socket overflow
    _socket->SetSendCallback (MakeCallback (&MeasurReplayClient::ResumeApp, this));

    // part for monitoring the congestion window
    if (_enableCwndMonitor) {
        string outputFolder = _resultsFolder + "/cong_algo_info_" + to_string(_clientId) + "/server/";
        _cwndMonitor = new CwndMonitor(_socket, outputFolder);
    }

}

void MeasurReplayClient::StopApplication(void) {
    NS_LOG_FUNCTION (this);
//    cout << "nb packets sent " << _nbSentPkts << endl;
    Simulator::Cancel (_sendEvent);

    // part for monitoring the congestion window
    if (_enableCwndMonitor) {
        _cwndMonitor->SaveCwndChanges();
        _cwndMonitor->SaveRtoChanges();
        _cwndMonitor->SaveRttChanges();
        _cwndMonitor->SaveCongStateChanges();
    }
}

void MeasurReplayClient::LoadTrace() {
    NS_LOG_FUNCTION(this);

    ifstream traceInput(_traceFilename);
    string line;
    ns3::Time finalTimestamp;
    while(getline(traceInput, line)) {
        vector<string> pkt_attributes = HelperMethods::SplitStr(line, ',');

        uint32_t frameNb = stoi(pkt_attributes[0]);
        ns3::Time timestamp = Seconds(stod(pkt_attributes[1]));
        uint32_t payload_size = stoi(pkt_attributes[2]);

        _traceItems.push_back({frameNb, timestamp, payload_size});

        // to know when the application should stop
        finalTimestamp = timestamp;
    }
    traceInput.close();

    // make the application stop itself based on the trace file data
    Simulator::Schedule(finalTimestamp, &MeasurReplayClient::StopApplication, this);
}

bool MeasurReplayClient::Send(uint32_t payload_size) {
    NS_LOG_FUNCTION(this);
//    NS_ASSERT (_sendEvent.IsExpired());

    Ptr<Packet> p;
    if (payload_size > (8+4)) {
        SeqTsHeader seqTs;
        seqTs.SetSeq(_traceItemIdx);
        p = Create<Packet>(payload_size - (8 + 4)); // 8+4 : the size of the seqTs header
        p->AddHeader(seqTs);
    }
    else {
        p = Create<Packet>(payload_size);
    }

//    cout << "timestamp: " << Simulator::Now().GetSeconds() << ", payload size: " << payload_size << endl;

    std::stringstream peerAddressStringStream;
    if (Ipv4Address::IsMatchingType (_peerAddress)) {
        peerAddressStringStream << Ipv4Address::ConvertFrom (_peerAddress);
    }
    else if (Ipv6Address::IsMatchingType (_peerAddress)) {
        peerAddressStringStream << Ipv6Address::ConvertFrom (_peerAddress);
    }

    if ((_socket->Send (p)) >= 0) {
        NS_LOG_INFO ("TraceDelay TX " << payload_size << " bytes to "
                                      << peerAddressStringStream.str () << " Uid: "
                                      << p->GetUid () << " Time: "
                                      << (Simulator::Now ()).GetSeconds ());
        return true;
    }
    NS_LOG_INFO ("Error while sending " << payload_size << " bytes to "
                                            << peerAddressStringStream.str ());
    return false;
}

void MeasurReplayClient::ScheduleNextSend() {
    NS_LOG_FUNCTION(this);

    if(_traceItemIdx >= _traceItems.size()) {return;}

    TraceReplayItem currItem = _traceItems[_traceItemIdx];
    if (Send(currItem.payloadSize)) {

        _traceItemIdx++;
        if(_traceItemIdx >= _traceItems.size()) {return;}

        TraceReplayItem nextItem = _traceItems[_traceItemIdx];
        ns3::Time relativeTime = Simulator::Now() - _startTime;
        ns3::Time remainingTime = (nextItem.timestamp > relativeTime) ? nextItem.timestamp - relativeTime : Seconds(0);

        Simulator::Schedule(remainingTime, &MeasurReplayClient::ScheduleNextSend, this);
    }
    else {
        _appPaused = true;
    }
}


// This is just a wrap-up function
void MeasurReplayClient::ResumeApp(Ptr<Socket> localSocket, uint32_t txSpace) {
    if(_appPaused) {
        _appPaused = false;
        ScheduleNextSend();
    }
}
