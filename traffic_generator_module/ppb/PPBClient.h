//
// Created by nal on 22.09.20.
//

#ifndef WEHE_PLUS_TOMOGRAPHY_PPBCLIENT_H
#define WEHE_PLUS_TOMOGRAPHY_PPBCLIENT_H

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

#include "../utility/Distribution.h"

using namespace std;
using namespace ns3;


class PPBClient : public Application {

private:
    virtual void StartApplication ();
    virtual void StopApplication ();

    void CancelEvents ();

    void StartSending();
    void StopSending();
    void SendPacket(uint32_t pktSize);

    Ptr<Socket>  _socket;
    TypeId _protocolTid;
    Address _peer;
    bool m_connected;

    uint32_t m_totalBytes;

    Time m_lastStartTime;
    EventId m_startStopEvent;
    EventId m_sendEvent;
    EventId	m_getUtilization;
    EventId	m_PoissonArrival;
    EventId	m_ppbp;
    EventId	m_ParetoDeparture;

    Distribution m_pktSize;

    TracedCallback< Ptr<const Packet> > m_txTrace;

    double m_burstArrivals;
    double m_burstLength;
    DataRate m_cbrRate;

    double m_h;
    double m_shape;
    Time m_timeSlot;
    int m_activebursts;
    bool m_offPeriod;
    uint32_t m_sent;

    void ScheduleStartEvent();
    void ScheduleStopEvent();
    void ConnectionSucceeded(Ptr<Socket>);
    void ConnectionFailed(Ptr<Socket>);


    void PPBP();
    void PoissonArrival();
    void ParetoDeparture();


    void ScheduleNextTx();

protected:
    virtual void DoDispose();

public:
    static TypeId GetTypeId(void);

    PPBClient();
    virtual ~PPBClient();

    uint32_t GetTotalBytes() const;

};


#endif //WEHE_PLUS_TOMOGRAPHY_PPBCLIENT_H
