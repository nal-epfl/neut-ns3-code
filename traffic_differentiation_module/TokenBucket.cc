//
// Created by Zeinab Shmeis on 16.04.20.
//

#include "TokenBucket.h"

NS_LOG_COMPONENT_DEFINE("TokenBucket");

ATTRIBUTE_HELPER_CPP(TokenBucket);

TokenBucket::TokenBucket(uint8_t tos_id, uint32_t burst, DataRate rate) {
    _tos_id = tos_id;
    _burst = burst;
    _rate = rate;

    // Token Buckets are full at the beginning.
    _btokens = _burst;
    _ptokens = _mtu;
    _timeCheckPoint = Seconds (0);
}

TokenBucket::TokenBucket(uint8_t tos_id, uint32_t burst, DataRate rate, uint32_t mtu, DataRate peakRate) {
    _tos_id = tos_id;
    _burst = burst;
    _rate = rate;
    _mtu = mtu;
    _peakRate = peakRate;

    // Token Buckets are full at the beginning.
    _btokens = _burst;
    _ptokens = _mtu;
    _timeCheckPoint = Seconds (0);
}

TokenBucket::TokenBucket(void (*fun)(TokenBucket)) {
    // do nothing
}

TokenBucket::TokenBucket() {
    // do nothing
}

uint8_t TokenBucket::GetTosId() const {
    return _tos_id;
}

bool TokenBucket::ConsumeTokens(uint32_t nbTokens) {
    int64_t btoks = 0;
    int64_t ptoks = 0;
    Time now = Simulator::Now ();

    double delta = (now  - _timeCheckPoint).GetSeconds ();
    NS_LOG_LOGIC ("Time Difference delta " << delta);

    if (_peakRate > DataRate ("0bps")) {
        ptoks =  _ptokens + round (delta * (_peakRate.GetBitRate () / 8));
        if (ptoks > _mtu) {
            ptoks = _mtu;
        }
        NS_LOG_LOGIC ("Number of ptokens we can consume " << ptoks);
        NS_LOG_LOGIC ("Required to dequeue next packet " << nbTokens);
        ptoks -= nbTokens;
    }

    btoks = _btokens + round (delta * (_rate.GetBitRate () / 8));
    if (btoks > _burst) {
        btoks = _burst;
    }

//    cout << "we have " << btoks << " available" << endl;
//    cout << "we need " << nbTokens << endl;
//    cout << "At time " << now.GetSeconds() << " sec we have " << (_btokens / 125000.0) << " Mb tokens" << endl;

    NS_LOG_LOGIC ("Number of btokens we can consume " << btoks);
    NS_LOG_LOGIC ("Required to dequeue next packet " << nbTokens);
    btoks -= nbTokens;

    if ((btoks|ptoks) >= 0) { // we have enough token
        _timeCheckPoint = now;
        _btokens = btoks;
        _ptokens = ptoks;

        NS_LOG_LOGIC (_btokens << " btokens and " << _ptokens << " ptokens after " << nbTokens << " consumption");
        return true;
    }

    return false;
}

Time TokenBucket::CalculateDelay(uint32_t nbTokens) {
    return std::max(_rate.CalculateBytesTxTime(nbTokens - _btokens),
            _peakRate.CalculateBytesTxTime (nbTokens -_ptokens));
}

string TokenBucket::toString() const{
    std::ostringstream strout;
    strout << "TB [tos = " << unsigned(_tos_id) << ", burst = " << _burst << "B, rate = " << _rate
                << ", mtu = " << _mtu << ", peakRate = " << _peakRate << "]";
    return strout.str();
}

int TokenBucket::operator==(const TokenBucket &object) const {
    return this->_tos_id == object._tos_id && this->_burst == object._burst && this->_rate == object._rate &&
            this->_mtu == object._mtu && this->_peakRate == object._peakRate;
}

uint32_t TokenBucket::GetBurst() const {
    return _burst;
}

uint32_t TokenBucket::GetMtu() const {
    return _mtu;
}

const DataRate &TokenBucket::GetRate() const {
    return _rate;
}

const DataRate &TokenBucket::GetPeakRate() const {
    return _peakRate;
}

void TokenBucket::SetMtu(uint32_t mtu) {
    _mtu = mtu;
}

/* For printing of token bucket */
std::ostream &operator << (std::ostream &os, const TokenBucket &tb) {
    os << tb.toString();
    return os;
}

/* Initialize a token bucket from an input stream */
std::istream &operator >> (std::istream &is, TokenBucket &tb)
{
    std::string value;
    is >> value;
    return is;
}
