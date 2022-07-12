//
// Created by shmeis on 7/12/22.
//

#ifndef SCRATCH_WEHE_P_TOMOGRAPHY_WEHE_P_TOMOGRAPHY_DSCPS2QUEUEBAND_H
#define SCRATCH_WEHE_P_TOMOGRAPHY_WEHE_P_TOMOGRAPHY_DSCPS2QUEUEBAND_H

#include "ns3/core-module.h"

#include "../helper_classes/HelperMethods.h"

using namespace ns3;
using namespace std;
using namespace helper_methods;

class Dscps2QueueBand {
private:
    uint16_t _band = -1;
    vector<uint8_t> _dscps = {};

public:
    Dscps2QueueBand(): _band(-1), _dscps({}) {};
    explicit Dscps2QueueBand(uint16_t band, vector<uint8_t> dscps): _band(band), _dscps(std::move(dscps)) {};

    void SetBand(uint16_t band) { _band = band; };
    [[nodiscard]] uint16_t GetBand() const { return _band; };
    void SetDscps(vector<uint8_t> dscps) { _dscps = std::move(dscps); };
    [[nodiscard]] vector<uint8_t> GetDscps() const { return _dscps; };

    bool CheckIfBelongsTo(uint8_t dscp) {
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

#endif //SCRATCH_WEHE_P_TOMOGRAPHY_WEHE_P_TOMOGRAPHY_DSCPS2QUEUEBAND_H
