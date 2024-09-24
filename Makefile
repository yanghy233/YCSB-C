CC=g++
CFLAGS=-std=c++11 -g -Wall -pthread -I./ -I/usr/local/include/rocksdb -I/usr/local/include/leveldb
LDFLAGS= -lpthread -lrocksdb -lleveldb -ldl -lz -lzstd -lsnappy -lbz2 -llz4

#ORIGINFLAGS= -lpthread -lrocksdborigin -ldl -lz -lzstd -lsnappy -lbz2 -llz4
# silk: another version of rocksdb
SILKFLAGS= -lpthread -lrocksdbsilk -ldl -lz -lzstd -lsnappy -lbz2 -llz4
SUBDIRS=core db
SILKDIRS=db
SUBSRCS=$(wildcard core/*.cc) db/db_factory.cc db/rocks_db.cc db/level_db.cc
OBJECTS=$(SUBSRCS:.cc=.o)
EXEC=ycsbc

all: $(SUBDIRS) $(EXEC)

$(SUBDIRS):
	$(MAKE) -C $@

$(EXEC): $(wildcard *.cc) $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

ORIGINEXEC: $(wildcard *.cc) $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(ORIGINFLAGS) -o $(EXEC)

SILKDIR:
	for dir in $(SILKDIRS); do \
		$(MAKE) -C $$dir silk; \
	done

SILKEXEC: $(wildcard *.cc) $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(SILKFLAGS) -o $(EXEC)

origin: $(SUBDIRS) ORIGINEXEC

silk: SILKDIR $(filter-out $(SILKDIRS), $(SUBDIRS)) SILKEXEC

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done
	$(RM) $(EXEC)
	$(RM) -r rocksdb_test
	$(RM) -rf ./ramdisk_path ./rocksdb_disk_path

.PHONY: $(SUBDIRS) $(EXEC) ORIGINEXEC SILKDIR SILKEXEC origin silk clean
