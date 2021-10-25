//
// Created by Zeinab Shmeis on 03.06.20.
//

#include "PoissonClient.h"

NS_LOG_COMPONENT_DEFINE ("PoissonClient");

NS_OBJECT_ENSURE_REGISTERED (PoissonClient);

TypeId PoissonClient::GetTypeId(void) {

    static TypeId tid = TypeId ("ns3::PoissonClient")
            .SetParent<Application> ()
            .SetGroupName("Applications")
            .AddConstructor<PoissonClient> ()
            .AddAttribute ("Interval",
                           "A RandomVariableStream used to pick the time to wait between packets",
                           StringValue("ns3::ConstantRandomVariable[Constant=1.0]"),
                           MakePointerAccessor (&PoissonClient::_interval),
                           MakePointerChecker <RandomVariableStream>())
            .AddAttribute ("RemoteAddress",
                           "The destination Address of the outbound packets",
                           AddressValue (),
                           MakeAddressAccessor (&PoissonClient::_peerAddress),
                           MakeAddressChecker ())
            .AddAttribute ("RemotePort", "The destination port of the outbound packets",
                           UintegerValue (100),
                           MakeUintegerAccessor (&PoissonClient::_peerPort),
                           MakeUintegerChecker<uint16_t> ())
            .AddAttribute ("Protocol", "protocol used by the socket: ns3::TcpSocketFactory or ns3::UdpSocketFactory",
                           StringValue ("ns3::UdpSocketFactory"),
                           MakeStringAccessor(&PoissonClient::_protocol),
                           MakeStringChecker())
            .AddAttribute ("PacketSize",
                           "Size of packets generated. The minimum packet size is 12 bytes which is the size of the header carrying the sequence number and the time stamp.",
                           UintegerValue (1024),
                           MakeUintegerAccessor (&PoissonClient::_size),
                           MakeUintegerChecker<uint32_t> (12,65507))
            .AddAttribute ("EnableCwndMonitor", "enable monitoring the cwnd for TCP applications",
                           BooleanValue (false),
                           MakeBooleanAccessor(&PoissonClient::_enableCwndMonitor),
                           MakeBooleanChecker())
            .AddAttribute ("ResultsFolder", "folder to which all results are saved",
                           StringValue (""),
                           MakeStringAccessor(&PoissonClient::_resultsFolder),
                           MakeStringChecker())
    ;
    return tid;
}

uint32_t PoissonClient::APPS_COUNT = 0;

PoissonClient::PoissonClient() {
    NS_LOG_FUNCTION (this);
    _appId = ++APPS_COUNT;
    _sent = 0;
    _socket = 0;
    _sendEvent = EventId ();
}

PoissonClient::~PoissonClient() {
    NS_LOG_FUNCTION (this);
}

void PoissonClient::SetRemote(Address ip, uint16_t port) {
    NS_LOG_FUNCTION (this << ip << port);
    _peerAddress = ip;
    _peerPort = port;
}

void PoissonClient::DoDispose(void) {
    NS_LOG_FUNCTION (this);
    Application::DoDispose ();
}

void PoissonClient::StartApplication(void) {
    NS_LOG_FUNCTION (this);

    if (_socket == 0)     {
        TypeId tid = TypeId::LookupByName (_protocol);
        isTCP = (_protocol == "ns3::TcpSocketFactory");
        _socket = Socket::CreateSocket (GetNode (), tid);
        if (Ipv4Address::IsMatchingType (_peerAddress) == true) {
            if (_socket->Bind () == -1) {
                NS_FATAL_ERROR ("Failed to bind socket");
            }
            _socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(_peerAddress), _peerPort));
        }
        else if (Ipv6Address::IsMatchingType (_peerAddress) == true) {
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

    _socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    _socket->SetAllowBroadcast (true);

    // This is added to avoid socket overflow
    _socket->SetSendCallback (MakeCallback (&PoissonClient::ResumeApp, this));

    // part for monitoring the congestion window
    if (isTCP && _enableCwndMonitor) {
        string outputFolder = _resultsFolder + "/cong_algo_info_" + to_string(_appId) + "/server/";
        _cwndMonitor = new CwndMonitor(_socket, outputFolder);
    }

    _sendEvent = Simulator::Schedule (Seconds (0.0), &PoissonClient::SchedualeSend, this);
}

void PoissonClient::StopApplication(void) {
    NS_LOG_FUNCTION (this);
    Simulator::Cancel (_sendEvent);
    if (isTCP && _enableCwndMonitor) {
        _cwndMonitor->SaveCwndChanges();
        _cwndMonitor->SaveRtoChanges();
        _cwndMonitor->SaveRttChanges();
        _cwndMonitor->SaveCongStateChanges();
    }
}

bool PoissonClient::Send(void) {
    SeqTsHeader seqTs;
    seqTs.SetSeq (_sent);
    Ptr<Packet> p = Create<Packet> (_size-(8+4)); // 8+4 : the size of the seqTs header
    p->AddHeader (seqTs);

    std::stringstream peerAddressStringStream;
    if (Ipv4Address::IsMatchingType (_peerAddress)) {
        peerAddressStringStream << Ipv4Address::ConvertFrom (_peerAddress);
    }
    else if (Ipv6Address::IsMatchingType (_peerAddress)) {
        peerAddressStringStream << Ipv6Address::ConvertFrom (_peerAddress);
    }

    if ((_socket->Send (p)) >= 0) {
        ++_sent;
        NS_LOG_INFO ("TraceDelay TX " << _size << " bytes to "
                                      << peerAddressStringStream.str () << " Uid: "
                                      << p->GetUid () << " Time: "
                                      << (Simulator::Now ()).GetSeconds ());
        return true;
    }

    NS_LOG_INFO ("Error while sending " << _size << " bytes to " << peerAddressStringStream.str ());
    return false;
}


void PoissonClient::SchedualeSend(void) {
    NS_LOG_FUNCTION(this);
    NS_LOG_FUNCTION(this);

    if (Send()) {
        double interval = _interval->GetValue();
        _sendEvent = Simulator::Schedule(Seconds(interval), &PoissonClient::SchedualeSend, this);
    }
}

// This is just a wrap-up function
void PoissonClient::ResumeApp(Ptr<Socket> localSocket, uint32_t txSpace) {
    SchedualeSend();
}
