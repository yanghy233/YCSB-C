# YCSB-C

Yahoo! Cloud Serving Benchmark in C++, a C++ version of YCSB (https://github.com/brianfrankcooper/YCSB/wiki)

## Quick Start

To build YCSB-C on Ubuntu, for example:

```
sudo apt-get install libtbb-dev
git clone git@github.com:yanghy233/YCSB-C.git
cd YCSB-C
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
make
mkdir ramdisk_path rocksdb_disk_path
./ycsba.sh
```

Run Workload A with a Rocksdb -based
implementation of the database, for example:
```
./ycsbc -db rocksdb -threads 32 -P workloads/workloada.spec 1>res/w1.out 2>res/w1.txt
```

Also reference run.sh and run\_redis.sh for the command line. See help by
invoking `./ycsbc` without any arguments.

Note that we do not have load and run commands as the original YCSB. Specify
how many records to load by the recordcount property. Reference properties
files in the workloads dir.

## Supported Databases
- CruiseDB

