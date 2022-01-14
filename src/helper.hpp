#ifndef _HELPER_HPP_
#define _HELPER_HPP_

#include <string>
#include <iostream>

#define _helper_fail3(msg, errornum, exitcode) {\
    std::cerr << "\033[1;37m" << std::string(__FILE__) + ":" + std::to_string(__LINE__) + ":" + "\033[0m" << " ";\
    std::cerr << "\033[1;31m" << "error:" << "\033[0m" << " " << (msg) << std::endl;\
    if (errornum)\
        std::cerr << " Error number: " << errno << std::endl;\
    exit(exitcode);\
}

#define _helper_fail2(msg, errornum) {\
    _helper_fail3(msg, errornum, 1);\
}

#define _helper_fail(msg) {\
    _helper_fail3(msg, 0, 1);\
}

#define _helper_warning2(msg, errornum) {\
    std::cerr << "\033[1;37m" << std::string(__FILE__) + ":" + std::to_string(__LINE__) + ":" + "\033[0m" << " ";\
    std::cerr << "\033[1;35m" << "warning:" << "\033[0m" << " " << (msg) << std::endl;\
    if (errornum)\
        std::cerr << " Error number: " << errno << std::endl;\
}

#define _helper_warning(msg) {\
    _helper_warning2(msg, 0);\
}

#ifdef DEBUG
#define _helper_msg(msg) {\
    std::cerr << "\033[1;37m" << std::string(__FILE__) + ":" + std::to_string(__LINE__) + ":" + "\033[0m" << " ";\
    std::cerr << "\033[1;34m" << "note:" << "\033[0m" << " " << (msg) << std::endl;\
}
#else
#define _helper_msg(msg)
#endif

#define _helper_log(msg) {\
    std::cerr << "\033[1;37m" << std::string(__FILE__) + ":" + std::to_string(__LINE__) + ":" + "\033[0m" << " ";\
    std::cerr << "\033[1;32m" << "log:" << "\033[0m" << " " << (msg) << std::endl;\
}
#else

#endif // _HELPER_HPP_
