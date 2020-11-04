#!/bin/python
# -*- coding: UTF-8 -*-

import sys
import xlrd
import pygal
import string
import getopt
import datetime

def main(argv):
    xlsfile = ''
    pid = []
    outputfile = ''
    type = ''
    try:
        opts, args = getopt.getopt(
            argv, "-h-xls:-pid-type:-output:", ["xls=", "pid=", "type=", "output="])
    except getopt.GetoptError:
        print('excel.py -xls <xls file> -pid <pid> -output <outputfile>')
        sys.exit(2)

    for opt, arg in opts:
        if opt == '-h':
            print('excel.py -xls <xls file> -pid <pid> -output <outputfile>')
            sys.exit()
        elif opt in ("-xls", "--xls"):
            xlsfile = arg
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

    wb = xlrd.open_workbook(xlsfile)
    table = wb.sheets()[0]
    line = pygal.Line()

    if not pid:
        for r in range(table.nrows):
            pid.append(str(int(table.row_values(r)[1])))
        pid = list(set(pid))

    for n in pid:
        tmp_list = []
        time_list = []
        name_ = ""
        lastReadIO = 0
        lastWriteIO = 0
        isFind = False
        for r in range(table.nrows):
            row = table.row_values(r)
            time = row[0]
            pid = str(int(row[1]))
            name = row[2]
            cpu_usage = row[3]
            vmrss = row[4]
            readio = row[5]
            writeio = row[6]

            if cpu_usage < 1:
                continue

            time_list.append(datetime.datetime.fromtimestamp(time))

            if (pid == str(n)):
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

    if  type == "png":
        line.render_to_png(outputfile)

if __name__ == "__main__":
   main(sys.argv[1:])
