//
// Created by Zeinab Shmeis on 03.06.20.
//

#include "ProbesSender.h"

NS_LOG_COMPONENT_DEFINE ("ProbesSender");

NS_OBJECT_ENSURE_REGISTERED (ProbesSender);

TypeId ProbesSender::GetTypeId() {
    
    static TypeId tid = TypeId ("ns3::ProbesSender")
            .SetParent<Application> ()
            .SetGroupName("Applications")
            .AddConstructor<ProbesSender> ()
            .AddAttribute ("AppTag", "A tag to identify the application",
                           StringValue (""),
                           MakeStringAccessor(&ProbesSender::_appTag),
                           MakeStringChecker())
            .AddAttribute ("RemoteAddress",
                           "The destination Address of the outbound packets",
                           AddressValue (),
                           MakeAddressAccessor (&ProbesSender::_receiverAddress),
                           MakeAddressChecker ())
            .AddAttribute ("Protocol", "protocol used by the socket: ns3::TcpSocketFactory or ns3::UdpSocketFactory",
                           StringValue ("ns3::UdpSocketFactory"),
                           MakeStringAccessor(&ProbesSender::_protocol),
                           MakeStringChecker())
            .AddAttribute ("Interval",
                           "A RandomVariableStream used to pick the time to wait between packets",
                           StringValue("ns3::ConstantRandomVariable[Constant=1.0]"),
                           MakePointerAccessor (&ProbesSender::_interval),
                           MakePointerChecker <RandomVariableStream>())
            .AddAttribute ("PacketSize",
                           "Size of packets generated. The minimum packet size is 12 bytes which is the size of the header carrying the sequence number and the time stamp.",
                           UintegerValue (1024),
                           MakeUintegerAccessor (&ProbesSender::_pktSize),
                           MakeUintegerChecker<uint32_t> (12,65507))
            .AddAttribute ("EnablePacing", "enable pacing of tcp packets",
                           BooleanValue (true),
                           MakeBooleanAccessor(&ProbesSender::_enablePacing),
                           MakeBooleanChecker())
            .AddAttribute ("EnableCwndMonitor", "enable monitoring the cwnd for TCP applications",
                           BooleanValue (false),
                           MakeBooleanAccessor(&ProbesSender::_enableCwndMonitor),
                           MakeBooleanChecker())
            .AddAttribute ("ResultsFolder", "folder to which all results are saved",
                           StringValue (""),
                           MakeStringAccessor(&ProbesSender::_resultsFolder),
                           MakeStringChecker())
    ;
    return tid;
}

ProbesSender::ProbesSender() {
    NS_LOG_FUNCTION (this);

    _nbSentPkts = 0;
    _pktSize = 0;
    _socket = nullptr;
    _appPaused = false;
    _sendEvent = EventId ();
    _enablePacing = true;
}

ProbesSender::~ProbesSender() {
    NS_LOG_FUNCTION (this);
}

void ProbesSender::DoDispose() {
    NS_LOG_FUNCTION (this);

    Application::DoDispose ();
}

void ProbesSender::StartApplication() {
    NS_LOG_FUNCTION (this);

    if (!_socket)     {
        TypeId tid = TypeId::LookupByName (_protocol);
        _socket = Socket::CreateSocket (GetNode (), tid);
        if (_socket->Bind () == -1) {
            NS_FATAL_ERROR ("Failed to bind socket");
        }
        _socket->Connect(_receiverAddress);
    }

    // adjust socket buffers
    _socket->SetAttribute("RcvBufSize", UintegerValue(131072));
    _socket->SetAttribute("SndBufSize", UintegerValue(131072));

    _socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    _socket->SetAllowBroadcast (true);

    // This is added to avoid socket overflow
    _socket->SetSendCallback (MakeCallback (&ProbesSender::ResumeApp, this));

    // to enable/disable pacing for the measurement traffic
    if(_protocol == "ns3::TcpSocketFactory") {
        Ptr<TcpSocketBase> tcpSocket = _socket->GetObject<TcpSocketBase>();
        tcpSocket->SetPacingStatus(_enablePacing);
    }

    // part for monitoring the congestion window
    if (_enableCwndMonitor) {
        string outputFolder = _resultsFolder + "/cong_algo_info_" + _appTag + "/sender/";
        _cwndMonitor = new CwndMonitor(_socket, outputFolder);
    }

    _sendEvent = Simulator::Schedule (Seconds (0.0), &ProbesSender::ScheduleNextSend, this);
}

void ProbesSender::StopApplication() {
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

bool ProbesSender::Send() {
    NS_LOG_FUNCTION (this);

    SeqTsHeader seqTs;
    seqTs.SetSeq (_nbSentPkts);
    Ptr<Packet> p = Create<Packet> (_pktSize - (8 + 4)); // 8+4 : the size of the seqTs header
    p->AddHeader (seqTs);

    if ((_socket->GetTxAvailable () > 0) && (_socket->Send (p)) >= 0) {
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


void ProbesSender::ScheduleNextSend() {
    NS_LOG_FUNCTION(this);

    if (!Send()) {
        _appPaused = true;
        return;
    }
    double interval = _interval->GetValue();
    _sendEvent = Simulator::Schedule(Seconds(interval), &ProbesSender::ScheduleNextSend, this);

}

// This is just a wrap-up function
void ProbesSender::ResumeApp(Ptr<Socket> localSocket, uint32_t txSpace) {
    NS_LOG_FUNCTION(this);

    if (_appPaused) {
        _appPaused = false;
        _sendEvent = Simulator::Schedule(Seconds(0.0), &ProbesSender::ScheduleNextSend, this);
    }
}
