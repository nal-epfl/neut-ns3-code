//
// Created by nal on 22.09.20.
//

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <random>

#include "Distribution.h"


DistElt::DistElt(uint32_t value, double pdf, double cdf) : value(value), pdf(pdf), cdf(cdf) {}

Distribution::Distribution() {
}

Distribution::Distribution(string filename) {
    LoadFromFile(filename);
}

void Distribution::LoadFromFile(string filename) {
    ifstream file(filename);
    string line;
    while(getline(file, line)) {
        vector<string> attributes = HelperMethods::SplitStr(line, ',');
        hist.emplace_back(stoi(attributes[0]), stod(attributes[2]), stod(attributes[3]));
    }
    file.close();

    sort(hist.begin(), hist.end(), [](const DistElt& lhs, const DistElt& rhs) {
        if(lhs.cdf < rhs.cdf) return true;
        if(lhs.cdf > rhs.cdf) return false;
        return lhs.value < rhs.value;
    });
}

uint32_t Distribution::GetRandInt() {
    double U = (float) rand() / (RAND_MAX + 1.0);

    for(uint32_t i = 0; i < hist.size() - 1; i++) {
        if(hist[i].cdf <= U && U < hist[i + 1].cdf) {
            return hist[i + 1].value;
        }
    }

    return 0;
}

