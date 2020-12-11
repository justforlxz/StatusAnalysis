#!/bin/python

import sys
from typing import ItemsView
import pygal
import string
import getopt
import datetime
import csv
from numpy import *

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
            csvObj.name = row[2]
            csvObj.cpu_usage = row[3]
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

    # 基于时间进行遍历
    # time: [pid][cpu]
    timemap: dict[int, list[CSVClass]] = {}
    for key in list:
        time = int(key.time)
        if time not in timemap.keys():
            timemap[time] = []
        timemap[time].append(key)
    graph = pygal.Line()
    graph.x_labels = []

    # 遍历timemap,生成所有pid的信息
    result:dict[str, list[int]] = {}

    index = -1
    for key, value in timemap.items():
        index += 1
        if len(timemap) > 100 and index % 90 != 0:
            continue
        value: list[CSVClass] = value
        for item in value:
            item: CSVClass = item
            if item.pid not in result.keys():
                result[item.pid] = []
            else:
                if result[item.pid][0] == int(item.cpu_usage):
                    continue
            result[item.pid].append(int(item.cpu_usage))
        graph.x_labels.append(key)

    for index, key in enumerate(result):
        tmp = result[key]
        result[key] = [tmp[index] - tmp[index - 1]
                       for index in range(1, len(tmp))]
    graph.x_labels = graph.x_labels[0:len(graph.x_labels) - 1]

    processMap: dict[str, str] = {}
    for item in list:
        processMap[item.pid] = item.name.split('/')[::-1][0]

    for key, value in result.items():
        if len(value) < 2:
            continue
        graph.add(str(processMap[key]), value)

    graph.legend_box_size = 5
    # graph.legend_at_bottom = True
    # graph.legend_at_bottom_columns = 1
    if type == "svg":
        graph.render_to_file(outputfile)
    if type == "png":
        graph.render_to_png(outputfile)

if __name__ == "__main__":
    main(sys.argv[1:])
