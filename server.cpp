#include "helper.hpp"
#include "http.hpp"
#include "db.hpp"
#include "param.hpp"
#include "main_thread.hpp"
#include "server_object.hpp"
#include "file.hpp"
#include "crypto.hpp"
#include <iostream>
#include <unistd.h>
#include <utility>
#include <sstream>
#include <string>
#include <cctype>
#include <unordered_map>

Main_Thread main_thread;
DataBase db;

namespace User_Handler {
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
            if (c == '=' || c == ';')
                c = ' ';
        std::stringstream ss(cookie);
        std::string tmp;
        ss >> tmp >> rt.first >> tmp >> rt.second;
        return rt;
    }
    bool cookie_format_checker(std::string cookie) {
        // TODO: content
        return 1;
    }
    User get_user(int id) {
        if (id <= 0)
            return User();
        return db.table_user.get_object(id);
    }
    User get_user(HTTPRequest &req) {
        if (req.header_field.find("Cookie") == req.header_field.end())
            return User();
        std::string cookie = req.header_field["Cookie"];
        auto rt = cookie_parser(cookie);
        return get_user(rt.first); 
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
            res.set_file(path_combine(SERVER_PUBLIC_DIR, "login.html"));
            return;
        }
        int user_id = db.table_user.get_id(dataraw["username"]);
        if (user_id <= 0) {
            res.set_file(path_combine(SERVER_PUBLIC_DIR, "login.html"));
            return;
        }
        User user = get_user(user_id);
        std::string pass = hash_password(dataraw["password"], user.password.substr(0, 4));
        if (pass != user.password) {
            res.set_file(path_combine(SERVER_PUBLIC_DIR, "login.html"));
            return;
        }
        res.set_redirect(res.header_field["Host"] + "/");
        res.header_field["Set-Cookie"] = "userid=" + std::to_string(user_id) + "; password=" + pass;
    }
    void try_register(HTTPRequest &req, HTTPResponse &res) {
        
    }
};

void help(char *name) {
    std::cerr << "Usage: " << name << " [port]" << std::endl;
    exit(0);
}

void client_handler(HTTPSender *connection) {
    while (true) {
        HTTPRequest req = connection->read_request();
        HTTPResponse res;
        if (req.request_target == "/login") {
            if (User_Handler::check_login(req))
                res.set_redirect(res.header_field["Host"] + "/");
            else if (req.method == "GET")
                res.set_file(path_combine(SERVER_PUBLIC_DIR, "login.html"));
            else if (req.method == "POST") {
                User_Handler::try_login(req, res);
            }
            else {
                res.set_status(HTTP::Status_Code::MethodNotAllowed);
            }
        }
        else if (req.request_target == "/register") {
            if (User_Handler::check_login(req))
                res.set_redirect(res.header_field["Host"] + "/");
            else if (req.method == "GET") {
                res.set_file(path_combine(SERVER_PUBLIC_DIR, "register.html"));
            }
            else if (req.method == "POST") {
                User_Handler::try_register(req, res);
            }
            else {
                res.set_status(HTTP::Status_Code::MethodNotAllowed);
            }
        }
        else if (User_Handler::check_login(req)) {
            if (req.request_target == "/") {
                if (req.method != "GET")
                    res.set_status(HTTP::Status_Code::MethodNotAllowed);
                else {
                    // TODO: html
                }
            }
            else if (req.request_target == "/friend_list") {
                if (req.method != "GET")
                    res.set_status(HTTP::Status_Code::MethodNotAllowed);
                else {
                    // TODO: html
                }
            }
            else if (req.request_target == "/add_friend") {
                if (req.method != "POST")
                    res.set_status(HTTP::Status_Code::MethodNotAllowed);
                else {
                    // TODO: action
                    res.set_redirect(res.header_field["Host"] + "/friend");
                }
            }
            else if (req.request_target == "/delete_friend") {
                if (req.method != "POST")
                    res.set_status(HTTP::Status_Code::MethodNotAllowed);
                else {
                    // TODO: action
                    res.set_redirect(res.header_field["Host"] + "/friend");
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
                    res.header_field["Set-Cookie"] = "userid=-1; password=null";
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
        if (rtv < 0) {
            _helper_warning2("something wrong on http response", 1);
        }
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
    if (argc < 2)
        help(argv[0]);
    HTTPServer server(atoi(argv[1]));
    create_thread(polling_handler, std::ref(server));
    // possibly race condition, how to solve it?
    main_thread.start();
}
