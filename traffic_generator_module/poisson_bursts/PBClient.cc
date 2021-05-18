//
// Created by nal on 13.10.20.
//

#include "PBClient.h"

NS_LOG_COMPONENT_DEFINE ("PBClient");

NS_OBJECT_ENSURE_REGISTERED (PBClient);

TypeId PBClient::GetTypeId (void) {

    static TypeId tid = TypeId ("ns3::PBClient")
            .SetParent<Application> ()
            .SetGroupName("Applications")
            .AddConstructor<PBClient> ()
            .AddAttribute ("BurstIntensity", "The data rate of each burst.",
                           DataRateValue (DataRate ("1Mb/s")),
                           MakeDataRateAccessor (&PBClient::m_cbrRate),
                           MakeDataRateChecker ())
            .AddAttribute ("MeanBurstArrivals", "Mean Active Sources",
                           DoubleValue (5.0),
                           MakeDoubleAccessor (&PBClient::m_burstArrivals),
                           MakeDoubleChecker<uint32_t> (0))
            .AddAttribute ("MeanBurstTimeLength", "burst durations",
                           DoubleValue (0.05),
                           MakeDoubleAccessor (&PBClient::m_burstLength),
                           MakeDoubleChecker<double> ())
            .AddAttribute ("Remote", "The address of the destination",
                           AddressValue (),
                           MakeAddressAccessor (&PBClient::_peer),
                           MakeAddressChecker ())
            .AddAttribute ("Protocol", "The type of protocol to use.",
                           TypeIdValue (UdpSocketFactory::GetTypeId ()),
                           MakeTypeIdAccessor (&PBClient::_protocolTid),
                           MakeTypeIdChecker ())
            .AddTraceSource ("Tx", "A new packet is created and is sent",
                             MakeTraceSourceAccessor (&PBClient::m_txTrace),
                             "ns3::Packet::TracedCallback")
    ;
    return tid;
}

PBClient::PBClient() {
    NS_LOG_FUNCTION_NOARGS ();
    _socket = 0;
    m_connected = false;
    m_lastStartTime = Seconds (0);
    m_totalBytes = 0;
    m_activebursts = 0;
    m_offPeriod = true;
    m_sent = 0;
    m_pktSize = Distribution((string)(getenv("PWD")) + "/data/CAIDA_pkt_sizes_distribution.csv");
}

PBClient::~PBClient() {
    NS_LOG_FUNCTION_NOARGS ();
}

uint32_t PBClient::GetTotalBytes() const {
    return m_totalBytes;
}

void PBClient::DoDispose (void) {
    NS_LOG_FUNCTION_NOARGS ();

    _socket = 0;
    Application::DoDispose ();
}

void PBClient::StartApplication() {
    NS_LOG_FUNCTION_NOARGS ();

    // Create the socket if not already
    if (!_socket)
    {
        _socket = Socket::CreateSocket (GetNode(), _protocolTid);
        _socket->Bind ();
        _socket->Connect (_peer);
    }
    // Insure no pending event
    CancelEvents ();
    ScheduleStartEvent();
}

void PBClient::PBP() {
    NS_LOG_FUNCTION_NOARGS ();

    // Poisson Arrival
    Ptr<ExponentialRandomVariable> expArrival = CreateObject<ExponentialRandomVariable> ();
    expArrival->SetAttribute ("Mean", DoubleValue (1/m_burstArrivals));
    expArrival->SetAttribute ("Bound", DoubleValue (1/m_burstArrivals));

//    Ptr<NormalRandomVariable> ndArrival = CreateObject<NormalRandomVariable> ();
//    ndArrival->SetAttribute ("Mean", DoubleValue (1.0 / m_burstArrivals));
//    ndArrival->SetAttribute ("Variance", DoubleValue (0.5));
//    ndArrival->SetAttribute ("Bound", DoubleValue (0.5));

    Time t_poisson_arrival = Seconds (expArrival->GetValue() + (1/(2*m_burstArrivals)));
//    cout << t_poisson_arrival.GetSeconds() << endl;
    m_burstArrival = Simulator::Schedule(t_poisson_arrival, &PBClient::BurstArrival, this);

    // Burst duration
//    Ptr<UniformRandomVariable> uvDuration = CreateObject<UniformRandomVariable> ();
//    uvDuration->SetAttribute ("Min", DoubleValue (m_burstLength - (m_burstLength/2)));
//    uvDuration->SetAttribute ("Max", DoubleValue (m_burstLength + (m_burstLength/2)));

    Ptr<NormalRandomVariable> ndDuration = CreateObject<NormalRandomVariable> ();
    ndDuration->SetAttribute ("Mean", DoubleValue (m_burstLength));
    ndDuration->SetAttribute ("Variance", DoubleValue (0.001));
    ndDuration->SetAttribute ("Bound", DoubleValue (0.001));

    double t_duration = ndDuration->GetValue(); // expDuration->GetValue();
    m_burstDeparture = Simulator::Schedule(t_poisson_arrival + Seconds (t_duration), &PBClient::BurstDeparture, this);

    // Next PBP event
    m_pbp = Simulator::Schedule(t_poisson_arrival, &PBClient::PBP, this);
}

