
VERSION = 0.0.2a

SOFILE = libdbus2vdr.so

CC ?= gcc
CFLAGS ?= -g -O3 -Wall

DEFINES  = -D_GNU_SOURCE
DEFINES += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE

CFLAGS   += -fPIC $(shell pkg-config --cflags glib-2.0 gio-unix-2.0)
LDADD    += $(shell pkg-config --libs glib-2.0 gio-unix-2.0)

XMLS = $(wildcard *.xml)
HEADERS = $(patsubst %.xml,%.h,$(XMLS))
CODES = $(patsubst %.xml,%.c,$(XMLS))
OBJS = $(patsubst %.xml,%.o,$(XMLS))


all: $(SOFILE)

%.o: %.c
	$(CC) $(CFLAGS) -c $(DEFINES) -o $@ $<

%.h: %.xml
	gdbus-codegen --generate-c-code `basename $< .xml` --interface-prefix de.tvdr.vdr --c-namespace DBus2vdr $<


MAKEDEP = $(CC) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile $(HEADERS)
	@-rm -f $(DEPFILE)
	for x in $(XMLS); do echo "`basename $$x .xml`.h: $$x" >> $@; done
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.c) >> $@

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
	@-rm -f $(OBJS) $(DEPFILE) $(HEADERS) $(CODES) *.so *.tgz core* *~

orig:
	if [ -d .git ]; then git archive --format=tar.gz --prefix=libdbus2vdr-$(VERSION)/ -o ../libdbus2vdr_$(VERSION).orig.tar.gz master; fi

