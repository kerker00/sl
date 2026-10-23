#==========================================
#    Makefile: makefile for sl 7.01
#	Copyright 1993, 1998, 2014-2026
#                 Toyoda Masashi et al.
#		  (mtoyoda@acm.org)
#==========================================

CC      = gcc
CFLAGS  = -O -Wall -Wextra
CPPFLAGS =
LDFLAGS =
LDLIBS  = -lncurses

PREFIX  = /usr/local
BINDIR  = $(PREFIX)/bin
MANDIR  = $(PREFIX)/share/man

all: sl

sl: sl.c sl.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o sl sl.c $(LDLIBS)

install: sl
	install -d $(DESTDIR)$(BINDIR) $(DESTDIR)$(MANDIR)/man1 $(DESTDIR)$(MANDIR)/ja/man1
	install -m 755 sl $(DESTDIR)$(BINDIR)/sl
	install -m 644 sl.1 $(DESTDIR)$(MANDIR)/man1/sl.1
	install -m 644 sl.1.ja $(DESTDIR)$(MANDIR)/ja/man1/sl.1

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/sl $(DESTDIR)$(MANDIR)/man1/sl.1 $(DESTDIR)$(MANDIR)/ja/man1/sl.1

clean:
	rm -f sl

distclean: clean

.PHONY: all install uninstall clean distclean
