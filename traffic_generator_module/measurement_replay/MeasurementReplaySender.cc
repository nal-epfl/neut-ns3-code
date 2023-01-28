//
// Created by nal on 25.10.21.
//

#include "MeasurementReplaySender.h"


NS_LOG_COMPONENT_DEFINE ("MeasurementReplaySender");

NS_OBJECT_ENSURE_REGISTERED (MeasurementReplaySender);

TypeId MeasurementReplaySender::GetTypeId() {

    static TypeId tid = TypeId ("ns3::MeasurementReplaySender")
            .SetParent<Application> ()
            .SetGroupName("Applications")
            .AddConstructor<MeasurementReplaySender> ()
            .AddAttribute ("AppTag", "A tag to identify the application",
                           StringValue (""),
                           MakeStringAccessor(&MeasurementReplaySender::_appTag),
                           MakeStringChecker())
            .AddAttribute ("RemoteAddress",
                           "The destination Address of the outbound packets",
                           AddressValue (),
                           MakeAddressAccessor (&MeasurementReplaySender::_receiverAddress),
                           MakeAddressChecker ())
            .AddAttribute ("Protocol", "the name of the protocol to use to send traffic by the applications",
                           StringValue ("ns3::UdpSocketFactory"),
                           MakeStringAccessor (&MeasurementReplaySender::_protocol),
                           MakeStringChecker())
            .AddAttribute ("TraceFile", "the name of the file to read the traces from",
                           StringValue (""),
                           MakeStringAccessor (&MeasurementReplaySender::_traceFilename),
                           MakeStringChecker())
            .AddAttribute ("EnablePacing", "enable pacing of tcp packets",
                           BooleanValue (true),
                           MakeBooleanAccessor(&MeasurementReplaySender::_enablePacing),
                           MakeBooleanChecker())
            .AddAttribute ("EnableCwndMonitor", "Enable monitoring the congestion window",
                           BooleanValue(false),
                           MakeBooleanAccessor(&MeasurementReplaySender::_enableCwndMonitor),
                           MakeBooleanChecker())
            .AddAttribute ("ResultsFolder", "the folder path to save the congestion algorithm records to",
                           StringValue (""),
                           MakeStringAccessor (&MeasurementReplaySender::_resultsFolder),
                           MakeStringChecker())
    ;
    return tid;
}

MeasurementReplaySender::MeasurementReplaySender() {
    NS_LOG_FUNCTION (this);

    _traceItemIdx = 0;
    _socket = nullptr;
    _appPaused = false;
    _sendEvent = EventId ();
    _enablePacing = true;
}

MeasurementReplaySender::~MeasurementReplaySender() {
    NS_LOG_FUNCTION (this);
}

void MeasurementReplaySender::LoadTrace() {
    NS_LOG_FUNCTION(this);

    ifstream traceInput(_traceFilename);
    string line;
    while(getline(traceInput, line)) {
        vector<string> pkt_attributes = SplitStr(line, ',');

        uint32_t frameNb = stoi(pkt_attributes[0]);
        ns3::Time timestamp = Seconds(stod(pkt_attributes[1]));
        uint32_t payload_size = stoi(pkt_attributes[2]);

        _traceItems.push_back({frameNb, timestamp, payload_size});
    }
    traceInput.close();

    // make the application stop itself based on the trace file data
    Simulator::Schedule(_traceItems.back().timestamp + Seconds(1), &MeasurementReplaySender::StopApplication, this);
}

void MeasurementReplaySender::DoDispose() {
    NS_LOG_FUNCTION (this);

    Application::DoDispose();
}

void MeasurementReplaySender::StartApplication() {
    NS_LOG_FUNCTION (this);

    LoadTrace();

    if (!_socket)     {
        TypeId tid = TypeId::LookupByName (_protocol);
        _socket = Socket::CreateSocket (GetNode (), tid);
        if (_socket->Bind () == -1) {
            NS_FATAL_ERROR ("Failed to bind socket");
        }
        _socket->Connect(_receiverAddress);
    }

    _startTime = Simulator::Now();

    // adjust socket buffers
    _socket->SetAttribute("RcvBufSize", UintegerValue(131072));
    _socket->SetAttribute("SndBufSize", UintegerValue(131072));

    // part to change starts from here
    _socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    _socket->SetAllowBroadcast (true);
    _sendEvent = Simulator::Schedule (Seconds (0.0), &MeasurementReplaySender::ScheduleNextSend, this);

    // This is added to avoid socket overflow
    _socket->SetSendCallback (MakeCallback (&MeasurementReplaySender::ResumeApp, this));

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

}

void MeasurementReplaySender::StopApplication() {
    NS_LOG_FUNCTION (this);

    _socket->Dispose();
    Simulator::Cancel (_sendEvent);

    // part for monitoring the congestion window
    if (_cwndMonitor) {
        _cwndMonitor->SaveCwndChanges();
        _cwndMonitor->SaveRtoChanges();
        _cwndMonitor->SaveRttChanges();
        _cwndMonitor->SaveCongStateChanges();
    }
}

bool MeasurementReplaySender::Send(const TraceReplayItem &item) {
    NS_LOG_FUNCTION(this);

    SeqTsHeader seqTs;
    seqTs.SetSeq (item.frameNb);
    uint32_t segmentSize = item.payloadSize > (8+4) ? item.payloadSize - (8+4) : item.payloadSize; // 8+4 : the size of the seqTs header
    Ptr<Packet> p = Create<Packet>(segmentSize);
    if(item.payloadSize > (8+4)) { p->AddHeader(seqTs); }

    if ((_socket->Send (p)) < 0) {
        NS_LOG_INFO ("Error while sending " << item.payloadSize << " bytes to "
                                            << Ipv4Address::ConvertFrom (_receiverAddress));
        return false;
    }

    return true;
}

void MeasurementReplaySender::ScheduleNextSend() {
    NS_LOG_FUNCTION(this);

    if(_traceItemIdx >= _traceItems.size()) {return;}

    if (!Send(_traceItems[_traceItemIdx])) {
        _appPaused = true;
        return;
    }
     _traceItemIdx++;
     if(_traceItemIdx >= _traceItems.size()) {return;}

     TraceReplayItem nextItem = _traceItems[_traceItemIdx];
     ns3::Time relativeTime = Simulator::Now() - _startTime;
     ns3::Time remainingTime = (nextItem.timestamp > relativeTime) ? nextItem.timestamp - relativeTime : Seconds(0);
     remainingTime += MicroSeconds(helper_methods::GetRandomNumber(0, 50)); // to add randomness
     _sendEvent = Simulator::Schedule(remainingTime, &MeasurementReplaySender::ScheduleNextSend, this);
}

// This is just a wrap-up function
void MeasurementReplaySender::ResumeApp(Ptr<Socket> localSocket, uint32_t txSpace) {
    NS_LOG_FUNCTION(this);

    if(_appPaused) {
        _appPaused = false;
        _sendEvent = Simulator::Schedule(Seconds(0.0), &MeasurementReplaySender::ScheduleNextSend, this);
    }
}
