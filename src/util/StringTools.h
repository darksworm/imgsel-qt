#pragma once

#include <string>
#include <vector>
#include <cstring>
#include <cstdio>
#include <algorithm>

class StringTools {
public:
    static bool isNumber(const std::string &s) {
        return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
    }

    static std::vector<int> splitIntoInts(const std::string s, const std::string sep) {
        auto split = StringTools::split(s, sep);
        std::vector<int> arr;

        for (auto &item : split) {
            if(isNumber(item)) {
                arr.emplace_back(stoi(item));
            }
        }

        return arr;
    }

    static std::vector<std::string> split(const std::string str, const std::string sep) {
        char *cstr = const_cast<char *>(str.c_str());
        char *current;

        std::vector<std::string> arr;
        current = strtok(cstr, sep.c_str());

        while (current != nullptr) {
            arr.emplace_back(current);
            current = strtok(nullptr, sep.c_str());
        }

        return arr;
    }
};