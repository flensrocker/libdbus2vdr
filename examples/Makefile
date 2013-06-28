
CC ?= gcc
CFLAGS ?= -g -O3 -Wall

DEFINES  = -D_GNU_SOURCE
DEFINES += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE

CFLAGS  += $(shell pkg-config --cflags glib-2.0 gio-unix-2.0)
LDADD   += -ldbus2vdr $(shell pkg-config --libs glib-2.0 gio-unix-2.0)


%.o: %.c
	$(CC) $(CFLAGS) -c $(DEFINES) -o $@ $<


remote-example: remote-example.o
	gcc -o $@ $< $(LDADD)

clean:
	@-rm -f remote-example *.o

all: remote-example

