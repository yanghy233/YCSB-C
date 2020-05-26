CC=g++
CFLAGS=-std=c++11 -c -g -Wall -fgnu-tm

INCLUDES=-I../ -I../../rocksdb/include/
SOURCES=$(wildcard *.cc)
OBJECTS=$(SOURCES:.cc=.o)

SILKINCLUDES=-I../ -I../../SILK/include/
SILKSOURCES=rocks_db.cc
SILKOBJECT=rocks_db.o

all: $(SOURCES) $(OBJECTS)

.cc.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

$(SILKOBJECT):
	$(CC) $(CFLAGS) $(INCLUDES) $(SILKSOURCES) -o $@

silk: $(SOURCES) $(filter-out $(OBJECTS), $(SILKOBJECT))
	$(CC) $(CFLAGS) $(SILKINCLUDES) $(SILKSOURCES) -o $(SILKOBJECT)

clean:
	$(RM) $(OBJECTS)

.PHONY: $(SILKOBJECT) silk clean
