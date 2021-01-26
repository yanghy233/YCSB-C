# !/bin/bash

# for i in $(seq 0 3)
# 	do
# 	make clean >> scale.log
# 	sleep 1m
# 	cur_time=$(date +%m-%d,%H:%M:%S)
# 	echo $cur_time
# 	echo $cur_time >> scale.log
# 	make >> scale.log
# 	./ycsbc -db rocksdb -threads 100 -P workloads/scalea${i}.spec 1>scale_res/avg${i}.out 2>scale_res/avg${i}.txt
# done

# make clean >> scale.log

make clean
sleep 3s
make
./ycsbc -db rocksdb -threads 100 -P workloads/workloadtest.spec 1>123.out 2>123.txt
make clean