//
// Created by nal on 07.09.20.
//

#ifndef WEHE_PLUS_TOMOGRAPHY_PPBSETTINGS_H
#define WEHE_PLUS_TOMOGRAPHY_PPBSETTINGS_H

#include <string>

using namespace std;

class PPBSettings {

private:
    string _burstIntensity;
    double _meanBurstArrivals; // bursts per sec
    double _meanBurstTimeLength; //sec
    double _hurstParameter;

public:
    PPBSettings(const string &burstIntensity, double meanBurstArrivals, double meanBurstTimeLength,
                double hurstParameter);

    [[nodiscard]] const string &GetBurstIntensity() const;
    [[nodiscard]] double GetMeanBurstArrivals() const;
    [[nodiscard]] double GetMeanBurstTimeLength() const;
    [[nodiscard]] double GetHurstParameter() const;

};


#endif //WEHE_PLUS_TOMOGRAPHY_PPBSETTINGS_H
