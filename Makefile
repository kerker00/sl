#==========================================
#    Makefile: makefile for sl 7.00
#	Copyright 1993, 1998, 2014-2026
#                 Toyoda Masashi et al.
#		  (mtoyoda@acm.org)
#==========================================

CC      = gcc
CFLAGS  = -O -Wall
LDFLAGS =
LDLIBS  = -lncurses

all: sl

sl: sl.c sl.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o sl sl.c $(LDLIBS)

clean:
	rm -f sl

distclean: clean
