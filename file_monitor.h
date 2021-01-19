#ifndef FILE_MONITOR_H
#define FILE_MONITOR_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <sys/inotify.h>

class FileInfo;

class FileMonitor {
public:
    using FileInfoVector = std::vector<FileInfo>;
    static const size_t FILENAME_LEN = 1024;
    static const size_t MAX_EVENTS = 1024;

public:
    FileMonitor();
    ~FileMonitor();

    bool Initialize();

    bool AddDirectory(const char *dirname, uint32_t flag = IN_CREATE);

    void Execute() { NotifierCallback(); }

    void Consume(const char *dir);

protected:
    std::string GetFullPathName(const char *path);
    
    void NotifierCallback();

private:
    //inotify instance
    int notifier_fd_;
    //watched files
    std::unordered_map<int, std::string> watching_files_;
    //new created files
    std::unordered_map<int, std::shared_ptr<FileInfoVector>> new_created_files_;
    //modified files 
    std::unordered_map<int, std::shared_ptr<FileInfoVector>> modified_files_;
};

#endif