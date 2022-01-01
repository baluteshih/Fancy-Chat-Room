#ifndef _HELPER_HPP_
#define _HELPER_HPP_

void _failed(const std::string &msg, const std::string &name, int line);
void _failed(const std::string &msg, const std::string &name, int line, bool errornum);
void _failed(const std::string &msg, const std::string &name, int line, bool errornum, int exitcode);
void _warning(const std::string &msg, const std::string &name, int line);
void _warning(const std::string &msg, const std::string &name, int line, bool errornum);
void _msg(const std::string &msg, const std::string &name, int line);

#define __INFO__ __FILE__, __LINE__

#endif // _HELPER_HPP_
