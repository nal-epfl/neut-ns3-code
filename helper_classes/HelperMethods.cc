//
// Created by nal on 18.01.22.
//

#include "HelperMethods.h"

vector<string> HelperMethods::SplitStr (const string &s, char delim) {
    vector<string> result;
    stringstream ss (s);
    string item;

    while (getline (ss, item, delim)) {
        result.push_back (item);
    }

    return result;
}

uint32_t HelperMethods::GetSubDirCount(string dirPath) {
    auto dirIter = std::filesystem::directory_iterator(dirPath);
    return count_if(
            begin(dirIter),
            end(dirIter),
            [](auto& entry) { return entry.is_regular_file(); }
    );
}