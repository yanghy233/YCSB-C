#!/bin/bash
cd 
cd rocksdb
make static_lib DEBUG_LEVEL=0 -j$(nproc)
sudo DEBUG_LEVEL=0 make uninstall && sudo make install
cd
cd YCSB-C
make
echo "running YCSB..."
./ycsba.sh
