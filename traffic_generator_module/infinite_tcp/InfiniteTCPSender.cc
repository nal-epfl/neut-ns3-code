//
// Created by nal on 01.11.21.
//

#include "InfiniteTCPSender.h"

NS_LOG_COMPONENT_DEFINE ("InfiniteTCPSender");

NS_OBJECT_ENSURE_REGISTERED (InfiniteTCPSender);

TypeId InfiniteTCPSender::GetTypeId() {

    static TypeId tid = TypeId ("ns3::InfiniteTCPSender")
            .SetParent<Application> ()
            .SetGroupName("Applications")
            .AddConstructor<InfiniteTCPSender> ()
            .AddAttribute ("AppTag", "A tag to identify the application",
                           StringValue (""),
                           MakeStringAccessor(&InfiniteTCPSender::_appTag),
                           MakeStringChecker())
            .AddAttribute ("RemoteAddress",
                           "The destination Address of the outbound packets",
                           AddressValue (),
                           MakeAddressAccessor (&InfiniteTCPSender::_receiverAddress),
                           MakeAddressChecker ())
            .AddAttribute ("TcpProtocol", "The congestion control algorithm",
                           StringValue (""),
                           MakeStringAccessor(&InfiniteTCPSender::_tcpProtocol),
                           MakeStringChecker())
            .AddAttribute ("PacketSize",
                           "Size of packets generated. The minimum packet size is 12 bytes which is the size of the header carrying the sequence number and the time stamp.",
                           UintegerValue (1024),
                           MakeUintegerAccessor (&InfiniteTCPSender::_pktSize),
                           MakeUintegerChecker<uint32_t> (12,65507))
            .AddAttribute ("MaxSendingRate", "max rate of generating data",
                           DataRateValue (DataRate("20Mbps")),
                           MakeDataRateAccessor(&InfiniteTCPSender::_maxSendingRate),
                           MakeDataRateChecker())
            .AddAttribute ("EnableCwndMonitor", "enable monitoring the cwnd for TCP applications",
                           BooleanValue (false),
                           MakeBooleanAccessor(&InfiniteTCPSender::_enableCwndMonitor),
                           MakeBooleanChecker())
            .AddAttribute ("ResultsFolder", "folder to which all results are saved",
                           StringValue (""),
                           MakeStringAccessor(&InfiniteTCPSender::_resultsFolder),
                           MakeStringChecker())
    ;
    return tid;
}

InfiniteTCPSender::InfiniteTCPSender() {
    NS_LOG_FUNCTION(this);

    _appTag = "";
    _nbSentPkts = 0;
    _socket = nullptr;
    _appPaused = false;
    _sendEvent = EventId ();
    _pktSize = 1024;
    _maxSendingRate = DataRate("20Mbps");
}

InfiniteTCPSender::~InfiniteTCPSender() {
    NS_LOG_FUNCTION (this);
}

void InfiniteTCPSender::DoDispose() {
    NS_LOG_FUNCTION (this);

    Application::DoDispose ();
}

void InfiniteTCPSender::StartApplication() {
    NS_LOG_FUNCTION (this);

    if (!_socket)     {
        Ptr<TcpL4Protocol> tcpProtocol = GetNode()->GetObject<TcpL4Protocol> ();
        _socket = tcpProtocol->CreateSocket(ns3::TypeId::LookupByName(_tcpProtocol));
        if (_socket->Bind () == -1) {
            NS_FATAL_ERROR ("Failed to bind socket");
        }
        _socket->Connect(_receiverAddress);
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
    _socket->SetSendCallback (MakeCallback (&InfiniteTCPSender::ResumeApp, this));

    // part for monitoring the congestion window
    if (_enableCwndMonitor) {
        string outputFolder = _resultsFolder + "/cong_algo_info_" + _appTag + "/sender/";
        _cwndMonitor = new CwndMonitor(_socket, outputFolder);
    }

    _sendEvent = Simulator::Schedule (Seconds (0.0), &InfiniteTCPSender::ScheduleNextSend, this);
}

void InfiniteTCPSender::StopApplication() {
    NS_LOG_FUNCTION (this);

    _socket->Dispose();
    Simulator::Cancel (_sendEvent);

    if (_cwndMonitor) {
        _cwndMonitor->SaveCwndChanges();
        _cwndMonitor->SaveRtoChanges();
        _cwndMonitor->SaveRttChanges();
        _cwndMonitor->SaveCongStateChanges();
    }
}

bool InfiniteTCPSender::Send() {
    NS_LOG_FUNCTION (this);

    SeqTsHeader seqTs;
    seqTs.SetSeq (_nbSentPkts);
    Ptr<Packet> p = Create<Packet> (_pktSize - (8 + 4)); // 8+4 : the size of the seqTs header
    p->AddHeader (seqTs);

    if ((_socket->GetTxAvailable () > 0) && ((_socket->Send (p)) >= 0)) {
        ++_nbSentPkts;
        NS_LOG_INFO ("TraceDelay TX " << _pktSize << " bytes to "
                                      << _receiverAddress << " Uid: "
                                      << p->GetUid () << " Time: "
                                      << (Simulator::Now ()).GetSeconds ());
        return true;
    }

    NS_LOG_INFO ("Error while sending " << _pktSize << " bytes to " << _receiverAddress);
    return false;
}


void InfiniteTCPSender::ScheduleNextSend() {
    NS_LOG_FUNCTION(this);

    if (!Send()) {
        _appPaused = true;
        return;
    }
    Time nextTime (Seconds ((_pktSize * 8) / static_cast<double>(_maxSendingRate.GetBitRate ()))); // Time till next packet
    _sendEvent = Simulator::Schedule (nextTime, &InfiniteTCPSender::ScheduleNextSend, this);
}

// This is just a wrap-up function
void InfiniteTCPSender::ResumeApp(Ptr<Socket> localSocket, uint32_t txSpace) {
    NS_LOG_FUNCTION(this);

    if (_appPaused) {
        _appPaused = false;
        _sendEvent = Simulator::Schedule(Seconds(0.0), &InfiniteTCPSender::ScheduleNextSend, this);
    }
}
