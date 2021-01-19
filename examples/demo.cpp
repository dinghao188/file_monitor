#include "file_monitor.h"
#include <thread>

using namespace std;

int main() {
    FileMonitor fmon;
    if (!fmon.Initialize()) {
        return 1;
    }

    fmon.AddDirectory("tmp");
    fmon.AddDirectory("tmp1");

    while (1) {
        fmon.Execute();
        this_thread::sleep_for(1s);
    }
}