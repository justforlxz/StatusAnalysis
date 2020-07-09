#ifndef PROCESSSTAT_H
#define PROCESSSTAT_H

#include <memory>
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <QString>
#include <QStringList>

class ProcessStat {
public:
    ProcessStat(int _pid)
        : ProcessStat(std::to_string(_pid))
    {
    }

    ProcessStat(const std::string _pid)
        : pid(std::stoi(_pid))
    {
        const std::string processPath = "/proc/" + _pid + "/stat";
        std::fstream statFile;
        statFile.open(processPath.data(), std::ios::in);
        if (statFile.is_open()) {
            statFile >> pid >> name >> state >> ppid >> pgid >> sid >> tty_nr
                    >> tty_pgrp >> flags >> min_flt >> cmin_flt >> maj_flt
                    >> cmaj_flt >> utime >> stime >> cutime >> cstime >> priority
                    >> nice >> num_threads >> zero >> start_time;
        }
        else {
            std::cout << processPath << _pid << std::endl;
        }

        std::fstream memFile;
        memFile.open("/proc/" + _pid + "/status", std::ios::in);
        if (memFile.is_open()) {
            std::string line;
            while (std::getline(memFile, line)) {
                const QString line_ {QString::fromStdString(line).simplified() };
                if (line_.startsWith("VmRSS:")) {
                    VmRSS = QString(line_.split(":").last()).toInt();
                    break;
                }
            }
        }
        else {
            std::cout << "mem failed" << std::endl;
        }

        std::fstream ioFile;
        ioFile.open("/proc/" + _pid + "/io", std::ios::in);
        if (memFile.is_open()) {
            std::string line;
            while (std::getline(ioFile, line)) {
                const QString line_ {QString::fromStdString(line).simplified() };
                if (line_.startsWith("rchar:")) {
                    ReadIO = QString(line_.split(":").last()).toDouble();
                    continue;
                }

                if (line_.startsWith("wchar:")) {
                    WriteIO = QString(line_.split(":").last()).toDouble();
                    continue;
                }
            }
        }
        else {
            std::cout << "io failed" << std::endl;
        }
    }

    typedef std::shared_ptr<ProcessStat> Ptr;
    std::string name;
    char state;
    int pid;
    int ppid;
    int pgid;
    int sid;
    int tty_nr;
    int tty_pgrp;
    int flags;
    int min_flt;
    int cmin_flt;
    int maj_flt;
    int cmaj_flt;
    int priority;
    int nice;
    long utime;
    long stime;
    long cutime;
    long cstime;
    int num_threads;
    int zero;
    unsigned long long start_time;

    // memory
    int VmRSS;
    long ReadIO;
    long WriteIO;
};

#endif // PROCESSSTAT_H
