#include <algorithm>
#include "helper.hpp"
#include "param.hpp"
#include "db.hpp"

std::vector<int> string_to_vector(std::string s){
    std::vector<int> res;
    const char delim = LIST_DELIMITER[0];
    int a = 0;
    for (int i = 1; i < (int)s.size(); i++){
        if (s[i] == delim){
            res.push_back(a);
            a = 0;
        }
        else {
            a = a * 10 + (s[i] - '0');
        }
    }
    return res;
}
std::string vector_to_string(std::vector<int> v){
    std::string res = LIST_DELIMITER;
    for (int x : v)
        res += std::to_string(x) + LIST_DELIMITER;
    return res;
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
   return 0;
}
DataBase::DataBase() : table_user(*this), table_message(*this), table_chatroom(*this){
    int res = sqlite3_open(DBNAME.c_str(), &this->db);
    std::string sqlcmd;
    char *zErrMsg = 0;
    if (res != SQLITE_OK){
        _helper_fail("Fail to open database.");
    }
    
    // create table_user
    sqlcmd = std::string("") +
             "CREATE TABLE IF NOT EXISTS table_user (" + 
                 "username TEXT NOT NULL," + 
                 "password TEXT NOT NULL," + 
                 "friend_idlist TEXT NOT NULL," + 
                 "user_id INT NOT NULL UNIQUE" +
             ");";
    res = sqlite3_exec(db, sqlcmd.c_str(), callback, 0, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to create table_user.");
    }

    // create table_message
    sqlcmd = std::string("") + 
             "CREATE TABLE IF NOT EXISTS table_message (" + 
                 "chatroom_id INT NOT NULL," + 
                 "sender_id INT NOT NULL," +
                 "timestamp INT NOT NULL," +
                 "text TEXT," +
                 "filehash TEXT NOT NULL," +
                 "type INT NOT NULL," +
                 "sequence_id INT NOT NULL," +
                 "message_id INT NOT NULL" +
             ");";
    res = sqlite3_exec(db, sqlcmd.c_str(), callback, 0, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to create table_message.");
    }

    // create table_chatroom
    sqlcmd = std::string("") +
             "CREATE TABLE IF NOT EXISTS table_chatroom (" +
                 "user_idlist TEXT NOT NULL," +
                 "chatroom_id INT NOT NULL UNIQUE" +
             ");";
    res = sqlite3_exec(db, sqlcmd.c_str(), callback, 0, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to create table_chatroom.");
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
    res = sqlite3_exec(parent.db, sqlcmd.c_str(), callback_exist, &exist, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to check if the username exists.");
    }
    if (exist)
        return -1;

    int count = 0;
    res = sqlite3_exec(parent.db, "select count(*) from table_user", 
                       callback_countrow, &count, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to find the current number of users.");
    }

    user.user_id = count + 1;

    std::string friend_idlist_in_str = vector_to_string(user.friend_idlist);

    sqlcmd = "INSERT INTO table_user (username, password, friend_idlist, user_id) VALUES (";
    sqlcmd += "\"" + user.username + "\", ";
    sqlcmd += "\"" + user.password + "\", ";
    sqlcmd += "\"" + friend_idlist_in_str + "\", ";
    sqlcmd += "\"" + std::to_string(user.user_id) + "\");";

    res = sqlite3_exec(parent.db, sqlcmd.c_str(), callback, 0, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to add user.");
    }
    return 0;
}
int DataBase::Table_User::update_password(int user_id, std::string password){
    int res;
    char *zErrMsg;
    std::string sqlcmd;

    sqlcmd = "UPDATE table_user SET password = \"" + password + "\" WHERE user_id = " + std::to_string(user_id) + ";";

    res = sqlite3_exec(parent.db, sqlcmd.c_str(), callback, 0, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to update password.");
    }

    return 0;
}
static int callback_getuser(void* ref, int argc, char **argv, char **azColName){
    User *user = (User*)ref;
    for (int i = 0; i < argc; i++){
        std::string argName(azColName[i]), argVal(argv[i]);
        if (argName == "username")
            user->username = argVal;
        if (argName == "password")
            user->password = argVal;
        if (argName == "friend_idlist")
            user->friend_idlist = string_to_vector(argVal);
        if (argName == "user_id")
            user->user_id = stoi(argVal);
    }
    return 0;
}
User DataBase::Table_User::get_object(int user_id){
    int res;
    char *zErrMsg;
    std::string sqlcmd;

    sqlcmd = "SELECT * FROM table_user WHERE user_id = " + std::to_string(user_id) + ";";

    User user;
    res = sqlite3_exec(parent.db, sqlcmd.c_str(), callback_getuser, &user, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        return User();
        _helper_fail("Fail to get user.");
    }
    return user;
}
int DataBase::Table_User::get_id(std::string username){
    int res;
    char *zErrMsg;
    std::string sqlcmd;

    sqlcmd = "SELECT * FROM table_user WHERE username = \"" + username + "\";";

    User user;
    res = sqlite3_exec(parent.db, sqlcmd.c_str(), callback_getuser, &user, &zErrMsg);
    
    _helper_msg(username);
    _helper_msg(zErrMsg);

    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        return -1;
        _helper_fail("Fail to get user.");
    }
    return user.user_id ? user.user_id : -1; // if user_id == 0, return -1
}
int DataBase::Table_User::add_friend(int user_id, int friendid){
    User user = this->get_object(user_id);
    int existFlag = 0;
    for (int f : user.friend_idlist){
        if (f == friendid){
            existFlag = 1;
            break;
        }
    }
    if (existFlag){
        return -1;
    }

    std::vector<int> newFriendList = user.friend_idlist;
    newFriendList.push_back(friendid);
    std::string str_list = vector_to_string(newFriendList);
    
    int res;
    char *zErrMsg;
    std::string sqlcmd;

    sqlcmd = "UPDATE table_user SET friend_idlist = \"" + str_list + "\" WHERE user_id = " + std::to_string(user_id) + ";";

    res = sqlite3_exec(parent.db, sqlcmd.c_str(), callback, 0, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to add a friend.");
    }

    return 0;
}
int DataBase::Table_User::delete_friend(int user_id, int friendid){
    User user = this->get_object(user_id);
    int existFlag = 0;
    std::vector<int> newFriendList;
    for (int f : user.friend_idlist){
        if (f == friendid){
            existFlag = 1;
        }
        else {
            newFriendList.push_back(f);
        }
    }
    if (!existFlag){
        return -1;
    }

    std::string str_list = vector_to_string(newFriendList);
    
    int res;
    char *zErrMsg;
    std::string sqlcmd;

    sqlcmd = "UPDATE table_user SET friend_idlist = \"" + str_list + "\" WHERE user_id = " + std::to_string(user_id) + ";";

    res = sqlite3_exec(parent.db, sqlcmd.c_str(), callback, 0, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to remove a friend.");
    }

    return 0;
}

