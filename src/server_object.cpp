#include "server_object.hpp"

User::User(std::string _name, std::string _pass, std::vector<int> _list, int _id) :
    username(_name),
    password(_pass),
    friend_idlist(_list),
    user_id(_id){}

Message::Message(int _cid, int _sid, long long _time, std::string _txt, std::string _hash, int _type, int _seqid, int _mid) :
    chatroom_id(_cid),
    sender_id(_sid),
    timestamp(_time),
    text(_txt),
    filehash(_hash),
    type(_type),
    sequence_id(_seqid),
    message_id(_mid) {}

Chatroom::Chatroom(std::vector<int> _list, int _id) :
    user_idlist(_list),
    chatroom_id(_id) {}
