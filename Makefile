CC=g++
CFLAGS=-std=c++11 -g -Wall -pthread -I./ -L../rocksdb -L./
LDFLAGS= -lpthread -ltbb -lhiredis -lrocksdb -ldl
ORIGINFLAGS= -lpthread -ltbb -lhiredis -lrocksdborigin -ldl
SILKFLAGS= -lpthread -ltbb -lhiredis -lrocksdbsilk -ldl
SUBDIRS=core db redis
SILKDIRS=db
SUBSRCS=$(wildcard core/*.cc) $(wildcard db/*.cc)
OBJECTS=$(SUBSRCS:.cc=.o)
EXEC=ycsbc

all: $(SUBDIRS) $(EXEC)

$(SUBDIRS):
	$(MAKE) -C $@

$(EXEC): $(wildcard *.cc) $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

ORIGINEXEC: $(wildcard *.cc) $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(ORIGINFLAGS) -o $(EXEC)

SILKDIR: $(filter-out $(SUBDIRS), $(SILKDIRS))
	for dir in $(SILKDIRS); do \
		$(MAKE) -C $$dir silk; \
	done

SILKEXEC: $(wildcard *.cc) $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(SILKFLAGS) -o $(EXEC)

origin: $(SUBDIRS) ORIGINEXEC

silk: SILKDIR SILKEXEC

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done
	$(RM) $(EXEC)
	$(RM) -rf rocksdb_test

.PHONY: $(SUBDIRS) $(EXEC) ORIGINEXEC SILKDIR SILKEXEC origin silk clean
