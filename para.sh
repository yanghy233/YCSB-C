#!/bin/bash

rm para.log

for i in $(seq 1 10)
	do
	for j in $(seq 1 10)
		do
			make clean >> para.log
			sleep 1m
			cur_time=$(date +%m-%d,%H:%M:%S)
			echo $cur_time
			echo $cur_time >> para.log
			echo i=$i j=$j
			k1=$(echo $i | awk '{ printf "%0.1f\n" ,$1 * 0.1}')
			k2=$(echo $j | awk '{ printf "%0.1f\n" ,$1 * 0.1}')
			echo k1=$k1 k2=$k2
			make >>para.log
			echo $k1 $k2 > input
			echo $k1 $k2 >> input
			echo $k1 $k2 >> input
			more input >> para.log
			./ycsbc -db cruisedb -threads 100 -P workloads/workloadtest.spec <input 1>para_res/${k1}_${k2}.out 2>para_res/${k1}_${k2}.txt
	done
done

rm input