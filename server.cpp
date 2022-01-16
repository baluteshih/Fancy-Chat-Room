#include "helper.hpp"
#include "http.hpp"
#include "db.hpp"
#include "param.hpp"
#include "main_thread.hpp"
#include "server_object.hpp"
#include "file.hpp"
#include "crypto.hpp"
#include "html.hpp"
#include "SHA256.h"
#include <iostream>
#include <unistd.h>
#include <utility>
#include <sstream>
#include <string>
#include <cctype>
#include <unordered_map>
#include <chrono>
#include <algorithm>
#include <cstdio>

Main_Thread main_thread;
DataBase db;

namespace Handler {
    std::unordered_map<std::string, std::string> data_parser(std::string dataraw) {
        std::unordered_map<std::string, std::string> rt;
        std::string attr, value;
        int state = 0;
        for (char c : dataraw) {
            if (c == '=') {
                state = 1;
                value.clear();
            }
            else if (c == '&') {
                state = 0;
                if (!attr.empty())
                    rt[attr] = value;
                attr.clear();
            }
            else if (state == 0)
                attr.push_back(c);
            else
                value.push_back(c);
        }
        if (!attr.empty())
            rt[attr] = value;
        return rt;
    }
    std::pair<int, std::string> cookie_parser(std::string cookie) {
        std::pair<int, std::string> rt;
        for (char &c : cookie)
            if (c == '=' || c == '$')
                c = ' ';
        std::stringstream ss(cookie);
        std::string tmp;
        ss >> tmp >> rt.first >> rt.second;
        return rt;
    }
    bool password_format_checker(std::string password) {
        if (int(password.size()) < 6) {
            return 0;
        }
        for (char c : password)
            if (!(std::isalnum(c) || c == '_')) {
                return 0;
            }
        return 1;
    }
    bool cookie_format_checker(std::string cookie) {
        // TODO: content
        // metadata=user_id$password
        return 1;
    }
    User get_user(int user_id) {
        if (user_id <= 0)
            return User();
        return db.table_user.get_object(user_id);
    }
    User get_user(HTTPRequest &req) {
        if (req.header_field.find("Cookie") == req.header_field.end())
            return User();
        std::string cookie = req.header_field["Cookie"];
        auto rt = cookie_parser(cookie);
        return get_user(rt.first); 
    }
    std::string get_username(int user_id) {
        if (user_id <= 0)
            return "";
        return get_user(user_id).username;
    }
    bool check_login(HTTPRequest &req) {
        if (req.header_field.find("Cookie") == req.header_field.end())
            return 0;
        std::string cookie = req.header_field["Cookie"];
        if (!cookie_format_checker(cookie))
            return 0;
        auto rt = cookie_parser(cookie);
        User user = get_user(rt.first);
        if (user.user_id <= 0)
            return 0;
        return rt.second == user.password;
    }
    void try_login(HTTPRequest &req, HTTPResponse &res) {
        auto dataraw = data_parser(req.message_body);
        if (dataraw.find("username") == dataraw.end() || dataraw.find("password") == dataraw.end()) {
            res.set_status(HTTP::Status_Code::NotAcceptable);
            return;
        }
        int user_id = db.table_user.get_id(dataraw["username"]);
        if (user_id <= 0) {
            res.set_file(path_combine(SERVER_PUBLIC_DIR, "Login_files/login_fail.html"));
            return;
        }
        User user = get_user(user_id);
        std::string pass = hash_password(dataraw["password"], user.password.substr(0, 4));
        if (pass != user.password) {
            res.set_file(path_combine(SERVER_PUBLIC_DIR, "Login_files/login_fail.html"));
            return;
        }
        res.set_redirect(res.header_field["Host"] + "/");
        res.header_field["Set-Cookie"] = "metadata=" + std::to_string(user_id) + "$" + pass;
    }
    void try_register(HTTPRequest &req, HTTPResponse &res) {
        auto dataraw = data_parser(req.message_body);
        if (dataraw.find("username") == dataraw.end() || 
            dataraw.find("password") == dataraw.end() || 
            dataraw.find("password2") == dataraw.end()) {
            res.set_status(HTTP::Status_Code::NotAcceptable);
            return;
        }
        if (dataraw["username"].empty()) {
            res.set_file(path_combine(SERVER_PUBLIC_DIR, "Register_files/register_acc_invalid.html"));
            return;
        }
        for (char c : dataraw["username"])
            if (!(std::isalnum(c) || c == '_')) {
                res.set_file(path_combine(SERVER_PUBLIC_DIR, "Register_files/register_acc_invalid.html"));
                return;
            }
        if (!password_format_checker(dataraw["password"])) {
            res.set_file(path_combine(SERVER_PUBLIC_DIR, "Register_files/register_pass_invalid.html"));
            return;
        }
        if (dataraw["password"] != dataraw["password2"]) {
            res.set_file(path_combine(SERVER_PUBLIC_DIR, "Register_files/register_different.html"));
            return;
        }
        User user;
        user.username = dataraw["username"];
        user.password = hash_password(dataraw["password"]);
        if (db.table_user.create_user(user) < 0) {
            res.set_file(path_combine(SERVER_PUBLIC_DIR, "Register_files/register_repeat.html"));
        }
        else {
            res.set_redirect(res.header_field["Host"] + "/");
            res.header_field["Set-Cookie"] = "metadata=" + std::to_string(user.user_id) + "$" + user.password;
        }
    }
    void change_password(HTTPRequest &req, HTTPResponse &res) {
        auto dataraw = data_parser(req.message_body);
        if (dataraw.find("oldpwd") == dataraw.end() || 
            dataraw.find("newpwd") == dataraw.end() || 
            dataraw.find("newpwd2") == dataraw.end()) {
            res.set_status(HTTP::Status_Code::NotAcceptable);
            return;
        }
        std::string cookie = req.header_field["Cookie"];
        auto rt = cookie_parser(cookie);
        std::string pass = hash_password(dataraw["oldpwd"], rt.second.substr(0, 4));
        if (pass != rt.second) {
            res.set_message(update_password("setting", "/", 1));
            return;    
        }
        if (!password_format_checker(dataraw["newpwd"])) {
            res.set_message(update_password("setting", "/", 2));
            return;    
        }
        if (dataraw["newpwd"] != dataraw["newpwd2"]) {
            res.set_message(update_password("setting", "/", 3));
            return;    
        }
        pass = hash_password(dataraw["newpwd"]);
        res.set_redirect(res.header_field["Host"] + "/");
        res.header_field["Set-Cookie"] = "metadata=" + std::to_string(rt.first) + "$" + pass;
        db.table_user.update_password(rt.first, pass);
    }
    bool is_console(HTTPRequest &req) {
        return req.header_field.find("User-Agent") != req.header_field.end() && 
               req.header_field["User-Agent"] == CONSOLE_AGENT;
    }
    int add_friend(User &user, std::string friend_name) {
        int fid = db.table_user.get_id(friend_name);
        if (fid <= 0)
            return -1;
        if (db.table_user.add_friend(user.user_id, fid) <= 0)
            return -1;
        return 0;
    }
    int delete_friend(User &user, std::string friend_name) {
        int fid = db.table_user.get_id(friend_name);
        if (fid <= 0)
            return -1;
        if (db.table_user.delete_friend(user.user_id, fid) <= 0)
            return -1;
        return 0;
    }
    std::string get_chatroom_name(Chatroom &chatroom) {
        std::string flist;
        for (auto id : chatroom.user_idlist) {
            User u = db.table_user.get_object(id);
            if (!flist.empty())
                flist += ", ";
            flist += u.username;
        }
        return flist;
    }
};

