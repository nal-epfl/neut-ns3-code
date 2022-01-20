//
// Created by nal on 22.09.20.
//

#ifndef WEHE_PLUS_TOMOGRAPHY_DISTRIBUTION_H
#define WEHE_PLUS_TOMOGRAPHY_DISTRIBUTION_H

#include <vector>

#include "../../helper_classes/HelperMethods.h"

using namespace std;

struct DistElt {
    uint32_t value;
    double pdf;
    double cdf;

    DistElt(uint32_t value, double pdf, double cdf);
};


class Distribution {

private:
    vector<DistElt> hist;

public:
    Distribution();
    explicit Distribution(string filename);

    void LoadFromFile(string filename);

    uint32_t GetRandInt();

};


#endif //WEHE_PLUS_TOMOGRAPHY_DISTRIBUTION_H
