#!/bin/bash

rm -rf ./res ./ramdisk_path ./rocksdb_disk_path ./ycsb-leveldb
mkdir res ramdisk_path rocksdb_disk_path ycsb-leveldb

# benchmark
./ycsbc -db rocksdb -threads 100 -P workloads/workload10GB.spec 1>res/w1.out 2>res/w1.txt
#./ycsbc -db cruisedb -threads 100 -P workloads/workload10GB.spec 1>res/w1.out 2>res/w1.txt
#./ycsbc -db leveldb -threads 32 -P workloads/workload10GB.spec 1>res/w1.out 2>res/w1.txt
