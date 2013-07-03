
CC ?= gcc
CFLAGS ?= -g -O3 -Wall

DEFINES  = -D_GNU_SOURCE
DEFINES += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE

CFLAGS  += -I. $(shell pkg-config --cflags glib-2.0 gio-unix-2.0)
LDADD   += -Llibdbus2vdr/ -ldbus2vdr $(shell pkg-config --libs glib-2.0 gio-unix-2.0)

EXAMPLES = pipswap-example remote-example volume-example


all: $(EXAMPLES)

%.o: %.c
	$(CC) $(CFLAGS) -c $(DEFINES) -o $@ $<

libdbus2vdr:
	if [ ! -e libdbus2vdr ]; then ln -s .. libdbus2vdr; fi
	make -C libdbus2vdr/ all

pipswap-example: libdbus2vdr pipswap-example.o
	gcc -o $@ $@.o $(LDADD)

remote-example: libdbus2vdr remote-example.o
	gcc -o $@ $@.o $(LDADD)

volume-example: libdbus2vdr volume-example.o
	gcc -o $@ $@.o $(LDADD)

clean:
	@if [ -e libdbus2vdr ]; then make -C libdbus2vdr/ clean; fi
	@-rm -f libdbus2vdr $(EXAMPLES) *.o

