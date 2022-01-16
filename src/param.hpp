#ifndef _PARAM_HPP_
#define _PARAM_HPP_

#include <string>
#include <unistd.h>

const std::string DBNAME                = "chatroom.db";
const std::string LIST_DELIMITER        = "$";
const std::string SERVER_PUBLIC_DIR     = "public_server";
const std::string CLIENT_PUBLIC_DIR     = "public_client";
const std::string TEMP_TEMPLATE         = "tmp/XXXXXX";
const std::string CONSOLE_AGENT         = "Fancy-Console/1.0";
const std::string NO_FRIEND_QQ          = "Unfortunately, you don't have a friend :(";
const std::string NO_CHATROOM_QQ        = "Unfortunately, you don't have a chatroom :(";
const int         MAXFD                 = getdtablesize();
const int         IMAGE_WIDTH           = 100;
const int         IMAGE_HEIGHT          = 120;

#endif // _PARAM_HPP_
