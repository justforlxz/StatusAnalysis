#ifndef PROCSTAT_H
#define PROCSTAT_H

#include <fstream>

static int total_cpu_time() {
    std::ifstream input;
    input.open("/proc/stat", std::ios_base::in);
    if (input.is_open()) {
        std::string cpu;
        int user, nice, system, idle, iowait, irq, softirq, stealstolen, guest;
        input >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> stealstolen >> guest;
        input.close();
        const int total_time = user + nice + system + idle + irq + softirq + guest + iowait + stealstolen;
        return total_time;
    }

    return -1;
}

#endif // PROCSTAT_H
