#include "html.hpp"
#include <map>
std::string p(std::string sentence){
    return "<p>" + sentence + "</p>";
}
std::string img(std::string src){
    return "<img src=\"" + src + "\" width=" + std::to_string(IMAGE_WIDTH) + " height=" + std::to_string(IMAGE_HEIGHT) + ">";
}
std::string href(std::string download_link, std::string filename){
    return "<a href=\"" + download_link + "\">" + filename +"</a>";
}
std::string homepage(std::string username, std::string addr_manage_friend, std::string addr_list_chatroom, std::string addr_change_password, std::string addr_homepage){
    std::map<std::string, std::string> val;
    val["username"] = username;
    val["addr_manage_friend"] = addr_manage_friend;
    val["addr_list_chatroom"] = addr_list_chatroom;
    val["addr_change_password"] = addr_change_password;
    val["addr_homepage"] = addr_homepage;
    const std::string filename("../public_server/html/homepage.html");
    std::ifstream input_file(filename);
    std::string ans;
    char c;
    int state = 0;
    std::string argName;
    while (input_file.get(c)){
        if (state == 0){
            if (c == '$'){
                state = 1;
            }
            else {
                ans += c;
            }
        }
        else {
            if (c == '$'){
                ans += val[argName];
                argName.clear();
                state = 0;
            }
            else {
                argName += c;
            }
        }
    }
    return ans;
}
std::string manage_friend(std::vector<std::string> friend_name, std::string form_action, std::string addr_homepage){
    const std::string filename("../public_server/html/manage_friend.html");
    std::ifstream input_file(filename);
    std::string ans;
    char c;
    while (input_file.get(c)){
        if (c == '$'){
            ans += "    <ol>\n";
            for (auto name : friend_name)
                ans += "        <li>" + name + "</li><br>\n";
            ans += "    </ol>";
        }
        else if (c == '@'){
            ans += form_action;
        }
        else if (c == '`'){
            ans += addr_homepage;
        }
        else {
            ans += c;
        }
    }
    return ans;
}
std::string manage_chatroom(std::vector<std::string> chatroom_name, std::vector<std::string> href_name, std::string create_action, std::string edit_action, std::string addr_homepage){
    if ((int)chatroom_name.size() != (int)href_name.size()){
        _helper_fail("#chatroom != #href");
    }
    const std::string filename("../public_server/html/manage_chatroom.html");
    std::ifstream input_file(filename);
    std::string ans;
    char c;
    while (input_file.get(c)){
        if (c == '$'){
            ans += "    <ol>\n";
            for (int i = 0; i < (int)chatroom_name.size(); i++){
                ans += "        <li>" + chatroom_name[i] + "</li><a href=\"" + href_name[i] + "\"><button>Chat!</button></a><br>\n";
            }
            ans += "    </ol>";
        }
        else if (c == '@'){
            ans += create_action;
        }
        else if (c == '&'){
            ans += edit_action;
        }
        else if (c == '`'){
            ans += addr_homepage;
        }
        else {
            ans += c;
        }
    }
    return ans;
}
std::string chat_page(std::string chatroom_name, std::vector<std::string> message, std::string prev_href, std::string next_href, std::string post_action, std::string addr_homepage){
    const std::string filename("../public_server/html/chat_page.html");
    std::ifstream input_file(filename);
    std::string ans;
    char c;
    while (input_file.get(c)){
        if (c == '*'){
            ans += chatroom_name;
        }
        else if (c == '$'){
            for (auto m : message)
                ans += "\t" + p(m) + '\n';
        }
        else if (c == '@'){
            ans += prev_href;
        }
        else if (c == '#'){
            ans += next_href;
        }
        else if (c == '%'){
            ans += post_action;
        }
        else if (c == '`'){
            ans += addr_homepage;
        }
        else {
            ans += c;
        }
    }
    return ans;
}
std::string update_password(std::string change_action, std::string addr_homepage){
    const std::string filename("../public_server/html/update_password.html");
    std::ifstream input_file(filename);
    std::string ans;
    char c;
    while (input_file.get(c)){
        if (c == '$'){
            ans += change_action;
        }
        else if (c == '`'){
            ans += addr_homepage;
        }
        else {
            ans += c;
        }
    }
    return ans;
}
