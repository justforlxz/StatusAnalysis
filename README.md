使用status-analysis进行数据采集，再通过excel.py进行svg报表生成。

例如以下例子：

使用status-analysis采集数据

```shell
./status-analysis -c /tmp/test.csv "dde-control-center --show"
```

使用excel.py对采集的数据进行svg报表生成

```shell
python3 excel.py --csv /tmp/test.csv --type svg --output /tmp/test.svg
```
