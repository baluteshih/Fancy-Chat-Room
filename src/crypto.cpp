#include "crypto.hpp"
#include <string>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <random>

void random_init() {
    Random_Engine::gen = std::mt19937((unsigned)time(NULL) * getpid()); 
}

std::string Random_Engine::gen_random_string(int len) {
    static const std::string alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string rt;
    std::uniform_int_distribution<int> dist(0, int(alphanum.size()) - 1);
    for (int i = 0; i < len; ++i) {
        rt += alphanum[dist(Random_Engine::gen)];
    }
    return rt;
}

std::string sha256sum(std::string plain) {
    return plain; 
}

std::string hash_password(std::string plain, std::string salt) {
    if (int(salt.size()) < 4) {
        salt = Random_Engine::gen_random_string(4);
    } 
    return salt + sha256sum(salt + plain);
}
