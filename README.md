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
mkdir ramdisk_path rocksdb_disk_path ycsb-leveldb
./ycsba.sh
```

Use the following command to get the max physical thread count of your systems.
```shell
echo $(nproc)
```

Run Workload A with Rocksdb, an implementation of the database,
for example:
```
./ycsbc -db cruisedb -threads 32 -P workloads/workloada.spec 1>res/w1.out 2>res/w1.txt
./ycsbc -db rocksdb -threads 32 -P workloads/workloada.spec 1>res/w1.out 2>res/w1.txt
```

See help by invoking `./ycsbc` without any arguments.

Note that we do not have load and run commands as the original YCSB. Specify
how many records to load by the recordcount property. Reference properties
files in the workloads dir.

## Supported Databases
- LevelDB
- RocksDB
- CruiseDB

Note that CruiseDB and Rocksdb will be install in the same directory(/usr),
so you should only install one of them or remove the other one.

## Note for Rocksdb
If you want to use Rocksdb, you should modify db/rocks_db.cc
```c++
//  Status s = db_->TbBegin(cf_, code);
//  assert(s.ok());
```
Then recompile the project.

## Before compile YCSB-C
Note that if you want to test Rocksdb or CruiseDB,
you should compile and install first.
```shell
cd rocksdb # or cd cruisedb
make static_lib share_lib DEBUG_LEVEL=0 -j$(nproc)
sudo DEBUG_LEVEL=0 make uninstall
sudo DEBUG_LEVEL=0 make install
```
