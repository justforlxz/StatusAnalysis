#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <map>
#include <filesystem>
#include <algorithm>
#include <list>

#include "processstat.h"
#include "procstat.h"

bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
                                      s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

int main(int argc, char *argv[])
{
    int total_cpu_time_ = 0;
    std::map<time_t, std::map<int, double>> list;

    while (true) {
        const int total_time{ total_cpu_time()};
        time_t time_ = time(nullptr);
        std::map<int, std::pair<ProcessStat::Ptr, ProcessStat::Ptr>> map;
        std::map<ProcessStat::Ptr, double> tmp;

        for(auto& p: std::filesystem::directory_iterator("/proc")) {
            if (!is_number(p.path().filename())) {
                continue;
            }

            ProcessStat::Ptr process(new ProcessStat(p.path().filename()));
            map[process->pid].first = process;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        for(auto& p: std::filesystem::directory_iterator("/proc")) {
            if (!is_number(p.path().filename())) {
                continue;
            }

            ProcessStat::Ptr process(new ProcessStat(p.path().filename()));
            // 清理不存在的任务
            if (ProcessStat::Ptr t = map[process->pid].first) {
                map[process->pid].second = process;
            }
            else {
                map.erase(process->pid);
            }
        }

        // 已经得到两个差异
        for (auto it = map.begin(); it != map.end(); ++it) {
            if (!it->second.second) {
                continue;
            }

            constexpr auto add = [](const ProcessStat::Ptr& ptr) -> long {
                return ptr->utime + ptr->stime + ptr->cutime + ptr->cstime;
            };

            constexpr auto diff = [add](const ProcessStat::Ptr& ptr1, const ProcessStat::Ptr& ptr2) {
                return add(ptr2) - add(ptr1);
            };

            const int pid = it->second.second->pid;
            const double usage{ (8
                                * diff(it->second.first, it->second.second)
                        * 100
                        / static_cast<double>(total_time - total_cpu_time_)) };
            list[time_][pid] = usage;
        }

        total_cpu_time_ = total_time;
    }

    return 0;
}
