#ifndef _SERVER_OBJECT_HPP_
#define _SERVER_OBJECT_HPP_

#include <string>
#include <vector>

/*
one based id!!
*/

class User {
public:
    User();
    User(std::string _name, std::string _pass, std::vector<int> _list, int _id = 0);
    std::string username;
    /*
    a unique name
    */
    std::string password;
    /*
    a hashed password
    */
    std::vector<int> friend_idlist;
    int user_id;
    /*
    a sequentially unique id, maintain by the database
    */
};

class Message {
public:
    Message();
    Message(int _cid, int _sid, long long _time, std::string _txt, std::string _hash, int _type, int _seqid = 0, int _mid = 0);
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
    int sequence_id;
    /*
    the order of the message in its chatroom (sort by message_id)
    */
    int message_id;
    /*
    a sequentially unique id, maintain by the database
    */
};

class Chatroom {
public:
    Chatroom();
    Chatroom(std::vector<int> _list, int _id = 0);
    std::vector<int> user_idlist;
    int chatroom_id;
    /*
    a sequentially unique id, maintain by the database
    */
};

#endif // _SERVER_OBJECT_HPP_
