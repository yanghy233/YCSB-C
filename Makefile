CC=g++
CFLAGS=-std=c++11 -g -Wall -pthread -I./ -L../rocksdb -L./
LDFLAGS= -lpthread -ltbb -lhiredis -lrocksdb -ldl
ORIGINFLAGS= -lpthread -ltbb -lhiredis -lrocksdborigin -ldl
SUBDIRS=core db redis
SUBSRCS=$(wildcard core/*.cc) $(wildcard db/*.cc)
OBJECTS=$(SUBSRCS:.cc=.o)
EXEC=ycsbc

all: $(SUBDIRS) $(EXEC)

$(SUBDIRS):
	$(MAKE) -C $@

$(EXEC): $(wildcard *.cc) $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

origin: $(wildcard *.cc) $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(ORIGINFLAGS) -o $(EXEC)

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done
	$(RM) $(EXEC)
	-rm -rf rocksdb_test

.PHONY: $(SUBDIRS) $(EXEC) clean origin
