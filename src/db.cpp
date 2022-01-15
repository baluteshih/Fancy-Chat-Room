#include "helper.hpp"
#include "param.hpp"
#include "db.hpp"

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
   return 0;
}
DataBase::DataBase() : table_user(*this), table_message(*this), table_chatroom(*this){
    int res = sqlite3_open(DBNAME.c_str(), &this->db);
    char *sqlcmd;
    char *zErrMsg = 0;
    if (res != SQLITE_OK){
        _helper_fail("Fail to open database.");
    }
    
    // create table_user
    sqlcmd = "CREATE TABLE IF NOT EXISTS table_user ("\
                 "username TEXT NOT NULL,"\
                 "password TEXT NOT NULL,"\
                 "friend_idlist TEXT NOT NULL,"\
                 "user_id INT NOT NULL UNIQUE"\
             ");";
    res = sqlite3_exec(db, sqlcmd, callback, 0, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to create table_user.");
    }

    // create table_message
    sqlcmd = "CREATE TABLE IF NOT EXISTS table_message ("\
                 "chatroom_id INT NOT NULL,"\
                 "timestamp INT NOT NULL,"\
                 "text TEXT,"\
                 "filehash TEXT NOT NULL,"\
                 "type INT NOT NULL,"\
                 "sequence_id INT NOT NULL,"\
                 "message_id INT NOT NULL,"\
                 "user_id INT NOT NULL UNIQUE"\
             ");";
    res = sqlite3_exec(db, sqlcmd, callback, 0, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to create table_message.");
    }

    // create table_chatroom
    sqlcmd = "CREATE TABLE IF NOT EXISTS table_chatroom ("\
                 "user_idlist TEXT NOT NULL,"\
                 "chatroom_id INT NOT NULL,"\
             ");";
    res = sqlite3_exec(db, sqlcmd, callback, 0, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to create table_user.");
    }
}
DataBase::~DataBase(){
    sqlite3_close(db);
}

// table user
// DataBase::Table_User::Table_User(){}
DataBase::Table_User::Table_User(DataBase &x) : parent(x) {}
static int callback_exist(void *exist, int argc, char **argv, char **azColName){
    int *b = (int*)exist;
    *b = atoi(argv[0]);
    return 0;
}
static int callback_countrow(void *count, int argc, char **argv, char **azColName){
    int *c = (int*)count;
    *c = atoi(argv[0]);
    return 0;
}
int DataBase::Table_User::create_user(User &user){
    int res;
    char *zErrMsg = 0;
    std::string sqlcmd;

    int exist = 0;
    sqlcmd = "SELECT EXISTS (SELECT 1 FROM table_user WHERE username=\"" + user.username + "\");";
    res = sqlite3_exec(db, sqlcmd.c_str(), callback_exist, &exist, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to check if the username exists.");
    }
    if (exist)
        return -1;

    int count = 0;
    res = sqlite3_exec(db, "select count(*) from table_user", 
                       callback_countrow, &count, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to find the current number of users.");
    }

    user.user_id = count + 1;

    std::string friend_idlist_in_str = "$";
    for (auto& idx : user.friend_idlist){
        friend_idlist_in_str += std::to_string(idx) + "$";
    }

    sqlcmd = "INSERT INTO table_user (username, password, friend_idlist, user_id) VALUES (";
    sqlcmd += "\"" + user.username + "\", ";
    sqlcmd += "\"" + user.password + "\", ";
    sqlcmd += "\"" + friend_idlist_in_str + "\", ";
    sqlcmd += "\"" + std::to_string(user.user_id) + "\");";

    res = sqlite3_exec(db, sqlcmd.c_str(), callback, 0, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to add user.");
    }
    return 0;
}
int DataBase::Table_User::update_password(int userid, std::string password){
    
}
int DataBase::Table_User::add_friend(int userid, int friendid){
    
}
int DataBase::Table_User::delete_friend(int user_id, int friendid){
    
}
User get_object(int user_id){
    
}
int get_id(std::string username){
    
}

// table message
// DataBase::Table_Message::Table_Message(){}
DataBase::Table_Message::Table_Message(DataBase &x) : parent(x) {}
int DataBase::Table_Message::create_message(Message &message){
    
}
Message DataBase::Table_Message::get_object(int message_id){
    
}
int DataBase::Table_Message::get_id(int chatroom_id, int sequence_id){
    
}
std::vector<Message> DataBase::Table_Message::query_range_fixed_chatroom
(int chatroom_id, int lower, int upper){
    
}
int DataBase::Table_Message::maxseqid_of_chatroom(int chatroom_id){
    
}

// table chatroom
// DataBase::Table_Chatroom::Table_Chatroom(){}
DataBase::Table_Chatroom::Table_Chatroom(DataBase &x) : parent(x) {}
int DataBase::Table_Chatroom::create_chatroom(Chatroom &chatroom){
    
}
int DataBase::Table_Chatroom::add_user(int chatroom_id, int user_id){
    
}
int DataBase::Table_Chatroom::delete_user(int chatroom, int user_id){
    
}
Chatroom DataBase::Table_Chatroom::get_object(int chatroom_id){
    
}
