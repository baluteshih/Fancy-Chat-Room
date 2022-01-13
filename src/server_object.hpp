#include <string>

class User {
    User(std::string _name, std::string _pass, int _id){}
    std::string username;
    /*
    a unique name
    */
    std::string password;
    /*
    a hashed password
    */
    int userid;
    /*
    a unique id, maintain by the database
    */
};

class Message {
    Message(int _cid, int _sid, long long _time, std::string _txt, std::string _hash, int _type, int _mid){}
    int chatroom_id;
    int sender_id;
    long long timestamp;
    std::string text;
    /* 
    type 0: string message itself
    type 1: file's name
    */
    std::string filehash;
    /*
    hash value of the file
    */
    int type;
    /* 
    type 0: string message
    type 1: file
    */
    int message_id;
    /*
    a unique id, maintain by the database
    */
};

class Chatroom {
    Chatroom(std::vector<User> _list, int _id = 0){}
    std::vector<User> userlist;
    int chatroom_id;
    /*
    a unique id, maintain by the database
    */
};
