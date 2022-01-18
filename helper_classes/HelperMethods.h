//
// Created by nal on 18.01.22.
//

#ifndef WEHE_P_TOMOGRAPHY_HELPERMETHODS_H
#define WEHE_P_TOMOGRAPHY_HELPERMETHODS_H

#include <string>
#include <chrono>
#include <filesystem>
#include <vector>
#include <algorithm>

using namespace std;

class HelperMethods {

    public:
        static vector<string> SplitStr (const string &s, char delim);
        static uint32_t GetSubDirCount(string dirPath);
};


#endif //WEHE_P_TOMOGRAPHY_HELPERMETHODS_H
