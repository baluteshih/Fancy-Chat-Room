#include <string>
#include "server_object.hpp"

class DataBase {
    /*
    For the objects, all of the items should be well-filled.
    The only exception is creating data. 
    The "id" can be arbitrary since the database will modify it.
    
    about return value:
    0 always means successful
    Return <0 value when failure. Different values represent different kinds of errors 
    */
    class Table_User {
    public:
        Table_User(){}
        int create_user(User &user);
        /* 
        return 0 if successfully added, and modify user.userid
        return -1 if the username already exists 
        */
        int update_password(int userid, std::string password);
        int add_friend(int userid, int friendid);
        /*
        retunr -1 if the user already has a friend with id "friendid"
        */
        int delete_friend(int user_id, int friendid);
        /*
        retunr -1 if there is no friend with id "friendid"
        */
        User get_object(int user_id);
        int get_id(std::string username);
    };
    class Table_Message {
    public:
        Table_Message(){}
        int create_message(Message &message);
        /*
        return 0 if successfully added, and modify message.message_id
        */
        Message get_object(int message_id);
        int get_id(int chatroom_id, int sequence_id);
        std::vector<Message> query_range_fixed_chatroom(int chatroom_id, int lower, int upper);
        /*
        return vector of Message which ids between [lower, upper]
        */
        int maxseqid_of_chatroom(int chatroom_id);
    };
    class Table_Chatroom {
    public:
        Table_Chatroom(){}
        int create_chatroom(Chatroom &chatroom);
        /*
        return 0 if successfully added, and modify chatroom.chatroom_id
        */
        int add_user(int chatroom_id, int user_id);
        /*
        retunr -1 if the user already in the chatroom
        */
        int delete_user(int chatroom_id, int user_id);
        /*
        retunr -1 if the user not in the chatroom
        */
        Chatroom get_object(int chatroom_id);
    };
public:
    DataBase(){}
    Table_User     table_user;
    Table_Message  table_message;
    Table_Chatroom table_chatroom;
};
