#!/bin/bash

rm -rf ./res ./ramdisk_path ./rocksdb_disk_path
mkdir res ramdisk_path rocksdb_disk_path
# only for insert benchmark
./ycsbc -db rocksdb -threads 100 -P workloads/a0.spec 1>res/a0.out 2>res/a0.txt