// table message
// DataBase::Table_Message::Table_Message(){}
DataBase::Table_Message::Table_Message(DataBase &x) : parent(x) {}
int DataBase::Table_Message::create_message(Message &message){
    int res;
    char *zErrMsg = 0;
    std::string sqlcmd;

    int count = 0;
    res = sqlite3_exec(parent.db, "select count(*) from table_message", 
                       callback_countrow, &count, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to find the current number of messages.");
    }

    message.message_id = count + 1;

    sqlcmd = "INSERT INTO table_message (chatroom_id, sender_id, timestamp, text, filehash, type, sequence_id, message_id) VALUES (";
    sqlcmd += std::to_string(message.chatroom_id) + ", ";
    sqlcmd += std::to_string(message.sender_id) + ", ";
    sqlcmd += std::to_string(message.timestamp) + ", ";
    sqlcmd += "\"" + message.text + "\", ";
    sqlcmd += "\"" + message.filehash + "\", ";
    sqlcmd += std::to_string(message.type) + ", ";
    sqlcmd += std::to_string(message.sequence_id) + ", ";
    sqlcmd += std::to_string(message.message_id) + ");";

    res = sqlite3_exec(parent.db, sqlcmd.c_str(), callback, 0, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to add message.");
    }
    return 0;
}
static int callback_getmessage(void* ref, int argc, char **argv, char **azColName){
    Message *message = (Message*)ref;
    for (int i = 0; i < argc; i++){
        std::string argName(azColName[i]), argVal(argv[i]);
        if (argName == "chatroom_id")
            message->chatroom_id = stoi(argVal);
        if (argName == "sender_id")
            message->sender_id = stoi(argVal);
        if (argName == "timestamp")
            message->timestamp = stoll(argVal);
        if (argName == "text")
            message->text = argVal;
        if (argName == "filehash")
            message->filehash = argVal;
        if (argName == "type")
            message->type = stoi(argVal);
        if (argName == "sequence_id")
            message->sequence_id = stoi(argVal);
        if (argName == "message_id")
            message->message_id = stoi(argVal);
    }
    return 0;
}
Message DataBase::Table_Message::get_object(int message_id){
    int res;
    char *zErrMsg;
    std::string sqlcmd;

    sqlcmd = "SELECT * FROM table_message WHERE message_id = " + std::to_string(message_id) + ";";

    Message message;
    res = sqlite3_exec(parent.db, sqlcmd.c_str(), callback_getmessage, &message, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        return Message();
        _helper_fail("Fail to get message.");
    }
    return message;
}
int DataBase::Table_Message::get_id(int chatroom_id, int sequence_id){
    int res;
    char *zErrMsg;
    std::string sqlcmd;

    sqlcmd = "SELECT * FROM table_message WHERE chatroom_id = " + std::to_string(chatroom_id) + " AND sequence_id = "  + std::to_string(sequence_id) +";";

    Message message;
    res = sqlite3_exec(parent.db, sqlcmd.c_str(), callback_getmessage, &message, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        return -1;
        _helper_fail("Fail to get message.");
    }
    return message.message_id ? message.message_id : -1;
}
std::vector<Message> DataBase::Table_Message::query_range_fixed_chatroom
(int chatroom_id, int lower, int upper){
    lower = max(1, lower);
    if (lower > upper)
        return vector<Message>();

    std::vector<Message> ans;
    for (int i = lower; i <= upper; i++)
        ans.push_back(this->get_object(this->get_id(chatroom_id, i)));

    return ans;
}
static int callback_findMaxSeq(void *ref, int argc, char **argv, char **azColName){
    int *mx = (int*)ref;
    for (int i = 0; i < argc; i++){
        std::string argName(azColName[i]), argVal(argv[i]);
        if (argName == "sequence_id")
            *mx = (stoi(argVal) > *mx ? stoi(argVal) : *mx);
    }
    return 0;
}
int DataBase::Table_Message::maxseqid_of_chatroom(int chatroom_id){
    int res;
    char *zErrMsg;
    std::string sqlcmd;

    sqlcmd = "SELECT * FROM table_message WHERE chatroom_id = " + std::to_string(chatroom_id) + ";";

    int ans = -1;
    res = sqlite3_exec(parent.db, sqlcmd.c_str(), callback_findMaxSeq, &ans, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to get max id.");
    }
    return ans;
}