void help(char *name) {
    std::cerr << "Usage: " << name << " [port]" << std::endl;
    exit(0);
}

void client_handler(HTTPSender *connection) {
    HTTPRequest req = connection->read_request();
    if (!req.method.empty()) {
        HTTPResponse res;
        if (req.request_target == "/login") {
            if (Handler::check_login(req))
                res.set_redirect(res.header_field["Host"] + "/");
            else if (req.method == "GET")
                res.set_file(path_combine(SERVER_PUBLIC_DIR, "Login_files/login.html"));
            else if (req.method == "POST") {
                Handler::try_login(req, res);
            }
            else {
                res.set_status(HTTP::Status_Code::MethodNotAllowed);
            }
        }
        else if (req.request_target == "/register") {
            if (Handler::check_login(req))
                res.set_redirect(res.header_field["Host"] + "/");
            else if (req.method == "GET") {
                res.set_file(path_combine(SERVER_PUBLIC_DIR, "Register_files/register.html"));
            }
            else if (req.method == "POST") {
                Handler::try_register(req, res);
            }
            else {
                res.set_status(HTTP::Status_Code::MethodNotAllowed);
            }
        }
        else if (Handler::check_login(req)) {
            User user = Handler::get_user(req);
            if (req.request_target == "/") {
                if (req.method != "GET")
                    res.set_status(HTTP::Status_Code::MethodNotAllowed);
                else {
                    res.set_message(homepage(user.username, "friend", "chatroom", "setting", "/", "logout"), true);
                }
            }
            else if (req.request_target == "/friend") {
                if (req.method != "GET") {
                    res.set_status(HTTP::Status_Code::MethodNotAllowed);
                    goto done;
                }
                std::vector<std::string> flist;
                for (int id : user.friend_idlist)
                    flist.push_back(Handler::get_username(id));
                if (Handler::is_console(req)) {
                    res.set_message(svector_to_string(flist));  
                }
                else {
                    res.set_message(manage_friend(flist, "manage_friend", "/"), true);
                }
            }
            else if (req.request_target == "/manage_friend") {
                if (req.method != "POST") {
                    res.set_status(HTTP::Status_Code::MethodNotAllowed);
                    goto done;
                }
                auto dataraw = Handler::data_parser(req.message_body); 
                if (dataraw.find("username") == dataraw.end() ||
                    dataraw.find("operation") == dataraw.end()) {
                    res.set_status(HTTP::Status_Code::NotAcceptable);
                    goto done;
                }
                if (dataraw["operation"] == "AddFriend") {
                    Handler::add_friend(user, dataraw["username"]);
                    res.set_redirect(res.header_field["Host"] + "/friend");
                }
                else if (dataraw["operation"] == "Unfriend") {
                    Handler::delete_friend(user, dataraw["username"]);
                    res.set_redirect(res.header_field["Host"] + "/friend");
                }
                else {
                    res.set_status(HTTP::Status_Code::NotAcceptable);
                }
            }
            else if (req.request_target == "/setting") {
                if (req.method == "GET") {
                    res.set_message(update_password("setting", "/"), true);
                }
                else if (req.method == "POST") {
                    Handler::change_password(req, res);
                }
                else {
                    res.set_status(HTTP::Status_Code::MethodNotAllowed);
                }
            }
            else if (int(req.request_target.size()) >= 9 && req.request_target.substr(0, 9) == "/chatroom") {
                std::string target_stack = "chatroom";
                req.request_target.erase(0, 9);
                if (req.request_target.empty()) {
                    auto roomlist = db.table_chatroom.get_chatroom_list(user.user_id);
                    if (req.method == "GET") {
                        std::vector<std::string> chatlist;
                        std::vector<std::string> hreflist;
                        for (auto chat : roomlist) {
                            chatlist.push_back(Handler::get_chatroom_name(chat));
                            hreflist.push_back("chatroom/" + std::to_string(chat.chatroom_id));
                        }
                        res.set_message(manage_chatroom(chatlist, hreflist, "chatroom", "/"), true);
                    }
                    else if (req.method == "POST") {
                        auto dataraw = Handler::data_parser(req.message_body); 
                        if (dataraw.find("member") == dataraw.end()) {
                            res.set_status(HTTP::Status_Code::NotAcceptable);
                        }
                        int mid = db.table_user.get_id(dataraw["member"]);
                        if (mid > 0) {
                            int flag = 0;
                            if (mid == user.user_id)
                                flag = 1;
                            for (int fid : user.friend_idlist) {
                                if (fid == mid)
                                    flag = 1;
                            }
                            if (flag == 1) {
                                for (auto chat : roomlist) {
                                    if (chat.user_idlist.size() == 1 && chat.user_idlist[0] == mid)
                                        flag = 0;
                                }
                                if (flag == 1) {
                                    Chatroom cht;
                                    cht.user_idlist.push_back(user.user_id);
                                    if (mid != user.user_id)
                                        cht.user_idlist.push_back(mid);
                                    db.table_chatroom.create_chatroom(cht);
                                }
                            }
                        }
                        res.set_redirect(res.header_field["Host"] + "/chatroom");
                    }
                    else {
                        res.set_status(HTTP::Status_Code::MethodNotAllowed);
                    }
                }
                else if (req.request_target[0] == '/') {
                    target_stack += "/";
                    req.request_target.erase(0, 1);
                    std::string tmp;
                    for (char c : req.request_target)
                        if (std::isdigit(c))
                            tmp.push_back(c);
                        else
                            break;
                    req.request_target.erase(0, int(tmp.size()));
                    int chat_id = std::stoi(tmp);
                    int valid = 0;
                    target_stack += tmp;
                    Chatroom chatroom = db.table_chatroom.get_object(chat_id);
                    for (int id : chatroom.user_idlist)
                        if (id == user.user_id)
                            valid = 1;
                    if (valid == 1) {
                        int maxid = db.table_message.maxseqid_of_chatroom(chat_id);
                        if (req.request_target.empty()) {
                            if (req.method == "GET") {
                                res.set_redirect(res.header_field["Host"] + "/" + target_stack + "/" + std::to_string(maxid));
                            }
                            else {
                                res.set_status(HTTP::Status_Code::MethodNotAllowed);
                            }
                        }
                        else if (req.request_target == "/manage_member") {
                            if (req.method != "POST") {
                                res.set_status(HTTP::Status_Code::MethodNotAllowed);
                                goto done;
                            }
                            auto dataraw = Handler::data_parser(req.message_body); 
                            if (dataraw.find("username") == dataraw.end() ||
                                dataraw.find("operation") == dataraw.end()) {
                                res.set_status(HTTP::Status_Code::NotAcceptable);
                                goto done;
                            }
                            int mid = db.table_user.get_id(dataraw["username"]);
                            int flag = 0;
                            for (int id : user.friend_idlist)
                                if (id == mid)
                                    flag = 1;
                            if (dataraw["operation"] == "Add") {
                                for (int id : chatroom.user_idlist)
                                    if (id == mid)
                                        flag = 0;
                                if (flag) {
                                    db.table_chatroom.add_user(chatroom.chatroom_id, mid);
                                }
                                res.set_redirect(res.header_field["Host"] + "/" + target_stack);
                            }
                            else if (dataraw["operation"] == "Remove") {
                                if (flag) {
                                    flag = 0;
                                    for (int id : chatroom.user_idlist)
                                        if (id == mid)
                                            flag = 1;
                                    if (flag) {
                                        db.table_chatroom.delete_user(chatroom.chatroom_id, mid);
                                    }
                                }
                                res.set_redirect(res.header_field["Host"] + "/" + target_stack);
                            }
                            else {
                                res.set_status(HTTP::Status_Code::NotAcceptable);
                            }
                        }
                        else if (req.request_target == "/send") {
                            if (req.method != "POST") {
                                res.set_status(HTTP::Status_Code::MethodNotAllowed);
                                goto done;
                            }
                            if (int(req.message_body.size()) < 8 || req.message_body.substr(0, 8) != "message=") {
                                res.set_status(HTTP::Status_Code::NotAcceptable);
                                goto done;
                            }
                            req.message_body.erase(0, 8);
                            Message message;
                            message.chatroom_id = chat_id;
                            message.sender_id = user.user_id;
                            message.timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                            message.text = req.message_body;
                            message.type = 0;
                            db.table_message.create_message(message);
                            res.set_redirect(res.header_field["Host"] + "/" + target_stack);
                        }
                        else if (req.request_target == "/upload") {
                            if (req.method != "POST") {
                                res.set_status(HTTP::Status_Code::MethodNotAllowed);
                                goto done;
                            }
                            Message message;
                            std::string tmp_file_name = req.file.file_name;
                            std::string lastline;
                            req.file.file_close();
                            File tmpf, realf;
                            tmpf.file_open_read(tmp_file_name);
                            {
                                SHA256 sha256sum;
                                char buffer[FILEBUF];
                                long long cur = 0;
                                long long sz = tmpf.size();
                                while (cur < sz) {
                                    int rt = tmpf.readf(buffer, std::min(sz - cur, (long long)FILEBUF - 1));
                                    buffer[rt] = 0;
                                    sha256sum.update(std::string(buffer));
                                    cur += rt;
                                }
                                uint8_t *digest = sha256sum.digest();
                                message.filehash = SHA256::toString(digest);
                                delete[] digest;
                            }
                            tmpf.seekto(0);
                            for (int i = 0; i < 4; ++i) {
                                std::string line;
                                char c;
                                while (true) {
                                    tmpf.readf(&c, 1);
                                    line.push_back(c);
                                    if (int(line.size() >= 2) && line.substr(int(line.size()) - 2, 2) == "\r\n") {
                                        line.pop_back(), line.pop_back();
                                        break;
                                    }
                                }
                                if (i == 1) {
                                    line.pop_back();
                                    int pos = line.find("filename=\"") + 10;
                                    message.text = line.substr(pos);
                                    message.filehash += "." + get_file_extension(message.text);
                                }
                            }
                            long long cur = tmpf.get_pos();
                            long long sz = tmpf.size();
                            long long endpos = 0;
                            for (int i = 0, state = 0; i < sz; ++i) {
                                char c;
                                tmpf.seekto(sz - i);
                                tmpf.readf(&c, 1);
                                lastline.push_back(c);
                                if (int(lastline.size() >= 2) && lastline.substr(int(lastline.size()) - 2, 2) == "\n\r") {
                                    if (state == 0)
                                        ++state;
                                    else
                                        break;
                                }
                            }
                            endpos = tmpf.get_pos() - 2;
                            tmpf.seekto(cur);
                            realf.file_open_write(path_combine(std::vector<std::string>({SERVER_PUBLIC_DIR, "file", message.filehash})));
                            char buffer[FILEBUF];
                            while (cur < endpos) {
                                int rt = tmpf.readf(buffer, std::min(endpos - cur, (long long)FILEBUF));
                                realf.writef(buffer, rt);
                                cur += rt;
                            }
                            tmpf.file_close(), realf.file_close();
                            remove(tmp_file_name.c_str());
                            message.chatroom_id = chat_id;
                            message.sender_id = user.user_id;
                            message.timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                            message.type = 1;
                            db.table_message.create_message(message);
                            res.set_redirect(res.header_field["Host"] + "/" + target_stack);
                        }
                        else if (int(req.request_target.size() > 10) && req.request_target.substr(0, 10) == "/download/") {
                            req.request_target.erase(0, 10);
                            res.set_file(path_combine(std::vector<std::string>({SERVER_PUBLIC_DIR, "file", req.request_target})));
                        }
                        else if (req.request_target[0] == '/') {
                            req.request_target.erase(0, 1);
                            std::string tmp2;
                            for (char c : req.request_target)
                                if (std::isdigit(c))
                                    tmp2.push_back(c);
                                else
                                    break;
                            int msg_id = 0;
                            if (!tmp2.empty())
                                msg_id = std::stoi(tmp2);
                            else
                                msg_id = maxid;
                            if (msg_id < 1)
                                msg_id = 0;
                            if (msg_id > maxid)
                                msg_id = maxid;
                            req.request_target.erase(0, tmp2.size());
                            if (req.method == "GET") {
                                std::string name = Handler::get_chatroom_name(chatroom);
                                std::vector<std::string> mlist;
                                std::string plink = std::to_string(msg_id - MESSAGE_BLOCK_SIZE);
                                std::string nlink = std::to_string(msg_id + MESSAGE_BLOCK_SIZE);

                                auto msg_vec = db.table_message.query_range_fixed_chatroom(chat_id, msg_id - MESSAGE_BLOCK_SIZE + 1, msg_id);
                                for (Message msg : msg_vec) {
                                    User sender = db.table_user.get_object(msg.sender_id);
                                    std::string result = sender.username + ": ";
                                    if (msg.type == 0) {
                                        result += msg.text; 
                                    }
                                    else if (msg.type == 1) {
                                        if (get_file_type(msg.text) == "image") {
                                            result += img(msg.filehash);
                                        }
                                        else {
                                            result += href("download/" + msg.filehash, msg.text); 
                                        }
                                    } 
                                    else {
                                        continue;
                                    }
                                    mlist.push_back(result);
                                }

                                res.set_message(chat_page(name, mlist, plink, nlink, "send", "upload", "manage_member", "/"), true);
                            }
                            else {
                                res.set_status(HTTP::Status_Code::MethodNotAllowed);
                            }
                        }
                        else {
                            res.set_redirect(res.header_field["Host"] + "/" + target_stack);
                        }
                    }
                    else {
                        res.set_redirect(res.header_field["Host"] + "/chatroom");
                    }
                }
                else {
                    res.set_redirect(res.header_field["Host"] + "/");
                }
            }
            else if (req.request_target == "/logout") {
                if (req.method != "GET") {
                    res.set_status(HTTP::Status_Code::MethodNotAllowed);
                    goto done;
                }
                res.set_redirect(res.header_field["Host"] + "/login");
                res.header_field["Set-Cookie"] = "metadata=-1$null";
            }
            else if (req.request_target == "/favicon.ico")
                res.set_file(path_combine(SERVER_PUBLIC_DIR, "favicon-16x16.png"));
            else 
                res.set_file(path_combine(SERVER_PUBLIC_DIR, req.request_target));
        }
        else {
            if (req.request_target == "/")
                res.set_redirect(res.header_field["Host"] + "/login");
            else if (req.request_target == "/favicon.ico")
                res.set_file(path_combine(SERVER_PUBLIC_DIR, "favicon-16x16.png"));
            else
                res.set_file(path_combine(SERVER_PUBLIC_DIR, req.request_target));
        }
done:
        int rtv = connection->send_response(res);
        if (rtv < 0)
            _helper_warning2("something wrong on http response", 1);
    }
    delete connection;
    main_thread.notify();
}

void polling_handler(HTTPServer &server) {
    while (true) {
        HTTPSender *connection = server.wait_client();
        create_thread(client_handler, connection);
    } 
}

int main(int argc, char *argv[]) {
    random_init();
    if (argc < 2)
        help(argv[0]);
    HTTPServer server(atoi(argv[1]));
    create_thread(polling_handler, std::ref(server));
    // possibly race condition, how to solve it?
    main_thread.start();
}
