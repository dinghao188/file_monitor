#include <thread>
#include <map>
#include "file_monitor.h"
#include "file_info.h"

using namespace std;

void Process(shared_ptr<fmon::FileInfoVector> files) {
    static uint32_t count = 0;
    printf("-------------the no.%u time toggled-------------\n", ++count);
    for (const auto &file_info : *files) {
        printf("[TODO] what will you do to [%s]?\n", file_info.filename.c_str());
    }
}

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);

    fmon::FileMonitor fm;
    if (!fm.Initialize()) {
        return 1;
    }

    fm.AddDirectory("/root/workspace/jobs", Process, IN_CREATE|IN_MOVED_TO);

    while (1) {
        fm.Execute();
        this_thread::sleep_for(1s);
    }
}