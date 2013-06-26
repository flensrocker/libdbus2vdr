
VERSION = 0.0.1a

SOFILE = libdbus2vdr.so

CC ?= gcc
CFLAGS ?= -g -O3 -Wall

DEFINES  = -D_GNU_SOURCE
DEFINES += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE

CFLAGS   += -fPIC $(shell pkg-config --cflags glib-2.0 gio-2.0)
LDADD    += $(shell pkg-config --libs glib-2.0 gio-2.0)

OBJS = $(patsubst %.c,%.o,$(wildcard *.c))

all: $(SOFILE)

%.o: %.c
	$(CC) $(CFLAGS) -c $(DEFINES) -o $@ $<

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.c) > $@

-include $(DEPFILE)

$(SOFILE): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared $(OBJS) $(LDADD) -o $@

install-includes:
	@mkdir -p $(DESTDIR)/usr/include/libdbus2vdr
	@cp *.h $(DESTDIR)/usr/include/libdbus2vdr

install-lib: $(SOFILE)
	install -D $^ $(DESTDIR)/usr/lib/$^

install: install-includes install-lib

clean:
	@-rm -f $(OBJS) $(DEPFILE) *.so *.tgz core* *~

orig:
	if [ -d .git ]; then git archive --format=tar.gz --prefix=libdbus2vdr-$(VERSION)/ -o ../libdbus2vdr_$(VERSION).orig.tar.gz master; fi

