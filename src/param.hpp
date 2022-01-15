#ifndef _PARAM_HPP_
#define _PARAM_HPP_

#include <string>
#include <unistd.h>

const std::string DBNAME                = "chatroom.db";
const std::string LIST_DELIMITER        = "$";
const std::string CLIENT_PUBLIC_DIR     = "public";
const int         MAXFD                 = getdtablesize();

#endif // _PARAM_HPP_
