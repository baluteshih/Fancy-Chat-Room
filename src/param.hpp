#ifndef _PARAM_HPP_
#define _PARAM_HPP_

#include <string>
#include <unistd.h>

const std::string DBNAME                = "chatroom.db";
const std::string LIST_DELIMITER        = "$";
const std::string SERVER_PUBLIC_DIR     = "public_server";
const std::string CLIENT_PUBLIC_DIR     = "public_client";
const std::string TEMP_TEMPLATE         = "tmp/XXXXXX";
const int         MAXFD                 = getdtablesize();

#endif // _PARAM_HPP_
