#!/bin/bash

rm ycsba.log

for i in $(seq 0 4)
	do
	./ycsbc -db rocksdb -threads 100 -P workloads/a${i}.spec 1>res/a${i}.out 2>res/a${i}.txt
done
