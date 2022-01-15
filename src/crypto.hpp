#ifndef _CRYPTO_HPP_
#define _CRYPTO_HPP_

#include <string>
#include <random>

namespace Random_Engine {
    static std::mt19937 gen;
    std::string gen_random_string(int len);
};

void random_init();

std::string sha256sum(std::string plain);

std::string hash_password(std::string plain, std::string salt = "");

#endif // _CRYPTO_HPP_
