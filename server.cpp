#include "helper.hpp"
#include "http.hpp"
#include "db.hpp"
#include "param.hpp"
#include "main_thread.hpp"
#include "server_object.hpp"
#include "file.hpp"
#include "crypto.hpp"
#include "html.hpp"
#include <iostream>
#include <unistd.h>
#include <utility>
#include <sstream>
#include <string>
#include <cctype>
#include <unordered_map>

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
        _helper_msg(user.password);
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
                    res.set_message(homepage(user.username, "friend", "chatroom", "setting", "/"), true);
                }
            }
            else if (req.request_target == "/friend") {
                if (req.method != "GET")
                    res.set_status(HTTP::Status_Code::MethodNotAllowed);
                else {
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
            }
            else if (req.request_target == "/manage_friend") {
                if (req.method != "POST")
                    res.set_status(HTTP::Status_Code::MethodNotAllowed);
                else {
                    auto dataraw = Handler::data_parser(req.message_body); 
                    if (dataraw.find("username") == dataraw.end() ||
                        dataraw.find("operation") == dataraw.end()) {
                        res.set_status(HTTP::Status_Code::NotAcceptable);
                        _helper_log("Not Acceptable");
                    }
                    else {
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
                std::string target_stack = "/chatroom";
                req.request_target.erase(0, 9);
                if (req.request_target.empty()) {
                    if (req.method != "GET")
                        res.set_status(HTTP::Status_Code::MethodNotAllowed);
                    else {
                        // TODO: html
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
                    target_stack += tmp;
                    Chatroom chatroom = db.table_chatroom.get_object(chat_id);
                    if (req.request_target.empty()) {
                        if (req.method == "GET") {
                            // TODO: html
                        }
                        else if (req.method == "POST") {
                            // TODO: action
                            res.set_redirect(res.header_field["Host"] + target_stack);
                        }
                        else {
                            res.set_status(HTTP::Status_Code::MethodNotAllowed);
                        }
                    }
                    else if (req.request_target == "/upload") {
                        // TODO: action
                        res.set_redirect(res.header_field["Host"] + target_stack);
                    }
                    else if (req.request_target[0] == '/') {
                        if (req.method != "GET")
                            res.set_status(HTTP::Status_Code::MethodNotAllowed);
                        else {
                            req.request_target.erase(0, 1);
                            std::string tmp;
                            for (char c : req.request_target)
                                if (std::isdigit(c))
                                    tmp.push_back(c);
                                else
                                    break;
                            int msg_id = 0;
                            int maxid = db.table_message.maxseqid_of_chatroom(chat_id);
                            if (!tmp.empty())
                                msg_id = std::stoi(tmp);
                            if (msg_id < 1)
                                msg_id = 1;
                            if (msg_id > maxid)
                                msg_id = maxid;
                            // TODO: fetch messages
                        }
                    }
                    else {
                        res.set_redirect(res.header_field["Host"] + target_stack);
                    }
                }
                else {
                    res.set_redirect(res.header_field["Host"] + "/");
                }
            }
            else if (req.request_target == "/logout") {
                if (req.method != "POST") {
                    res.set_status(HTTP::Status_Code::MethodNotAllowed);
                }
                else {
                    res.set_message("Bye~");
                    res.header_field["Set-Cookie"] = "metadata=-1$null";
                }
            }
        }
        else {
            if (req.request_target == "/")
                res.set_redirect(res.header_field["Host"] + "/login");
            else
                res.set_file(path_combine(SERVER_PUBLIC_DIR, req.request_target));
        }
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
