#ifndef _FILE_HPP_
#define _FILE_HPP_

#include <string>
#include <cstdio>
#include <cstddef>
#include <vector>

#define FILEBUF 65536
using std::size_t;

std::string path_combine(std::string path1, std::string path2);

std::string path_combine(std::vector<std::string> paths);

std::string get_file_extension(std::string filename);

std::string get_file_type(std::string filename);

class File {
    FILE *fp;
public:
    std::string file_name;
    File();
    size_t readf(void *buf, size_t count);
    size_t writef(void *buf, size_t count);
    int file_open_read(const std::string &_name);
    int file_open_write(const std::string &_name);
    int file_close();
    bool is_open();
    long long get_pos();
    int seekto(long long pos);
    size_t size();
    std::string extension(); 
    std::string type();
};

#endif // _FILE_HPP_
