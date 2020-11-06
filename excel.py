#!/bin/python
# -*- coding: UTF-8 -*-

import sys
import pygal
import string
import getopt
import datetime
import csv
from numpy import *

class CSVClass:
    time = 0
    pid = 0
    name = ""
    cpu_usage = 0
    vmrss = 0
    readio = 0
    writeio = 0

def main(argv):
    csvfiles = []
    pids = []
    outputfile = ''
    type = ''
    try:
        opts, args = getopt.getopt(
            argv, "-h-csv:-pid-type:-output:", ["csv=", "pid=", "type=", "output="])
    except getopt.GetoptError:
        print('excel.py -csv <csv file> --type <type> -pid <pid> -output <outputfile>')
        sys.exit(2)

    for opt, arg in opts:
        if opt == '-h':
            print(
                'excel.py -csv <csv file> --type <type> -pid <pid> -output <outputfile>')
            sys.exit()
        elif opt in ("-csv", "--csv"):
            csvfiles.append(arg)
        elif opt in ("-output", "--output"):
            outputfile = arg
        elif opt in ("-column", "--column"):
            column = arg
        elif opt in ("-pid", "--pid"):
            pids.append(arg)
        elif opt in ("-type", "--type"):
            type = arg

    if type != "svg" and type != "png":
        print("type only svg or png")
        sys.exit()

    if not pids:
        for csvfile in csvfiles:
            with open(csvfile, newline='') as file:
                spamreader = csv.reader(file, delimiter=",")
                for row in spamreader:
                    pids.append(str(int(row[1])))
                pids = list(set(pids))

    data = {}
    result = {}
    for csvfile in csvfiles:
        with open(csvfile, newline='') as file:
            spamreader = csv.reader(file, delimiter=",")
            array = []
            for row in spamreader:
                csvObj = CSVClass()
                csvObj = row
                array.append(csvObj)
            data[csvfile] = array
            result[csvfile] = []

    line = pygal.Line()
    time_list = []
    # for n in pids:
    tmp_list = []
    index = 0
    rec_csvfiles = csvfiles
    firstTime = {}
    while (True):
        if len(rec_csvfiles) == 0:
            break
        times = []
        for key in data.keys():
            value = data[key]
            if (len(value) <= index):
                if key in rec_csvfiles:
                    rec_csvfiles.remove(key)
                continue
            cpu_usage = float(value[index][3])
            if not key in firstTime:
                firstTime[key] = float(value[index][0])
            times.append(float(value[index][0]) - firstTime[key])
            p = str(int(value[index][1]))
            tmp_list = []
            tmp_list.extend(result[key])
            tmp_list.append(cpu_usage)
            result[key] = tmp_list
        for key in data.keys():
            if not key in rec_csvfiles:
                data.pop(key)
                break
        time_list.append(mean(times))
        index += 1

    for key in result.keys():
        line.add(key, result[key])
    line.x_labels = time_list

    line.legend_at_bottom = True
    if type == "svg":
        line.render_to_file(outputfile)
    if type == "png":
        line.render_to_png(outputfile)

if __name__ == "__main__":
    main(sys.argv[1:])
