#ifndef FILE_INFO_H
#define FILE_INFO_H

#include <string>

namespace fmon
{

struct FileInfo {
    FileInfo(std::string &&filename = "", bool dir_flag = false) : filename(filename), dir_flag(dir_flag) {}
    std::string filename;
    bool dir_flag;
};

}
#endif