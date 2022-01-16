#include "url.hpp"
#include <functional>
std::string urldecode(std::string raw){
    char p[2] = {0};
    int n = (int)raw.size();
    std::string res;
    for (int i = 0; i < n; i++){
        p[0] = p[1] = 0;
        if (raw[i] == '+'){
            res += ' ';
            continue;
        }
        if (raw[i] != '%'){
            res += raw[i];
            continue;
        }
        p[0] = raw[++i];
        p[1] = raw[++i];
        std::function<void(char&)> magic = [&](char& c){
            c = c - 48 - (c >= 'A' ? 7 : 0) - (c >= 'a' ? 32 : 0);
        };
        magic(p[0]); magic(p[1]);
        res += (unsigned char)((p[0] << 4) + p[1]);
    }
    return res;
}
