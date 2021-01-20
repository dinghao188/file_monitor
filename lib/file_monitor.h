#ifndef FILE_MONITOR_H
#define FILE_MONITOR_H

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <sys/inotify.h>

class FileInfo;

class FileMonitor {
public:
    using FileInfoVector = std::vector<FileInfo>;
    using ConsumeFileFuncType = std::function<void(const std::string &filename)>;
    static const size_t FILENAME_LEN = 1024;
    static const size_t MAX_EVENTS = 1024;

public:
    FileMonitor();
    ~FileMonitor();

    bool Initialize();

    bool AddDirectory(const char *dirname,
                      ConsumeFileFuncType &&new_file_consume_func,
                      ConsumeFileFuncType &&mod_file_consume_func);

    void Execute() { NotifierCallback(); }

protected:
    std::string GetFullPathName(const char *path);
    
    void NotifierCallback();

    //consume the files watched by wd
    void ConsumeOneWd(int wd);

private:
    //inotify instance
    int notifier_fd_;
    //watched files
    std::unordered_map<int, std::string> watching_files_;
    //new created files
    std::unordered_map<int, std::shared_ptr<FileInfoVector>> new_files_;
    //modified files 
    std::unordered_map<int, std::shared_ptr<FileInfoVector>> mod_files_;

    //the function to consume new created files and modified files
    std::unordered_map<int, ConsumeFileFuncType> new_file_consume_funcs_;
    std::unordered_map<int, ConsumeFileFuncType> mod_file_consume_funcs_;
};

#endif