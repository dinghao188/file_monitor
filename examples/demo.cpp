#include "file_monitor.h"
#include <thread>

using namespace std;

void ProcessOneFile(const string &filename) {
    printf("TODO: what will you do to [%s]?\n", filename.c_str());
}

int main() {
    FileMonitor fmon;
    if (!fmon.Initialize()) {
        return 1;
    }

    fmon.AddDirectory("tmp", ProcessOneFile, ProcessOneFile);
    fmon.AddDirectory("tmp1", ProcessOneFile, ProcessOneFile);

    while (1) {
        fmon.Execute();
        this_thread::sleep_for(1s);
    }
}