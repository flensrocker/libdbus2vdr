
remote-example: remote-example.c
	gcc -o $@ $< $(shell pkg-config --cflags glib-2.0 gio-2.0) -I../.. $(shell pkg-config --libs glib-2.0 gio-2.0) -L.. -ldbus2vdr

clean:
	@-rm -f remote-example

