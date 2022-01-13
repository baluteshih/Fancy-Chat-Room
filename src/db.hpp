#include <string>
#include <vector>

class DataBase {
    /*
    For the objects, all of the items should be well-filled.
    The only exception is creating data. 
    The "id" can be arbitrary since the database will modify it.
    */
    class Table_User {
    public:
        Table_User(){}
        int create_user(User &user);
        /* 
        return 0 if successfully added, and modify user.userid
        return -1 if the username already exists 
        return -2 if other failure
        */
        int login(User &user);
        /*
        return 0 if successfully login
        return -1 if failure
        */
        User get_object(int userid);
        int get_id(std::string username);
    };
    class Table_Message {
    public:
        Table_Message(){}
        int create_message(Message &message);
        /*
        return 0 if successfully added, and modify message.message_id
        return -1 if failure
        */
        Message get_object(int message_id);
    };
    class Table_Chatroom {
    public:
        Table_Chatroom(){}
        int create_chatroom(Chatroom &chatroom);
        /*
        return 0 if successfully added, and modify chatroom.chatroom_id
        return -1 if failure
        */
        Chatroom get_object(int chatroom_id);
    };
public:
    DataBase(){}
    Table_User     table_user;
    Table_Message  table_message;
    Table_Chatroom table_chatroom;
};
