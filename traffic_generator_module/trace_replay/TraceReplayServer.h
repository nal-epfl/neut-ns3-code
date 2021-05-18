//
// Created by nal on 27.04.21.
//

#ifndef WEHE_P_TOMOGRAPHY_TRACEREPLAYSERVER_H
#define WEHE_P_TOMOGRAPHY_TRACEREPLAYSERVER_H

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

using namespace ns3;
using namespace std;

class TraceReplayServer : public Application {
private:

    virtual void StartApplication(void);
    virtual void StopApplication(void);

    void HandleTCPAccept (Ptr<Socket> socket, const Address& from);
    void Recv(Ptr<Socket> socket);

    Address _serverAddress;
    string _protocol;
    Ptr<Socket> _socket;

protected:
    virtual void DoDispose(void);

public:
    static TypeId GetTypeId(void);

    TraceReplayServer();
    virtual ~TraceReplayServer();
};


#endif //WEHE_P_TOMOGRAPHY_TRACEREPLAYSERVER_H
