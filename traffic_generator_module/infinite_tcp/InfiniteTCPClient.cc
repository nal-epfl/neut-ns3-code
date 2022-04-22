//
// Created by nal on 01.11.21.
//

#include "InfiniteTCPClient.h"

NS_LOG_COMPONENT_DEFINE ("InfiniteTCPClient");

NS_OBJECT_ENSURE_REGISTERED (InfiniteTCPClient);

TypeId InfiniteTCPClient::GetTypeId(void) {

    static TypeId tid = TypeId ("ns3::InfiniteTCPClient")
            .SetParent<Application> ()
            .SetGroupName("Applications")
            .AddConstructor<InfiniteTCPClient> ()
            .AddAttribute ("RemoteAddress",
                           "The destination Address of the outbound packets",
                           AddressValue (),
                           MakeAddressAccessor (&InfiniteTCPClient::_peerAddress),
                           MakeAddressChecker ())
            .AddAttribute ("RemotePort", "The destination port of the outbound packets",
                           UintegerValue (100),
                           MakeUintegerAccessor (&InfiniteTCPClient::_peerPort),
                           MakeUintegerChecker<uint16_t> ())
            .AddAttribute ("TcpProtocol", "The congestion control algorithm",
                           StringValue (""),
                           MakeStringAccessor(&InfiniteTCPClient::_tcpProtocol),
                           MakeStringChecker())
            .AddAttribute ("PacketSize",
                           "Size of packets generated. The minimum packet size is 12 bytes which is the size of the header carrying the sequence number and the time stamp.",
                           UintegerValue (1024),
                           MakeUintegerAccessor (&InfiniteTCPClient::_pktSize),
                           MakeUintegerChecker<uint32_t> (12,65507))
            .AddAttribute ("EnableCwndMonitor", "enable monitoring the cwnd for TCP applications",
                           BooleanValue (false),
                           MakeBooleanAccessor(&InfiniteTCPClient::_enableCwndMonitor),
                           MakeBooleanChecker())
            .AddAttribute ("ResultsFolder", "folder to which all results are saved",
                           StringValue (""),
                           MakeStringAccessor(&InfiniteTCPClient::_resultsFolder),
                           MakeStringChecker())
            .AddAttribute ("MaxSendingRate", "max rate of generating data",
                           DataRateValue (DataRate("20Mbps")),
                           MakeDataRateAccessor(&InfiniteTCPClient::_maxSendingRate),
                           MakeDataRateChecker())
    ;
    return tid;
}

uint32_t InfiniteTCPClient::APPS_COUNT = 0;

InfiniteTCPClient::InfiniteTCPClient() {
    NS_LOG_FUNCTION (this);
    _appId = ++APPS_COUNT;
    _nbSentPkts = 0;
    _socket = 0;
    _appPaused = false;
    _sendEvent = EventId ();
    _maxSendingRate = DataRate("20Mbps");
}

InfiniteTCPClient::~InfiniteTCPClient() {
    NS_LOG_FUNCTION (this);
}

void InfiniteTCPClient::SetRemote(Address ip, uint16_t port) {
    NS_LOG_FUNCTION (this << ip << port);
    _peerAddress = ip;
    _peerPort = port;
}

void InfiniteTCPClient::DoDispose(void) {
    NS_LOG_FUNCTION (this);
    Application::DoDispose ();
}

void InfiniteTCPClient::StartApplication(void) {
    NS_LOG_FUNCTION (this);

    if (_socket == 0)     {
        Ptr<TcpL4Protocol> tcpProtocol = GetNode()->GetObject<TcpL4Protocol> ();
        _socket = tcpProtocol->CreateSocket(ns3::TypeId::LookupByName(_tcpProtocol));

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

    // adjust socket buffers
    _socket->SetAttribute("RcvBufSize", UintegerValue(131072));
    _socket->SetAttribute("SndBufSize", UintegerValue(131072));

    // un-comment this code if you want to disable pacing for the measurement traffic
    //Ptr<TcpSocketBase> tcpSocket = _socket->GetObject<TcpSocketBase>();
    //tcpSocket->SetPacingStatus(false);

    _socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    _socket->SetAllowBroadcast (true);

    // This is added to avoid socket overflow
    _socket->SetSendCallback (MakeCallback (&InfiniteTCPClient::ResumeApp, this));

    // part for monitoring the congestion window
    if (_enableCwndMonitor) {
        string outputFolder = _resultsFolder + "/cong_algo_info_" + to_string(_appId) + "/server/";
        _cwndMonitor = new CwndMonitor(_socket, outputFolder);
    }

    _sendEvent = Simulator::Schedule (Seconds (0.0), &InfiniteTCPClient::SchedualeSend, this);
}

void InfiniteTCPClient::StopApplication(void) {
    NS_LOG_FUNCTION (this);
    _socket->Dispose();
    Simulator::Cancel (_sendEvent);
    if (_enableCwndMonitor) {
        _cwndMonitor->SaveCwndChanges();
        _cwndMonitor->SaveRtoChanges();
        _cwndMonitor->SaveRttChanges();
        _cwndMonitor->SaveCongStateChanges();
    }
}

bool InfiniteTCPClient::Send(void) {
    NS_LOG_FUNCTION (this);
    NS_ASSERT (_sendEvent.IsExpired ());

    SeqTsHeader seqTs;
    seqTs.SetSeq (_nbSentPkts);
    Ptr<Packet> p = Create<Packet> (_pktSize - (8 + 4)); // 8+4 : the size of the seqTs header
    p->AddHeader (seqTs);

    std::stringstream peerAddressStringStream;
    if (Ipv4Address::IsMatchingType (_peerAddress)) {
        peerAddressStringStream << Ipv4Address::ConvertFrom (_peerAddress);
    }
    else if (Ipv6Address::IsMatchingType (_peerAddress)) {
        peerAddressStringStream << Ipv6Address::ConvertFrom (_peerAddress);
    }

    if ((_socket->GetTxAvailable () > 0) && ((_socket->Send (p)) >= 0)) {
        ++_nbSentPkts;
        NS_LOG_INFO ("TraceDelay TX " << _pktSize << " bytes to "
                                      << peerAddressStringStream.str () << " Uid: "
                                      << p->GetUid () << " Time: "
                                      << (Simulator::Now ()).GetSeconds ());
        return true;
    }

    NS_LOG_INFO ("Error while sending " << _pktSize << " bytes to " << peerAddressStringStream.str ());
    return false;
}


void InfiniteTCPClient::SchedualeSend(void) {
    NS_LOG_FUNCTION(this);
    if (!Send()) {
        _appPaused = true;
        return;
    }
    Time nextTime (Seconds ((_pktSize * 8) / static_cast<double>(_maxSendingRate.GetBitRate ()))); // Time till next packet
    _sendEvent = Simulator::Schedule (nextTime, &InfiniteTCPClient::SchedualeSend, this);
}

// This is just a wrap-up function
void InfiniteTCPClient::ResumeApp(Ptr<Socket> localSocket, uint32_t txSpace) {
    if (_appPaused) {
        _appPaused = false;
        SchedualeSend();
    }
}
