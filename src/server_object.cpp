#include "server_object.hpp"

User::User() :
    username(""),
    password(""),
    friend_idlist(std::vector<int>()),
    user_id(0) {}
User::User(std::string _name, std::string _pass, std::vector<int> _list, int _id) :
    username(_name),
    password(_pass),
    friend_idlist(_list),
    user_id(_id){}

Message::Message() :
    chatroom_id(0),
    sender_id(0),
    timestamp(0LL),
    text(""),
    filehash(""),
    type(0),
    sequence_id(0),
    message_id(0) {}
Message::Message(int _cid, int _sid, long long _time, std::string _txt, std::string _hash, int _type, int _seqid, int _mid) :
    chatroom_id(_cid),
    sender_id(_sid),
    timestamp(_time),
    text(_txt),
    filehash(_hash),
    type(_type),
    sequence_id(_seqid),
    message_id(_mid) {}

Chatroom::Chatroom() :
    user_idlist(std::vector<int>()),
    chatroom_id(0) {}
Chatroom::Chatroom(std::vector<int> _list, int _id) :
    user_idlist(_list),
    chatroom_id(_id) {}
