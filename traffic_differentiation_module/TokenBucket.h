//
// Created by Zeinab Shmeis on 16.04.20.
//

#ifndef NEUTRALITY_TOKENBUCKET_H
#define NEUTRALITY_TOKENBUCKET_H

#include "ns3/core-module.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;
using namespace std;

class TokenBucket {

    private:

        /** Parameters for the differentiated Service **/
        uint8_t _tos_id;

        /** Parameters of the token bucket **/
        uint32_t _burst; // in bytes
        uint32_t _mtu;
        DataRate _rate;
        DataRate _peakRate;

        /** Variables used by the token bucket **/
        TracedValue<uint32_t> _btokens;
        TracedValue<uint32_t> _ptokens;
        Time _timeCheckPoint;


    public:

        TokenBucket();
        TokenBucket(void (*fun)(TokenBucket));
        TokenBucket(uint8_t tos_id, uint32_t burst, DataRate rate);
        TokenBucket(uint8_t tos_id, uint32_t burst, DataRate rate, uint32_t mtu, DataRate peakRate);

        uint8_t GetTosId() const;
        uint32_t GetBurst() const;
        uint32_t GetMtu() const;
        const DataRate &GetRate() const;
        const DataRate &GetPeakRate() const;

        void SetMtu(uint32_t mtu);

        bool ConsumeTokens(uint32_t nbTokens);
        Time CalculateDelay(uint32_t nbTokens);

        string toString() const;

        int operator == (const TokenBucket &object) const;

};


std::ostream &operator << (std::ostream &os, const TokenBucket &tb);
std::istream &operator >> (std::istream &is, TokenBucket &tb);

ATTRIBUTE_HELPER_HEADER(TokenBucket);


#endif //NEUTRALITY_TOKENBUCKET_H
