CC=g++
CFLAGS=-std=c++11 -c -g -Wall -fgnu-tm

INCLUDES=-I../ -I../../rocksdb/include/
SOURCES=rocks_db.cc db_factory.cc
OBJECTS=$(SOURCES:.cc=.o)

SILKINCLUDES=-I../ -I../../SILK/include/
SILKOBJECTS=rocks_db.o db_factory.o

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
