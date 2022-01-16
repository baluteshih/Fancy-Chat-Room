#ifndef _HTML_HPP_
#define _HTML_HPP_

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "param.hpp"
#include "helper.hpp"

std::string p(std::string sentence);
/*
convert a sentence to html-displayable
*/

std::string img(std::string src);
/*
convert an image name to a renderable html command (i.e. <img src=...>
*/

std::string href(std::string download_link, std::string filename);
/*
convert a file name to a downloadable html command (i.e. <a href=$download_link>...)
*/
// ^^^ These functions are used to generate one single line

// vvv These functions are used to generate a whole html file
std::string homepage(std::string username, std::string addr_manage_friend, std::string addr_list_chatroom, std::string addr_change_password, std::string addr_homepage, std::string addr_logout);

std::string manage_friend(std::vector<std::string> friend_name, std::string form_action, std::string addr_homepage);

std::string manage_chatroom(std::vector<std::string> chatroom_name, std::vector<std::string> href_name, std::string create_action, std::string edit_action, std::string addr_homepage);

std::string chat_page(std::string chatroom_name, std::vector<std::string> message, std::string prev_href, std::string next_href, std::string post_action, std::string addr_homepage);

std::string update_password(std::string change_action, std::string addr_homepage, int type = 0);

#endif // _HTML_HPP_
