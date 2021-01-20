#include <sys/inotify.h>
#include <unistd.h>
#include <cstdio>
#include <sstream>
#include <unordered_set>

#include "file_monitor.h"
#include "file_info.h"

FileMonitor::FileMonitor() {
    notifier_fd_ = -1;
}

FileMonitor::~FileMonitor() {
    if (notifier_fd_ < 0) {
        return;
    }
    close(notifier_fd_);
}

bool FileMonitor::Initialize() {
    notifier_fd_ = inotify_init();
    if (notifier_fd_ < 0) {
        perror("inotify_init");
        return false;
    }
    return true;
}

bool FileMonitor::AddDirectory(const char *dirname, 
    FileMonitor::ConsumeFileFuncType &&new_file_consume_func, FileMonitor::ConsumeFileFuncType &&mod_file_consume_func)
{
    uint32_t flags = IN_CREATE | IN_MOVED_TO | IN_MODIFY;
    int wd = inotify_add_watch(notifier_fd_, dirname, flags);
    if (wd < 0) {
        perror("inotify_add_watch");
        return false;
    }

    auto full_path_name = GetFullPathName(dirname);
    if (watching_files_.count(wd) != 0) {
        printf("WARNING: %s has been watched.\n", full_path_name.c_str());
        return false;
    }

    watching_files_[wd] = std::move(full_path_name);
    new_files_[wd] = std::make_shared<FileInfoVector>();
    mod_files_[wd] = std::make_shared<FileInfoVector>();
    new_file_consume_funcs_.emplace(wd, std::forward<ConsumeFileFuncType>(new_file_consume_func));
    mod_file_consume_funcs_.emplace(wd, std::forward<ConsumeFileFuncType>(mod_file_consume_func));

    return true;
}

std::string FileMonitor::GetFullPathName(const char *path) {
    char path_buf[1024] = { '\0' };
    const char *cwd = getenv("PWD");

    // just return the path
    if (path == nullptr || path[0] == '\0' || path[0] == '/') {
        return path;
    }

    snprintf(path_buf, sizeof(path_buf), "%s/%s", cwd, path);
    return path_buf;
}

void FileMonitor::ConsumeOneWd(int wd) {
    auto files = new_files_[wd];
    if (files == nullptr) {
        printf("%d not watched\n", wd);
        return;
    }
    auto &new_file_consume_f = new_file_consume_funcs_[wd];
    for (const auto &file : *files) {
        new_file_consume_f(file.filename);
    }
    files->clear();

    files = mod_files_[wd];
    auto &mod_file_consume_f = mod_file_consume_funcs_[wd];
    for (const auto &file : *files) {
        mod_file_consume_f(file.filename);
    }
    files->clear();
}

void FileMonitor::NotifierCallback() {
    static char events_buf[MAX_EVENTS*(sizeof(inotify_event)+FILENAME_LEN)] = { 0 };
    static std::unordered_set<int> wds;
    
    wds.clear();
    ssize_t len = read(notifier_fd_, events_buf, sizeof(events_buf));

    ssize_t cur_event_pos = 0;
    inotify_event *event = nullptr;
    while (cur_event_pos < len) {
        event = reinterpret_cast<inotify_event*>(events_buf+cur_event_pos);
        wds.insert(event->wd);
        auto &watch_point_path = watching_files_[event->wd];
        FileInfo file_info = {event->name, (event->mask & IN_ISDIR) != 0 };

        if (event->mask & IN_CREATE | event->mask & IN_MOVED_TO) {
            auto files_vector = new_files_[event->wd];
            printf("%s [%s] in [%s] was created!\n", file_info.dir_flag ? " dir" : "file", event->name, watch_point_path.c_str());
            files_vector->emplace_back(std::move(file_info));
        } else if (event->mask & IN_MODIFY) {
            auto &files_vector = mod_files_[event->wd];
            printf("%s [%s] in [%s] was modified!\n", file_info.dir_flag ? " dir" : "file", event->name, watch_point_path.c_str());
            files_vector->emplace_back(std::move(file_info));
        }

        cur_event_pos += sizeof(inotify_event)+event->len;
    }

    for (auto wd : wds) {
        ConsumeOneWd(wd);
    }
}