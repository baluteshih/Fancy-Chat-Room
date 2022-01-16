#include "helper.hpp"
#include "main_thread.hpp"
#include "http.hpp"
#include "param.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sstream>

Main_Thread main_thread;

std::pair<std::string, unsigned short> parse(std::string addr) {
    std::pair<std::string, unsigned short> rt;
    for (char &c : addr)
        if (c == ':')
            c = ' ';
    std::stringstream ss(addr);
    ss >> rt.first >> rt.second;
    return rt;
}

HTTPSender *gen_connection(std::string &server_addr) {
    auto addr = parse(server_addr);
    int conn_fd;
    if ((conn_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        _helper_fail2("socket error", 1);
    struct sockaddr_in sockinfo;
    memset(&sockinfo, 0, sizeof sockinfo);
    sockinfo.sin_family = AF_INET;
    sockinfo.sin_port = htons(addr.second);
    int res = inet_pton(AF_INET, addr.first.c_str(), &sockinfo.sin_addr);
    if (res == 0) {
        close(conn_fd);
        _helper_fail("the ip address doesn't valid");
    }
    if (connect(conn_fd, (struct sockaddr *)&sockinfo, sizeof sockinfo) < 0)
        _helper_fail2("connect error", 1);
    HTTPSender *rt = new HTTPSender(conn_fd);
    return rt;
}

void client_handler(HTTPSender *connection, std::string &server_addr) {
    HTTPRequest req = connection->read_request();
    HTTPResponse res;
    res.set_predirect(server_addr + req.request_target);
    int rtv = connection->send_response(res);
    if (rtv < 0)
         _helper_warning2("something wrong on http response", 1);
    delete connection;
    main_thread.notify();
}

void help(char *name) {
    std::cerr << "Usage: " << name << " [serverip:port]" << std::endl;
    exit(0);
}

void polling_handler(HTTPServer &server, std::string &server_addr) {
    while (true) {
        HTTPSender *connection = server.wait_client();
        create_thread(client_handler, connection, std::ref(server_addr));
    } 
}

int main(int argc, char *argv[]) {
    if (argc < 2)
        help(argv[0]);
    std::string server_addr(argv[1]);
    std::string cookie;
    /*HTTPServer server(atoi(argv[2]));
    create_thread(polling_handler, std::ref(server), std::ref(server_addr));*/
    while (true) {
        std::cout << "Fancy Chat Room" << std::endl;
        std::cout << " (1) Login" << std::endl;
        std::cout << " (2) Register" << std::endl;
        std::cout << " (3) Exit" << std::endl;
        std::cout << "$ " << std::flush;
        int option;
        std::cin >> option;
        if (option == 1) {
            while (true) {
                std::string handle;
                std::string pass;
                std::cout << "Please enter your handle: " << std::flush;
                std::cin >> handle;
                std::cout << "Please enter your password: " << std::flush;
                std::cin >> pass;
                HTTPSender *connection = gen_connection(server_addr);
                HTTPRequest req;
                req.set_message("username=" + handle + "&password=" + pass, "/login", "POST");
                connection->send_request(req);
                HTTPResponse res = connection->read_response();
                delete connection;
                if (res.header_field.find("Set-Cookie") != res.header_field.end()) {
                    cookie = res.header_field["Set-Cookie"];
                    std::cout << "-- Login successfully, welcome " + handle << std::endl;
                    break;
                }
                std::cout << "-- Unknown handle or password!" << std::endl;
            }
            while (true) {
                std::cout << "Home (for full function, connection to " + server_addr + " directly with your browser!)" << std::endl;
                std::cout << " (1) List all friends" << std::endl;
                std::cout << " (2) Add friend" << std::endl;
                std::cout << " (3) Delete friend" << std::endl;
                std::cout << " (4) Choose a chat room" << std::endl;
                std::cout << " (5) Logout" << std::endl;
                std::cout << "$ " << std::flush;
                int option;
                std::cin >> option;
                if (option == 1) {
                    HTTPSender *connection = gen_connection(server_addr);
                    HTTPRequest req;
                    req.set_message("", "/friend", "GET", cookie);
                    connection->send_request(req);
                    HTTPResponse res = connection->read_response();
                    delete connection;
                    if (res.message_body == "")
                        std::cout << "Unfortunately, you don't have friend :(" << std::endl;
                    else {
                        for (char &c : res.message_body)
                            if (c == LIST_DELIMITER[0])
                                c = ' ';
                        std::stringstream ss(res.message_body);
                        std::string tmp;
                        for (int i = 1; ss >> tmp; ++i)
                            std::cout << "  " << i << ". " << tmp << std::endl;
                    }
                }
                else if (option == 2) {
                    std::cout << "Please type the handle you want to make a friend with: \n$ " << std::flush;
                    std::string name;
                    std::cin >> name;
                    HTTPSender *connection = gen_connection(server_addr);
                    HTTPRequest req;
                    req.set_message("username=" + name + "&operation=AddFriend", "/manage_friend", "POST", cookie);
                    connection->send_request(req);
                    HTTPResponse res = connection->read_response();
                    delete connection;
                    std::cout << "-- If the user exists and he or she is not your friend, then you added successfully." << std::endl; 
                }
                else if (option == 3) {
                    std::cout << "Please type the handle you don't want to make a friend with: \n$ " << std::flush;
                    std::string name;
                    std::cin >> name;
                    HTTPSender *connection = gen_connection(server_addr);
                    HTTPRequest req;
                    req.set_message("username=" + name + "&operation=Unfriend", "/manage_friend", "POST", cookie);
                    connection->send_request(req);
                    HTTPResponse res = connection->read_response();
                    delete connection;
                    std::cout << "-- If the user is your friend, then you unfriended successfully." << std::endl; 
                }
                else if (option == 4) {
                    std::vector<std::string> name_list(1, "");
                    {
                        HTTPSender *connection = gen_connection(server_addr);
                        HTTPRequest req;
                        req.set_message("", "/friend", "GET", cookie);
                        connection->send_request(req);
                        HTTPResponse res = connection->read_response();
                        delete connection;
                        if (res.message_body == "")
                            std::cout << "Unfortunately, you don't have friend :(" << std::endl;
                        else {
                            for (char &c : res.message_body)
                                if (c == LIST_DELIMITER[0])
                                    c = ' ';
                            std::stringstream ss(res.message_body);
                            std::string tmp;
                            for (int i = 1; ss >> tmp; ++i) {
                                std::cout << "  " << i << ". " << tmp << std::endl;
                                name_list.push_back(tmp);
                            }
                        }
                    }
                    if (int(name_list.size()) == 1) {
                        std::cout << "-- Please add a friend first." << std::endl;
                    }
                    else {
                        int max_id, chat_id, cur_id;
                        while (true) {
                            std::cout << "Please choose a friend to chat with: \n$ " << std::flush;
                            int num;
                            std::cin >> num;
                            if (num <= 0 || num >= int(name_list.size()))
                                std::cout << "Out of range." << std::endl;
                            else {
                                HTTPSender *connection = gen_connection(server_addr);
                                HTTPRequest req;
                                req.set_message("member=" + name_list[num], "/chatroom", "GET", cookie);
                                connection->send_request(req);
                                HTTPResponse res = connection->read_response();
                                delete connection;
                                std::stringstream ss(res.message_body);
                                ss >> chat_id >> cur_id;
                                max_id = cur_id;
                                break;
                            }
                        }
                        while (true) {
                            std::string command, tmp;
                            {
                                HTTPSender *connection = gen_connection(server_addr);
                                HTTPRequest req;
                                req.set_message("", "/chatroom/" + std::to_string(chat_id) + "/" + std::to_string(cur_id), "GET", cookie);
                                connection->send_request(req);
                                HTTPResponse res = connection->read_response();
                                delete connection;
                                std::stringstream ss(res.message_body);
                                std::getline(ss, tmp);
                                max_id = std::stoi(tmp);
                                while (getline(ss, tmp))
                                    std::cout << tmp << std::endl;
                            }
                            std::cout << "Options:" << std::endl;
                            std::cout << " - send<space><message>" << std::endl;
                            std::cout << " - prev" << std::endl;
                            std::cout << " - next" << std::endl;
                            std::cout << " - quit" << std::endl;
                            std::cout << "$ " << std::flush;
                            std::cin >> command;
                            if (command == "send") {
                                std::cin.get();
                                std::getline(std::cin, tmp);
                                HTTPSender *connection = gen_connection(server_addr);
                                HTTPRequest req;
                                req.set_message("message=" + tmp, "/chatroom/" + std::to_string(chat_id) + "/send", "POST", cookie);
                                connection->send_request(req);
                                HTTPResponse res = connection->read_response();
                                delete connection;
                                cur_id = max_id = std::stoi(res.message_body);
                            }
                            else if (command == "prev") {
                                cur_id = std::max(0, cur_id - MESSAGE_BLOCK_SIZE); 
                            }
                            else if (command == "next") {
                                cur_id = std::min(max_id, cur_id + MESSAGE_BLOCK_SIZE); 
                            }
                            else if (command == "quit") {
                                break; 
                            }
                        }        
                    }
                }
                else if (option == 5) {
                    cookie = "";
                    std::cout << "Bye!" << std::endl;
                    break;
                }
            }
        }
        else if (option == 2) {
            while (true) {
                std::string handle;
                std::string pass1, pass2;
                std::cout << "Please enter your handle: " << std::flush;
                std::cin >> handle;
                std::cout << "Please enter your password: " << std::flush;
                std::cin >> pass1;
                std::cout << "Retype your password again: " << std::flush;
                std::cin >> pass2;
                if (pass1 != pass2) {
                    std::cout << "-- Passwords are not the same." << std::endl;
                    continue;
                }
                HTTPSender *connection = gen_connection(server_addr);
                HTTPRequest req;
                req.set_message("username=" + handle + "&password=" + pass1 + "&password2=" + pass2, "/register", "POST");
                connection->send_request(req);
                HTTPResponse res = connection->read_response();
                delete connection;
                if (res.header_field.find("Set-Cookie") != res.header_field.end()) {
                    std::cout << "-- Registration success. Login again to use the fancy chat room." << std::endl;
                    break;
                }
                std::cout << "-- Invalid handle or password!" << std::endl;
            }
        }
        else if (option == 3) {
            break;
        }
    }
}
