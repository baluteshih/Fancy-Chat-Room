#ifndef _FILE_HPP_
#define _FILE_HPP_

#include <string>
#include <cstdio>
#include <cstddef>

#define FILEBUF 65536
using std::size_t;

std::string path_combine(std::string path1, std::string path2);

class File {
    FILE *fp;
    std::string file_name;
public:
    File();
    size_t readf(void *buf, size_t count);
    size_t writef(void *buf, size_t count);
    int file_open_read(const std::string &_name);
    int file_open_write(const std::string &_name);
    int file_close();
    bool is_open();
    size_t size();
    std::string extension(); 
    std::string type();
};

#endif // _FILE_HPP_
