#ifndef FILE_MONITOR_H
#define FILE_MONITOR_H

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <sys/inotify.h>

namespace fmon
{

class FileInfo;
using FileInfoVector = std::vector<FileInfo>;
using ConsumeFileFuncType = std::function<void(std::shared_ptr<FileInfoVector>)>;

struct WatchPoint {
    WatchPoint(const std::string &pathname = "", uint32_t interested_events = 0)
        : pathname(pathname), interested_events(interested_events) {}

    std::string pathname;
    uint32_t interested_events;
};

class FileMonitor {
public:
    static const size_t FILENAME_LEN = 1024;
    static const size_t MAX_EVENTS = 1024;

public:
    FileMonitor();
    ~FileMonitor();

    bool Initialize();

    bool AddDirectory(const char *dirname, ConsumeFileFuncType &&file_consume_func, uint32_t flags);

    void Execute() { NotifierCallback(); }

protected:
    std::string GetFullPathName(const char *path);
    
    void NotifierCallback();

    //consume the files watched by wd
    void ConsumeOneWd(int wd);

private:
    //inotify instance
    int notifier_fd_;
    //watched files and dirs
    std::unordered_map<int, WatchPoint> watch_points_;
    //files collected by notifier
    std::unordered_map<int, std::shared_ptr<FileInfoVector>> files_;

    //the function to consume the files
    std::unordered_map<int, ConsumeFileFuncType> file_consume_funcs_;
};

}
#endif