#include <sys/inotify.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <sstream>
#include <unordered_set>

#include "file_monitor.h"
#include "file_info.h"

namespace fmon
{

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
    //get a inotify instance
    notifier_fd_ = inotify_init();
    if (notifier_fd_ < 0) {
        perror("inotify_init");
        return false;
    }
    //set nonblocking
    auto fl = fcntl(notifier_fd_, F_GETFL);
    fl |= O_NONBLOCK;
    fcntl(notifier_fd_, F_SETFL, fl);
    return true;
}

bool FileMonitor::AddDirectory(const char *dirname, ConsumeFileFuncType &&file_consume_func, uint32_t flags)
{
    int wd = inotify_add_watch(notifier_fd_, dirname, flags);
    if (wd < 0) {
        perror("inotify_add_watch");
        return false;
    }

    auto full_path_name = GetFullPathName(dirname);
    if (watch_points_.count(wd) != 0) {
        printf("WARNING: %s has been watched.\n", full_path_name.c_str());
        return false;
    }

    watch_points_[wd] = { full_path_name, flags };
    files_[wd] = std::make_shared<FileInfoVector>();
    file_consume_funcs_[wd] = std::move(file_consume_func);

    printf("watching for [%s] ... ...\n", full_path_name.c_str());
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
    auto files = files_[wd];
    if (files == nullptr) {
        printf("%d not watched\n", wd);
        return;
    }
    auto &new_file_consume_f = file_consume_funcs_[wd];
    new_file_consume_f(files);
    files_[wd] = std::make_shared<FileInfoVector>();
}

void FileMonitor::NotifierCallback() {
    static char events_buf[MAX_EVENTS*(sizeof(inotify_event)+FILENAME_MAX)] = { 0 };
    static std::unordered_set<int> wds;
    
    wds.clear();
    ssize_t len = read(notifier_fd_, events_buf, sizeof(events_buf));

    //read all events from notifier
    ssize_t cur_event_pos = 0;
    inotify_event *event = nullptr;
    while (cur_event_pos < len) {
        event = reinterpret_cast<inotify_event*>(events_buf+cur_event_pos);
        wds.insert(event->wd);
        auto &watch_point = watch_points_[event->wd];
        FileInfo file_info = {event->name, (event->mask & IN_ISDIR) != 0 };

        if ((event->mask & watch_point.interested_events) != 0) {
            auto files_vector = files_[event->wd];
            files_vector->emplace_back(std::move(file_info));
        }
        cur_event_pos += sizeof(inotify_event)+event->len;
    }

    //consume all collected files this time
    for (auto wd : wds) {
        ConsumeOneWd(wd);
    }
}

}