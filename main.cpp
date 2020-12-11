#include <iostream>
#include <map>
#include <algorithm>
#include <list>
#include <thread>

#include <QDir>
#include <QFileInfo>
#include <unistd.h>
#include <QDebug>
#include <QProcess>
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
    QCommandLineOption intervalOption("i", "The sampling interval, seconds", "interval", "5");
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

    QStringList arguments = parser.positionalArguments();

    if (!parser.isSet(csvOption)) {
        qErrnoWarning("not set csv file path.");
        return -1;
    }

    const bool isAll = parser.isSet(samplingAll);
    const bool isSetPid = parser.isSet(pidOption);
    const bool isDebug = parser.isSet(debugOption);
    const uint interval = parser.value(intervalOption).toUInt();

    if (arguments.isEmpty() && !isAll && !isSetPid) {
        qErrnoWarning("not set sampling action. all or pid.");
        return -1;
    }

    QFile file(parser.value(csvOption));
    if (!file.open(QIODevice::Text | QIODevice::WriteOnly)) {
        qErrnoWarning("cannot create file!");
        return -1;
    }

    QTextStream stream(&file);

    std::map<time_t, std::map<int, double>> list;

    QString pid;

    if (!arguments.isEmpty()) {
        QProcess* process = new QProcess;
        QStringList list = arguments.takeFirst().split(" ");
        process->setProgram(list.takeFirst());
        process->setArguments(list + arguments);
        process->start();
        pid = QString::number(process->pid());
//        QObject::connect(process, static_cast<void (QProcess::*)(int)>(&QProcess::finished), qApp, &QCoreApplication::quit);
    }
    else {
        pid = parser.value(pidOption);
    }

    const int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);

    while (true) {
        const CPUTime cpu1;

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

            if (!isAll && info.fileName() != pid) {
                continue;
            }

            ProcessStat stat(info.fileName().toStdString());

            if (stat.name.empty()) {
                continue;
            }

            constexpr auto add = [](ProcessStat stat) -> unsigned long long {
                return stat.utime + stat.stime + stat.cutime + stat.cstime;
            };

            const QString &result {
                QString("%1,%2,%3,%4")
                    .arg(time_)
                    .arg(info.fileName())
                    .arg(QString::fromStdString(stat.name))
                    .arg(QString::number(add(stat)))};

            stream
                << result
                << "\n";

            qDebug() << result;
            stream.flush();
        }

        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }

    return 0;
}
