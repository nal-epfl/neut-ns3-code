//
// Created by nal on 31.08.20.
//

#include "TraceReplayClient.h"

namespace trace_replay_helper {

    vector<string> split (const string &s, char delim) {
        vector<string> result;
        stringstream ss (s);
        string item;

        while (getline (ss, item, delim)) {
            result.push_back (item);
        }

        return result;
    }

    template <class T>
    string VectorToString(vector<T> vector, string separator) {
        if (vector.empty()) return "";

        stringstream ss;
        ss << vector[0];
        auto aggregate = [&ss, &separator](const T &s) { ss << separator << s; };
        for_each(vector.begin() + 1, vector.end(), aggregate);

        return ss.str();
    }

}


NS_LOG_COMPONENT_DEFINE ("TraceReplayClient");

NS_OBJECT_ENSURE_REGISTERED (TraceReplayClient);

int TraceReplayClient::CLIENTS_COUNT = 0;

TypeId TraceReplayClient::GetTypeId(void) {

    static TypeId tid = TypeId ("ns3::TraceReplayClient")
            .SetParent<Application> ()
            .SetGroupName("Applications")
            .AddConstructor<TraceReplayClient> ()
            .AddAttribute ("RemoteAddress",
                           "The destination Address of the outbound packets",
                           AddressValue (),
                           MakeAddressAccessor (&TraceReplayClient::_peerAddress),
                           MakeAddressChecker ())
            .AddAttribute ("RemotePort", "The destination port of the outbound packets",
                           UintegerValue (100),
                           MakeUintegerAccessor (&TraceReplayClient::_peerPort),
                           MakeUintegerChecker<uint16_t> ())
            .AddAttribute ("Protocol", "the name of the protocol to use to send traffic by the applications",
                           StringValue ("ns3::UdpSocketFactory"),
                           MakeStringAccessor (&TraceReplayClient::_protocol),
                           MakeStringChecker())
            .AddAttribute ("TraceFile", "the name of the file to read the traces from",
                           StringValue (""),
                           MakeStringAccessor (&TraceReplayClient::_traceFilename),
                           MakeStringChecker())
            .AddAttribute ("EnableCwndMonitor", "Enable monitoring the congestion window",
                           BooleanValue(false),
                           MakeBooleanAccessor(&TraceReplayClient::_enableCwndMonitor),
                           MakeBooleanChecker())
            .AddAttribute ("CongAlgoFolder", "the folder path to save the congestion algorithm records to",
                           StringValue (""),
                           MakeStringAccessor (&TraceReplayClient::_congAlgoFolder),
                           MakeStringChecker())
    ;
    return tid;
}

TraceReplayClient::TraceReplayClient() {
    NS_LOG_FUNCTION (this);
    _clientId = CLIENTS_COUNT++;
    _nbSentPkts = 0;
    _socket = 0;
    _sendEvent = EventId ();
}

TraceReplayClient::~TraceReplayClient() {
    NS_LOG_FUNCTION (this);
}

void TraceReplayClient::SetRemote(Address ip, uint16_t port) {
    NS_LOG_FUNCTION (this << ip << port);
    _peerAddress = ip;
    _peerPort = port;
}

void TraceReplayClient::DoDispose(void) {
    NS_LOG_FUNCTION (this);
    Application::DoDispose();
}

void TraceReplayClient::StartApplication(void) {
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

    // part to change starts from here
    _socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    _socket->SetAllowBroadcast (true);
    _sendEvent = Simulator::Schedule (Seconds (0.0), &TraceReplayClient::ScheduleSendEvents, this);

    // part for monitoring the congestion window
    if (_enableCwndMonitor) {
        _cwndMonitor = new CwndMonitor(_socket, _congAlgoFolder);
    }
}

void TraceReplayClient::StopApplication(void) {
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

void TraceReplayClient::Send(uint32_t payload_size) {
    NS_LOG_FUNCTION(this);
    NS_ASSERT (_sendEvent.IsExpired());

    Ptr<Packet> p;
    if (payload_size > (8+4)) {
        SeqTsHeader seqTs;
        seqTs.SetSeq(_nbSentPkts);
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
        ++_nbSentPkts;
        NS_LOG_INFO ("TraceDelay TX " << payload_size << " bytes to "
                                      << peerAddressStringStream.str () << " Uid: "
                                      << p->GetUid () << " Time: "
                                      << (Simulator::Now ()).GetSeconds ());
    }
    else {
        NS_LOG_INFO ("Error while sending " << payload_size << " bytes to "
                                             << peerAddressStringStream.str ());
        cout << "Error while sending " << payload_size << " bytes to "
             << ns3::Now() << endl;
        Simulator::Schedule(Seconds(0.0), &TraceReplayClient::Send, this, payload_size);
    }
}

void TraceReplayClient::ScheduleSendEvents() {
    NS_LOG_FUNCTION(this);

    ifstream traceInput(_traceFilename);
    string line;
    ns3::Time finalTimestamp;
    while(getline(traceInput, line)) {
        vector<string> pkt_attributes = trace_replay_helper::split(line, ',');
        ns3::Time timestamp = Time(pkt_attributes[1] + std::string("s"));// + NanoSeconds(rand() % 100000);
        uint32_t payload_size = stoi(pkt_attributes[2]);
        Simulator::Schedule(timestamp, &TraceReplayClient::Send, this, payload_size);

        // to know when the application should stop
        finalTimestamp = timestamp;
    }
    traceInput.close();

    // make the application stop itself based on the trace file data
    Simulator::Schedule(finalTimestamp, &TraceReplayClient::StopApplication, this);
}
