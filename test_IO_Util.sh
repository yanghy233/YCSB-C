#!/bin/bash

OUTPUT_FILE="./io_record.txt"

# 监控进程的PID
YCSB_PID=$(pidof ./ycsbc)

# 设备名称，比如 vda, 可以通过 iostat 确定目标设备
DEVICE="vda"

rm "$OUTPUT_FILE"

# 每秒进行一次监控
while true; do
    # 获取当前时间戳
    TIMESTAMP=$(date)

    # 获取进程的pid (防止ycsb进程重启PID变化)
    YCSB_PID=$(pidof ./ycsb)

    # 使用 iostat 获取设备的 I/O 统计信息
    IO_STAT=$(iostat -dx 1 2 | grep "$DEVICE" | tail -n1)

    # 提取相关的字段（例如 %util）
    RS=$(echo "$IO_STAT" | awk '{print $4}')  # r/s
    RKBPS=$(echo "$IO_STAT" | awk '{print $5}')  # rkB/s
    WRITES=$(echo "$IO_STAT" | awk '{print $8}')  # w/s
    WKBPS=$(echo "$IO_STAT" | awk '{print $9}')  # wkB/s
    AWAIT=$(echo "$IO_STAT" | awk '{print $10}')  # w_await
    UTIL=$(echo "$IO_STAT" | awk '{print $15}')   # %util

    # 将数据写入文件
    echo "./ycsbc: 磁盘每秒读取次数: $RS reads, 读带宽: $RKBPS KB/s, 磁盘每秒写入次数: $WRITES writes, 写带宽: $WKBPS KB/s, %util: $UTIL" >> "$OUTPUT_FILE"

    # 等待1秒后再次执行
    sleep 1
done

