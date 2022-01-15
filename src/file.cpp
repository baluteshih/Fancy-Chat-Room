#include "file.hpp"
#include "helper.hpp"
#include <cstdio>

std::string path_combine(std::string path1, std::string path2) {
    while (!path2.empty() && path2[0] == '/')
        path2.erase(path2.begin());
    return path1 + "/" + path2;
}

File::File() : fp(NULL) {}

size_t File::readf(void *buf, size_t count) {
    return fread(buf, 1, count, fp);   
}

size_t File::writef(void *buf, size_t count) {
    size_t rt = fwrite(buf, 1, count, fp);
    fflush(fp);
    return rt;
}

int File::file_open_read(const std::string &_name) {
    file_name = _name;
    if ((fp = fopen(file_name.c_str(), "rb")) == NULL) {
        _helper_warning2("error on opening " + file_name, 1);
        return -1;
    }
    return 0;
}

int File::file_open_write(const std::string &_name) {
    file_name = _name;
    if ((fp = fopen(file_name.c_str(), "wb")) == NULL) {
        _helper_warning2("error on opening " + file_name, 1);
        return -1;
    }
    return 0;
}

int File::file_close() {
    int rt;
    if ((rt = fclose(fp)) == EOF) {
        _helper_warning2("error on closing " + file_name, 1);
        return -1;
    }
    fp = NULL;
    return 0;
}

bool File::is_open() {
    return fp != NULL;
}

size_t File::size() {
    size_t tmp = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    size_t sz = ftell(fp);
    fseek(fp, tmp, SEEK_SET);
    return sz;
}

std::string File::extension() {
    if (!is_open())
        return "";
    std::string rt;
    // This is a very bad method, but I give up
    for (char c : file_name)
        if (c == '.')
            rt.clear();
        else
            rt.push_back(c);
    return rt;
}

std::string File::type() {
    if (!is_open())
        return "";
    std::string ext = extension();
    for (std::string str : {
        "asp",
        "aspx",
        "cer",
        "cfm",
        "cgi",
        "pl",
        "css",
        "htm",
        "html",
        "js",
        "jsp",
        "part",
        "php",
        "py",
        "rss",
        "xhtml",
        "c",
        "class",
        "cpp",
        "cs",
        "h",
        "hpp",
        "java",
        "sh",
        "swift",
        "vb",
        "txt",
        "json"
    }) {
        if (ext == str)
            return "text";
    }
    for (std::string str : {
        "jpeg",
        "jpg",
        "png",
        "gif",
        "ai",
        "bmp",
        "ico",
        "svg",
        "tiff",
        "tif",
        "ps",
        "psd"
    }) {
        if (ext == str)
            return "image";
    }
    return "application";
}
