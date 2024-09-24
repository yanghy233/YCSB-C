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
./ycsbc -db cruisedb -threads 100 -P workloads/workloada.spec 1>res/w1.out 2>res/w1.txt
./ycsbc -db rocksdb -threads 100 -P workloads/workloada.spec 1>res/w1.out 2>res/w1.txt
```

Also reference run.sh and run\_redis.sh for the command line. See help by
invoking `./ycsbc` without any arguments.

Note that we do not have load and run commands as the original YCSB. Specify
how many records to load by the recordcount property. Reference properties
files in the workloads dir.

## Supported Databases
- CruiseDB
- RocksDB

Note that CruiseDB and Rocksdb will be install in the same directory(/usr),
so you should only install one of them or remove the other one.

## Note for Rocksdb
if you want to use Rocksdb, you should modify db/rocks_db.cc
```c++
//  Status s = db_->TbBegin(cf_, code);
//  assert(s.ok());
```
then recompile the project.

## Before compile YCSB-C
Note that you should compile and install Rocksdb or CruiseDB first
```shell
cd rocksdb # or cd cruisedb
make static_lib share_lib DEBUG_LEVEL=0 -j$(nproc)
sudo DEBUG_LEVEL=0 make uninstall
sudo DEBUG_LEVEL=0 make install
```