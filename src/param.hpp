#include <string>
#include <unistd.h>

const std::string DBNAME                = "chatroom.db";
const int         MAXFD                 = getdtablesize();
const int         BUFSIZE               = 65536;
