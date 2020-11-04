#!/bin/python
# -*- coding: UTF-8 -*-

import sys
import pygal
import string
import getopt
import datetime
import csv


def main(argv):
    csvfile = ''
    pid = []
    outputfile = ''
    type = ''
    try:
        opts, args = getopt.getopt(
            argv, "-h-csv:-pid-type:-output:", ["csv=", "pid=", "type=", "output="])
    except getopt.GetoptError:
        print('excel.py -csv <csv file> -pid <pid> -output <outputfile>')
        sys.exit(2)

    for opt, arg in opts:
        if opt == '-h':
            print(
                'excel.py -csv <csv file> --type <type> -pid <pid> -output <outputfile>')
            sys.exit()
        elif opt in ("-csv", "--csv"):
            csvfile = arg
        elif opt in ("-output", "--output"):
            outputfile = arg
        elif opt in ("-column", "--column"):
            column = arg
        elif opt in ("-pid", "--pid"):
            pid.append(arg)
        elif opt in ("-type", "--type"):
            type = arg

    if type != "svg" and type != "png":
        print("type only svg or png")
        sys.exit()

    if not pid:
        with open(csvfile, newline='') as file:
            spamreader = csv.reader(file, delimiter=",")
            for row in spamreader:
                pid.append(str(int(row[1])))
            pid = list(set(pid))

    line = pygal.Line()
    for n in pid:
        tmp_list = []
        time_list = []
        name_ = ""
        lastReadIO = 0
        lastWriteIO = 0
        isFind = False
        with open(csvfile, newline='') as file:
            for row in csv.reader(file, delimiter=","):
                time = int(row[0])
                p = str(int(row[1]))
                name = row[2]
                cpu_usage = float(row[3])
                vmrss = row[4]
                readio = row[5]
                writeio = row[6]

                if cpu_usage < 0:
                    continue

                time_list.append(datetime.datetime.fromtimestamp(time))

                if (p == n):
                    tmp_list.append(cpu_usage)
                    name_ = name
                    lastReadIO = readio
                    lastWriteIO = writeio
                    isFind = True
                else:
                    tmp_list.append(None)

            if isFind:
                line.add(name_, tmp_list)
                line.x_labels = time_list

    line.legend_at_bottom = True
    if type == "svg":
        line.render_to_file(outputfile)
    if type == "png":
        line.render_to_png(outputfile)


if __name__ == "__main__":
    main(sys.argv[1:])
