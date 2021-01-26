#!/bin/bash

rm ycsba.log

for i in $(seq 0 4)
	do
	make clean >> ycsba.log
	sleep 1m
	cur_time=$(date +%m-%d,%H:%M:%S)
	echo $cur_time
	echo $cur_time >> ycsba.log
	make >> ycsba.log
	./ycsbc -db rocksdb -threads 100 -P workloads/a${i}.spec 1>res/a${i}.out 2>res/a${i}.txt
done