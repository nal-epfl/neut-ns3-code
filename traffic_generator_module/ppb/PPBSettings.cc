//
// Created by nal on 07.09.20.
//

#include "PPBSettings.h"

PPBSettings::PPBSettings(const string &burstIntensity, double meanBurstArrivals, double meanBurstTimeLength,
                         double hurstParameter) : _burstIntensity(burstIntensity),
                                                  _meanBurstArrivals(meanBurstArrivals),
                                                  _meanBurstTimeLength(meanBurstTimeLength),
                                                  _hurstParameter(hurstParameter) {}

const string &PPBSettings::GetBurstIntensity() const {
    return _burstIntensity;
}

double PPBSettings::GetMeanBurstArrivals() const {
    return _meanBurstArrivals;
}

double PPBSettings::GetMeanBurstTimeLength() const {
    return _meanBurstTimeLength;
}

double PPBSettings::GetHurstParameter() const {
    return _hurstParameter;
}
