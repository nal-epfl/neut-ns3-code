//
// Created by shmeis on 7/12/22.
//

#ifndef SCRATCH_WEHE_P_TOMOGRAPHY_WEHE_P_TOMOGRAPHY_DSCPS2QUEUEBAND_H
#define SCRATCH_WEHE_P_TOMOGRAPHY_WEHE_P_TOMOGRAPHY_DSCPS2QUEUEBAND_H

#include <utility>

#include "ns3/core-module.h"

#include "../helper_classes/HelperMethods.h"

using namespace ns3;
using namespace std;
using namespace helper_methods;

class Dscps2QueueBand {
protected:
    uint16_t _band = -1;
    vector<uint8_t> _dscps = {};

public:
    Dscps2QueueBand(): _band(-1), _dscps({}) {};
    explicit Dscps2QueueBand(uint16_t band, vector<uint8_t> dscps): _band(band), _dscps(std::move(dscps)) {};

    void SetBand(uint16_t band) { _band = band; };
    [[nodiscard]] uint16_t GetBand() const { return _band; };
    void SetDscps(vector<uint8_t> dscps) { _dscps = std::move(dscps); };
    [[nodiscard]] vector<uint8_t> GetDscps() const { return _dscps; };

    virtual bool CheckIfBelongsTo(uint8_t dscp) {
        return (std::find(_dscps.begin(), _dscps.end(), dscp) != _dscps.end());
    };

    friend ostream &operator<<(ostream &os, const Dscps2QueueBand &dscps2band);
    friend istream &operator>>(istream &is, Dscps2QueueBand &dscps2band);
};

inline ostream& operator<< (ostream& os, const Dscps2QueueBand &dscps2band) {
    os << dscps2band.GetBand() << ",";
    vector<uint8_t> dscps = dscps2band.GetDscps();
    std::copy (dscps.begin (), dscps.end () - 1, std::ostream_iterator<uint8_t>(os, ","));
    os << dscps.back ();
    return os;
}

inline istream& operator>> (istream& is, Dscps2QueueBand &dscps2band) {
    string str; is >> str;
    vector<string> vals = SplitStr(str, ',');

    dscps2band.SetBand(stoi(vals[0]));
    vector<uint8_t> dscps;
    for (uint32_t i = 1; i < vals.size(); i++) { dscps.push_back(stoi(vals[i])); }
    dscps2band.SetDscps(dscps);

    return is;
}

/** A special case of the Dscps2QueueBand where background traffic is classified only during certain time intervals **/
struct TimeInterval {
    Time start, end;
    [[nodiscard]] bool includes(const Time& time) const { return (time > start) && (time < end); };
};
class TimeBasedDscps2QueueBand: public Dscps2QueueBand {
private:
    vector<TimeInterval> _throttleWithBackIntervals = {
            {Seconds(2.64), Seconds(2.88)},
            {Seconds(3.60), Seconds(4.48)},
            {Seconds(4.96), Seconds(5.04)},
            {Seconds(14.48), Seconds(14.72)},
            {Seconds(15.04), Seconds(15.44)},
    };

public:
    TimeBasedDscps2QueueBand(uint16_t band, vector<uint8_t> dscps, const string& throttling_times): Dscps2QueueBand(band, std::move(dscps)) {
        ifstream traceInput(throttling_times);
        string line;
        while(getline(traceInput, line)) {
            vector<string> times = helper_methods::SplitStr(line, ',');
            Time interval_start = Seconds((stoi(times[0])));
            Time interval_end = Seconds((stoi(times[1])));
            _throttleWithBackIntervals.push_back({interval_start, interval_end});
        }
        traceInput.close();
    };

    using Dscps2QueueBand::CheckIfBelongsTo;
    bool CheckIfBelongsTo(uint8_t dscp) override {
        Time currTime = ns3::Now() - Seconds(10);
        for(const auto& interval: _throttleWithBackIntervals) {
            if (interval.includes(currTime))  {
                return (std::find(_dscps.begin(), _dscps.end(), dscp) != _dscps.end());
            }
        }
        return dscp == _dscps[0];
    };
};

#endif //SCRATCH_WEHE_P_TOMOGRAPHY_WEHE_P_TOMOGRAPHY_DSCPS2QUEUEBAND_H
