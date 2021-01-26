#!/bin/bash

rm mem.log

for i in $(seq 1 8)
	do
	make clean >> mem.log
	sleep 1m
	cur_time=$(date +%m-%d,%H:%M:%S)
	echo $cur_time
	echo $cur_time >> mem.log
	echo i=$i
	make >>mem.log
	echo $i > input
	echo $i >> input
	echo $i >> input
	more input
	more input >> mem.log
	./ycsbc -db rocksdb -threads 100 -P workloads/workloadtest.spec <input 1>mem_res/${i}.out 2>mem_res/${i}.txt 
done

rm input