// table chatroom
// DataBase::Table_Chatroom::Table_Chatroom(){}
DataBase::Table_Chatroom::Table_Chatroom(DataBase &x) : parent(x) {}
int DataBase::Table_Chatroom::create_chatroom(Chatroom &chatroom){
    int res;
    char *zErrMsg = 0;
    std::string sqlcmd;

    int count = 0;
    res = sqlite3_exec(parent.db, "select count(*) from table_chatroom", 
                       callback_countrow, &count, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to find the current number of chatrooms.");
    }

    chatroom.chatroom_id = count + 1;

    std::string user_idlist_in_str = vector_to_string(chatroom.user_idlist);

    sqlcmd = "INSERT INTO table_chatroom (user_idlist, chatroom_id) VALUES (";
    sqlcmd += "\"" + user_idlist_in_str + "\", ";
    sqlcmd += std::to_string(chatroom.chatroom_id) + ");";

    res = sqlite3_exec(parent.db, sqlcmd.c_str(), callback, 0, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to add chatroom.");
    }
    return 0;
}
static int callback_getchatroom(void* ref, int argc, char **argv, char **azColName){
    Chatroom *chatroom = (Chatroom*)ref;
    for (int i = 0; i < argc; i++){
        std::string argName(azColName[i]), argVal(argv[i]);
        if (argName == "user_idlist")
            chatroom->user_idlist = string_to_vector(argVal);
        if (argName == "chatroom_id")
            chatroom->chatroom_id = stoi(argVal);
    }
    return 0;
}
Chatroom DataBase::Table_Chatroom::get_object(int chatroom_id){
    int res;
    char *zErrMsg;
    std::string sqlcmd;

    sqlcmd = "SELECT * FROM table_chatroom WHERE chatroom_id = " + std::to_string(chatroom_id) + ";";

    Chatroom chatroom;
    res = sqlite3_exec(parent.db, sqlcmd.c_str(), callback_getchatroom, &chatroom, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        return Chatroom();
        _helper_fail("Fail to get chatroom.");
    }
    return chatroom;
}
int DataBase::Table_Chatroom::add_user(int chatroom_id, int user_id){
    Chatroom chatroom = this->get_object(chatroom_id);
    int existFlag = 0;
    for (int member : chatroom.user_idlist){
        if (member == user_id){
            existFlag = 1;
            break;
        }
    }
    if (existFlag){
        return -1;
    }

    std::vector<int> newList = chatroom.user_idlist;
    newList.push_back(user_id);
    std::string str_list = vector_to_string(newList);
    
    int res;
    char *zErrMsg;
    std::string sqlcmd;

    sqlcmd = "UPDATE table_chatroom SET user_idlist = \"" + str_list + "\" WHERE chatroom_id = " + std::to_string(chatroom_id) + ";";

    res = sqlite3_exec(parent.db, sqlcmd.c_str(), callback, 0, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to add an user in a chatroom.");
    }

    return 0;
}
int DataBase::Table_Chatroom::delete_user(int chatroom_id, int user_id){
    Chatroom chatroom = this->get_object(chatroom_id);
    int existFlag = 0;
    std::vector<int> newList;
    for (int member : chatroom.user_idlist){
        if (member == user_id){
            existFlag = 1;
        }
        else {
            newList.push_back(member);
        }
    }
    if (!existFlag){
        return -1;
    }

    std::string str_list = vector_to_string(newList);
    
    int res;
    char *zErrMsg;
    std::string sqlcmd;

    sqlcmd = "UPDATE table_chatroom SET user_idlist = \"" + str_list + "\" WHERE chatroom_id = " + std::to_string(chatroom_id) + ";";

    res = sqlite3_exec(parent.db, sqlcmd.c_str(), callback, 0, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to delete an user in a chatroom.");
    }

    return 0;
}
std::vector<Chatroom> DataBase::Table_Chatroom::get_chatroom_list(int user_id){
    int res;
    char *zErrMsg = 0;
    std::string sqlcmd;

    int count = 0;
    res = sqlite3_exec(parent.db, "select count(*) from table_chatroom", 
                       callback_countrow, &count, &zErrMsg);
    if (res != SQLITE_OK){
        sqlite3_free(zErrMsg);
        _helper_fail("Fail to find the current number of chatrooms.");
    }
    
    std::vector<Chatroom> ans;
    for (int i = 1; i <= count; i++){
        Chatroom chatroom = this->get_object(i);
        if (std::find(chatroom.user_idlist.begin(), chatroom.user_idlist.end(), user_id) != chatroom.user_idlist.end()){
            ans.push_back(chatroom);
        }
    }

    return ans;
}
