#include <iostream>
#include <map>
#include <algorithm>
#include <list>
#include <thread>

#include <QDir>
#include <QFileInfo>
#include <unistd.h>

#include <QFile>
#include <QTextStream>

#include "processstat.hpp"
#include "procstat.hpp"

bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
                                      s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

int main(int argc, char *argv[])
{
    QFile file("/tmp/analysis.csv");
    if (!file.open(QIODevice::Text | QIODevice::WriteOnly)) {
        qErrnoWarning("cannot create file!");
        return -1;
    }

    QTextStream stream(&file);

    int total_cpu_time_ = 0;
    std::map<time_t, std::map<int, double>> list;

    const int pid = getpid();

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

            // if (info.fileName().toInt() < pid) {
            //     continue;
            // }

            ProcessStat::Ptr process(new ProcessStat(info.fileName().toStdString()));
            map[process->pid].first = process;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        procList = procDir.entryInfoList();
        for (const QFileInfo& info : procList) {
            if (!is_number(info.fileName().toStdString())) {
                continue;
            }

            if (info.fileName().toInt() < pid) {
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

            if (!it->second.second->name.empty()) {
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

                stream << QString("%1,%2,%3,%4,%5,%6,%7")
                              .arg(time_)
                              .arg(pid)
                              .arg(QString::fromStdString(it->second.second->name))
                              .arg(usage)
                              .arg(QString::number(it->second.second->VmRSS))
                              .arg(ReadIO)
                              .arg(WriteIO)
                       << "\n";
                stream.flush();
            }
        }

        total_cpu_time_ = total_time;
    }

    return 0;
}