void PBClient::BurstArrival() {
    NS_LOG_FUNCTION_NOARGS ();
    ++m_activebursts;
//    NS_LOG_INFO(std::to_string(m_activebursts) + ", " + std::to_string(Simulator::Now().GetNanoSeconds()));
    if (m_offPeriod) ScheduleNextTx();
}

void PBClient::BurstDeparture() {
    NS_LOG_FUNCTION_NOARGS ();
    --m_activebursts;
//    NS_LOG_INFO(std::to_string(m_activebursts) + ", " + std::to_string(Simulator::Now().GetNanoSeconds()));
}

void PBClient::StopApplication() {
    NS_LOG_FUNCTION_NOARGS ();

    CancelEvents ();
    if(_socket != 0) _socket->Close ();
    else NS_LOG_WARN("PBClient found null socket to close in StopApplication");
}

void PBClient::CancelEvents () {
    NS_LOG_FUNCTION_NOARGS ();
    Simulator::Cancel(m_sendEvent);
    Simulator::Cancel(m_startStopEvent);

    Simulator::Cancel(m_pbp);
    Simulator::Cancel(m_burstArrival);
    Simulator::Cancel(m_burstDeparture);
}

void PBClient::StartSending() {
    NS_LOG_FUNCTION_NOARGS ();
    m_lastStartTime = Simulator::Now();
    ScheduleNextTx();
    ScheduleStopEvent();
}

void PBClient::StopSending() {
    NS_LOG_FUNCTION_NOARGS ();
    CancelEvents();

    ScheduleStartEvent();
}

void PBClient::ScheduleNextTx() {
    NS_LOG_FUNCTION_NOARGS ();
    uint32_t pktSize = m_pktSize.GetRandInt();
    uint32_t bits = (pktSize + 30) * 8;
    Time nextTime(Seconds (bits / static_cast<double>(m_cbrRate.GetBitRate())));

    if (m_activebursts != 0) {
        m_offPeriod = false;
        double data_rate = (double) nextTime.GetSeconds();// / m_activebursts;
        m_sendEvent = Simulator::Schedule(Seconds(data_rate),&PBClient::SendPacket, this, pktSize);
    }
    else {
        m_offPeriod = true;
    }
}

void PBClient::ScheduleStartEvent() {
    NS_LOG_FUNCTION_NOARGS ();
    m_pbp = Simulator::Schedule(Seconds(0.0), &PBClient::PBP, this);
    m_startStopEvent = Simulator::Schedule(Seconds(0.0), &PBClient::StartSending, this);
}

void PBClient::ScheduleStopEvent() {
    NS_LOG_FUNCTION_NOARGS ();
}

void PBClient::SendPacket(uint32_t pktSize) {
    NS_LOG_FUNCTION_NOARGS ();

    SeqTsHeader seqTs;
    seqTs.SetSeq (m_sent);
    Ptr<Packet> packet = Create<Packet> (pktSize > (8+4) ? pktSize - (8+4) : (8+4)); // 8+4 : the size of the seqTs header
    packet->AddHeader (seqTs);
    ++m_sent;

    m_txTrace (packet);
    _socket->Send (packet);
    m_totalBytes += packet->GetSize();
    m_lastStartTime = Simulator::Now();
    ScheduleNextTx();
}

void PBClient::ConnectionSucceeded(Ptr<Socket>) {
    NS_LOG_FUNCTION_NOARGS ();
    m_connected = true;
    ScheduleStartEvent();
}

void PBClient::ConnectionFailed(Ptr<Socket>) {
    NS_LOG_FUNCTION_NOARGS ();
    cout << "PBClient, Connection Failed" << endl;
}
