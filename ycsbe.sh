#!/bin/bash

rm ycsbe.log

for i in $(seq 0 4)
	do
	make clean >> ycsbe.log
	sleep 1m
	cur_time=$(date +%m-%d,%H:%M:%S)
	echo $cur_time
	echo $cur_time >> ycsbe.log
	make >> ycsbe.log
	./ycsbc -db rocksdb -threads 100 -P workloads/e${i}.spec 1>res/e${i}.out 2>res/e${i}.txt
done