CC=g++
CFLAGS=-std=c++17 -c -g -Wall -fgnu-tm

INCLUDES=-I../ -I/usr/local/include/rocksdb -I/usr/local/include/leveldb/
SOURCES=rocks_db.cc db_factory.cc level_db.cc
OBJECTS=$(SOURCES:.cc=.o)

SILKINCLUDES=-I../ -I../../SILK/include/
SILKOBJECTS=$(OBJECTS)

all: $(SOURCES) $(OBJECTS)

.cc.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

$(SILKOBJECTS):
	$(CC) $(CFLAGS) $(INCLUDES) $(@:.o=.cc) -o $@

silk: $(SOURCES) $(filter-out $(SILKOBJECTS), $(OBJECTS))
	$(foreach obj, $(SILKOBJECTS), \
	$(CC) $(CFLAGS) $(SILKINCLUDES) $(obj:.o=.cc) -o $(obj) -DSILK; \
	)

clean:
	$(RM) $(OBJECTS)

.PHONY: $(SILKOBJECTS) silk clean
