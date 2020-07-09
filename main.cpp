#include <iostream>
#include <map>
#include <algorithm>
#include <list>
#include <thread>

#include <QDir>
#include <QFileInfo>

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

        QDir procDir("/proc");
        procDir.setFilter(QDir::Dirs);

        QFileInfoList procList{ procDir.entryInfoList() };
        for (const QFileInfo& info : procList) {
            if (!is_number(info.fileName().toStdString())) {
                continue;
            }

            ProcessStat::Ptr process(new ProcessStat(info.fileName().toStdString()));
            map[process->pid].first = process;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        procList = procDir.entryInfoList();
        for (const QFileInfo& info : procList) {
            if (!is_number(info.fileName().toStdString())) {
                continue;
            }

            ProcessStat::Ptr process(new ProcessStat(info.fileName().toStdString()));
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
            const long ReadIO{
                it->second.second->ReadIO - it->second.first->ReadIO
            };

            const long WriteIO{
                it->second.second->WriteIO - it->second.first->WriteIO
            };

            list[time_][pid] = usage;
            std::cout
            << time_ << ","
            << pid << ","
            << it->second.second->name << ","
            << usage << ","
            << it->second.second->VmRSS  << ","
            << ReadIO << ","
            << WriteIO
            << std::endl;
        }

        total_cpu_time_ = total_time;
    }

    return 0;
}
