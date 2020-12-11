#!/bin/python
# -*- coding: UTF-8 -*-

import sys
import getopt
import csv
from numpy import *
import matplotlib.pyplot as plt

class CSVClass:
    time: int = ""
    pid: int = ""
    name: str = ""
    cpu_usage: str = ""

def convertCSVToArray(csvfile: str):
    result:list[CSVClass] = []
    with open(csvfile, newline='') as file:
        spamreader = csv.reader(file, delimiter=",")
        for row in spamreader:
            csvObj = CSVClass()
            csvObj.time = row[0]
            csvObj.pid = row[1]
            csvObj.cpu_usage = row[2]
            csvObj.name = row[3]
            result.append(csvObj)
    return result

def main(argv):
    csvfiles:list[str] = []
    pids:list[int] = []
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

    list = convertCSVToArray(csvfiles[0])

    # 保存成{x: {time: [], data: []}}
    class Result:
        times: list[str]
        data: list[str]
    data: dict[str, Result] = {}

    # 要知道有多少程序
    for key in list:
        if key.name not in data:
            data[key.name] = Result()
        data[key.name].times.append(key.time)
        data[key.name].data.append(key.cpu_usage)
    print(data)

if __name__ == "__main__":
    main(sys.argv[1:])
