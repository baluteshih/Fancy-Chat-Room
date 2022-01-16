#include "html.hpp"
#include "file.hpp"
#include <map>
std::string p(std::string sentence){
    return "<p>" + sentence + "</p>";
}
std::string img(std::string src){
    return "<img src=\"" + src + "\" width=" + std::to_string(IMAGE_WIDTH) + " height=" + std::to_string(IMAGE_HEIGHT) + "/>";
}
std::string href(std::string download_link, std::string filename){
    return "<a href=\"" + download_link + "\">" + filename +"</a>";
}
std::string homepage(std::string username, std::string addr_manage_friend, std::string addr_list_chatroom, std::string addr_change_password, std::string addr_homepage, std::string addr_logout){
    std::map<std::string, std::string> val;
    val["username"] = username;
    val["addr_manage_friend"] = addr_manage_friend;
    val["addr_list_chatroom"] = addr_list_chatroom;
    val["addr_change_password"] = addr_change_password;
    val["addr_homepage"] = addr_homepage;
    val["addr_logout"] = addr_logout;
    const std::string filename(path_combine(std::vector<std::string>({SERVER_PUBLIC_DIR, "html", "homepage.html"})));
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
    const std::string filename(path_combine(std::vector<std::string>({SERVER_PUBLIC_DIR, "html", "manage_friend.html"})));
    std::ifstream input_file(filename);
    std::string ans;
    char c;
    while (input_file.get(c)){
        if (c == '$'){
            if (friend_name.empty()){
                ans += p(NO_FRIEND_QQ);
            }
            else {
                ans += "    <ol>\n";
                for (auto name : friend_name)
                    ans += "        <li>" + name + "</li><br>\n";
                ans += "    </ol>";
            }
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
std::string manage_chatroom(std::vector<std::string> chatroom_name, std::vector<std::string> href_name, std::string create_action, std::string addr_homepage){
    if ((int)chatroom_name.size() != (int)href_name.size()){
        _helper_fail("#chatroom != #href");
    }
    const std::string filename(path_combine(std::vector<std::string>({SERVER_PUBLIC_DIR, "html", "manage_chatroom.html"})));
    std::ifstream input_file(filename);
    std::string ans;
    char c;
    while (input_file.get(c)){
        if (c == '$'){
            if (chatroom_name.empty()){
                ans += p(NO_CHATROOM_QQ);
            }
            else {
                ans += "    <ol>\n";
                for (int i = 0; i < (int)chatroom_name.size(); i++){
                    ans += "        <li>" + chatroom_name[i] + "</li><a href=\"" + href_name[i] + "\"><button>Chat!</button></a><br>\n";
                }
                ans += "    </ol>";
            }
        }
        else if (c == '@'){
            ans += create_action;
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
std::string chat_page(std::string chatroom_name, std::vector<std::string> message, std::string prev_href, std::string next_href, std::string send_action, std::string upload_action, std::string edit_action, std::string addr_homepage){
    const std::string filename(path_combine(std::vector<std::string>({SERVER_PUBLIC_DIR, "html", "chat_page.html"})));
    std::ifstream input_file(filename);
    std::string ans;
    char c;
    while (input_file.get(c)){
        if (c == '*'){
            ans += chatroom_name;
        }
        else if (c == '$'){
            if (message.empty())
                ans += p(NO_MESSAGE_QQ);
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
            ans += send_action;
        }
        else if (c == '^'){
            ans += upload_action;
        }
        else if (c == '`'){
            ans += addr_homepage;
        }
        else if (c == '('){
            ans += edit_action;
        }
        else {
            ans += c;
        }
    }
    return ans;
}
std::string update_password(std::string change_action, std::string addr_homepage, int type){
    std::string filename;
    if (type == 1)
        filename = path_combine(std::vector<std::string>({SERVER_PUBLIC_DIR, "html", "change_password_passfail.html"}));
    else if (type == 2)
        filename = path_combine(std::vector<std::string>({SERVER_PUBLIC_DIR, "html", "change_password_invalid.html"}));
    else if (type == 3)
        filename = path_combine(std::vector<std::string>({SERVER_PUBLIC_DIR, "html", "change_password_different.html"}));
    else
        filename = path_combine(std::vector<std::string>({SERVER_PUBLIC_DIR, "html", "change_password.html"}));
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
