#include <iostream>
#include <map>
#include <algorithm>
#include <list>
#include <thread>

#include <QDir>
#include <QFileInfo>
#include <unistd.h>
#include <QDebug>

#include <QFile>
#include <QTextStream>

#include <QCoreApplication>
#include <QCommandLineParser>

#include "processstat.hpp"
#include "procstat.hpp"

// 二进制路径+pid作为一个进程的跟踪记录

bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
                                      s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    QCommandLineOption csvOption("c", "csv file path", "csv");
    QCommandLineOption intervalOption("i", "The sampling interval", "interval", "1");
    QCommandLineOption pidOption("pid", "Sampling procedure", "pid");
    QCommandLineOption samplingAll("all", "sampling all program");
    QCommandLineOption debugOption("d", "enable debug mode", "debug");
    parser.addHelpOption();
    parser.addOption(csvOption);
    parser.addOption(intervalOption);
    parser.addOption(pidOption);
    parser.addOption(samplingAll);
    parser.addOption(debugOption);
    parser.process(app);

    if (!parser.isSet(csvOption)) {
        qErrnoWarning("not set csv file path.");
        return -1;
    }

    const bool isAll = parser.isSet(samplingAll);
    const bool isSetPid = parser.isSet(pidOption);
    const bool isDebug = parser.isSet(debugOption);

    if (!isAll && !isSetPid) {
        qErrnoWarning("not set sampling action. all or pid.");
        return -1;
    }

    QFile file(parser.value(csvOption));
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

            if (!isAll && info.fileName() != parser.value(pidOption)) {
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

            if (!isAll && info.fileName() != parser.value(pidOption)) {
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

                if (isDebug) {
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
