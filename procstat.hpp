#ifndef PROCSTAT_H
#define PROCSTAT_H

#include <fstream>

class CPUTime {
public:
    CPUTime() {
        std::ifstream input;
        input.open("/proc/stat", std::ios_base::in);
        if (input.is_open()) {
            std::string cpu;
            int user, nice, system, idle, iowait, irq, softirq, stealstolen, guest;
            input >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> stealstolen >> guest;
            input.close();
        }
    };

    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long stealstolen;
    unsigned long long guest;

    unsigned long long totalTime() const
    {
        return (user + nice + system + idle + iowait + irq + softirq + stealstolen + guest) / 100;
    }
};

#endif // PROCSTAT_H
