#ifndef _HELPER_HPP_
#define _HELPER_HPP_

#include <string>

namespace Helper {
    void fail(const std::string &msg, const std::string &name, int line);
    void fail(const std::string &msg, const std::string &name, int line, bool errornum);
    void fail(const std::string &msg, const std::string &name, int line, bool errornum, int exitcode);
    void warning(const std::string &msg, const std::string &name, int line);
    void warning(const std::string &msg, const std::string &name, int line, bool errornum);
    void msg(const std::string &msg, const std::string &name, int line);
}

#define __INFO__ __FILE__, __LINE__

#endif // _HELPER_HPP_
