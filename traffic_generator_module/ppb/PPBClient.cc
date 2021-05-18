//
// Created by nal on 22.09.20.
//

#include "PPBClient.h"

NS_LOG_COMPONENT_DEFINE ("PPBClient");

NS_OBJECT_ENSURE_REGISTERED (PPBClient);

TypeId PPBClient::GetTypeId (void) {

    static TypeId tid = TypeId ("ns3::PPBClient")
            .SetParent<Application> ()
            .SetGroupName("Applications")
            .AddConstructor<PPBClient> ()
            .AddAttribute ("BurstIntensity", "The data rate of each burst.",
                           DataRateValue (DataRate ("1Mb/s")),
                           MakeDataRateAccessor (&PPBClient::m_cbrRate),
                           MakeDataRateChecker ())
            .AddAttribute ("MeanBurstArrivals", "Mean Active Sources",
                           DoubleValue (5.0),
                           MakeDoubleAccessor (&PPBClient::m_burstArrivals),
                           MakeDoubleChecker<uint32_t> (0))
            .AddAttribute ("MeanBurstTimeLength", "Pareto distributed burst durations",
                           DoubleValue (0.05),
                           MakeDoubleAccessor (&PPBClient::m_burstLength),
                           MakeDoubleChecker<double> ())
            .AddAttribute ("H", "Hurst parameter",
                           DoubleValue (0.7),
                           MakeDoubleAccessor (&PPBClient::m_h),
                           MakeDoubleChecker<double> ())
            .AddAttribute ("Remote", "The address of the destination",
                           AddressValue (),
                           MakeAddressAccessor (&PPBClient::_peer),
                           MakeAddressChecker ())
            .AddAttribute ("Protocol", "The type of protocol to use.",
                           TypeIdValue (UdpSocketFactory::GetTypeId ()),
                           MakeTypeIdAccessor (&PPBClient::_protocolTid),
                           MakeTypeIdChecker ())
            .AddTraceSource ("Tx", "A new packet is created and is sent",
                             MakeTraceSourceAccessor (&PPBClient::m_txTrace),
                             "ns3::Packet::TracedCallback")
    ;
    return tid;
}

PPBClient::PPBClient() {
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

PPBClient::~PPBClient() {
    NS_LOG_FUNCTION_NOARGS ();
}

uint32_t PPBClient::GetTotalBytes() const {
    return m_totalBytes;
}

void PPBClient::DoDispose (void) {
    NS_LOG_FUNCTION_NOARGS ();

    _socket = 0;
    Application::DoDispose ();
}

void PPBClient::StartApplication() {
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

void PPBClient::PPBP() {
    NS_LOG_FUNCTION_NOARGS ();

    // Poisson
    double inter_burst_intervals;
    inter_burst_intervals = (double) 1/m_burstArrivals;

    Ptr<ExponentialRandomVariable> exp = CreateObject<ExponentialRandomVariable> ();
    exp->SetAttribute ("Mean", DoubleValue (inter_burst_intervals));

    Time t_poisson_arrival = Seconds (exp->GetValue());
    m_PoissonArrival = Simulator::Schedule(t_poisson_arrival,&PPBClient::PoissonArrival, this);

    // Pareto
    m_shape = 3 - 2 * m_h;
    m_timeSlot = Seconds((double) (m_shape - 1) * m_burstLength / m_shape);

    Ptr<ParetoRandomVariable> pareto = CreateObject<ParetoRandomVariable> ();
    pareto->SetAttribute ("Scale", DoubleValue (m_burstLength));
    pareto->SetAttribute ("Shape", DoubleValue (m_shape));

    double t_pareto = pareto->GetValue();
    m_ParetoDeparture = Simulator::Schedule(t_poisson_arrival + Seconds (t_pareto),&PPBClient::ParetoDeparture, this);

    // Next PBP event
    m_ppbp = Simulator::Schedule(t_poisson_arrival,&PPBClient::PPBP, this);
}

void PPBClient::PoissonArrival() {
    NS_LOG_FUNCTION_NOARGS ();
    ++m_activebursts;
//    NS_LOG_INFO(std::to_string(m_activebursts) + ", " + std::to_string(Simulator::Now().GetNanoSeconds()));
    if (m_offPeriod) ScheduleNextTx();
}

void PPBClient::ParetoDeparture() {
    NS_LOG_FUNCTION_NOARGS ();
    --m_activebursts;
//    NS_LOG_INFO(std::to_string(m_activebursts) + ", " + std::to_string(Simulator::Now().GetNanoSeconds()));
}

void PPBClient::StopApplication() {
    NS_LOG_FUNCTION_NOARGS ();

    CancelEvents ();
    if(_socket != 0) _socket->Close ();
    else NS_LOG_WARN("PPBClient found null socket to close in StopApplication");
}

void PPBClient::CancelEvents () {
    NS_LOG_FUNCTION_NOARGS ();
    Simulator::Cancel(m_sendEvent);
    Simulator::Cancel(m_startStopEvent);

    Simulator::Cancel(m_ppbp);
    Simulator::Cancel(m_PoissonArrival);
    Simulator::Cancel(m_ParetoDeparture);
}

void PPBClient::StartSending() {
    NS_LOG_FUNCTION_NOARGS ();
    m_lastStartTime = Simulator::Now();
    ScheduleNextTx();
    ScheduleStopEvent();
}

void PPBClient::StopSending() {
    NS_LOG_FUNCTION_NOARGS ();
    CancelEvents();

    ScheduleStartEvent();
}

void PPBClient::ScheduleNextTx() {
    NS_LOG_FUNCTION_NOARGS ();
    uint32_t pktSize = m_pktSize.GetRandInt();
    uint32_t bits = (pktSize + 30) * 8;
    Time nextTime(Seconds (bits / static_cast<double>(m_cbrRate.GetBitRate())));

    if (m_activebursts != 0) {
        m_offPeriod = false;
        double data_rate = (double) nextTime.GetSeconds() / m_activebursts;
        m_sendEvent = Simulator::Schedule(Seconds(data_rate),&PPBClient::SendPacket, this, pktSize);
    }
    else {
        m_offPeriod = true;
    }
}

void PPBClient::ScheduleStartEvent() {
    NS_LOG_FUNCTION_NOARGS ();
    m_ppbp = Simulator::Schedule(Seconds(0.0), &PPBClient::PPBP, this);
    m_startStopEvent = Simulator::Schedule(Seconds(0.0), &PPBClient::StartSending, this);
}

void PPBClient::ScheduleStopEvent() {
    NS_LOG_FUNCTION_NOARGS ();
}

void PPBClient::SendPacket(uint32_t pktSize) {
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

void PPBClient::ConnectionSucceeded(Ptr<Socket>) {
    NS_LOG_FUNCTION_NOARGS ();
    m_connected = true;
    ScheduleStartEvent();
}

void PPBClient::ConnectionFailed(Ptr<Socket>) {
    NS_LOG_FUNCTION_NOARGS ();
    cout << "PPBClient, Connection Failed" << endl;
}