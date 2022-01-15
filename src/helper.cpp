#include "helper.hpp"
#include "param.hpp"

std::string svector_to_string(const std::vector<std::string> &vec) {
    if (vec.empty())
        return "";
    std::string rt = LIST_DELIMITER;
    for (auto s : vec) {
        rt += s;
        rt += LIST_DELIMITER;
    }
    return rt;
}

std::vector<std::string> string_to_svector(const std::string &str) {
    if (str.empty())
        return {};
    std::vector<std::string> rt;
    for (int i = 0; i < int(str.size());)
        if (str.substr(i, LIST_DELIMITER.size()) == LIST_DELIMITER)
            rt.push_back(""), i += LIST_DELIMITER.size();
        else
            rt.back().push_back(str[i]), ++i;
    rt.pop_back();
    return rt;
}
