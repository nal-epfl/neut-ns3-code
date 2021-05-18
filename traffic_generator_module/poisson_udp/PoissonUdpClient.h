//
// Created by Zeinab Shmeis on 03.06.20.
//

#ifndef NEUTRALITY_POISSONUDPCLIENT_H
#define NEUTRALITY_POISSONUDPCLIENT_H

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

using namespace ns3;
using namespace std;

class PoissonUdpClient : public Application {

private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    void Send(void);

    Ptr<RandomVariableStream> _interval; // interval between sending two packets
    uint32_t _size; // size of sent packets

    uint32_t _sent;
    Ptr<Socket> _socket;
    Address _peerAddress;
    uint16_t _peerPort;

    EventId _sendEvent;

protected:
    virtual void DoDispose(void);

public:
    static TypeId GetTypeId(void);

    PoissonUdpClient();
    virtual ~PoissonUdpClient();

    void SetRemote(Address ip, uint16_t port);
};


#endif //NEUTRALITY_POISSONUDPCLIENT_H
