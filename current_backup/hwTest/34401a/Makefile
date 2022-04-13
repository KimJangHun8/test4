
CC = gcc
CFLAGS = -g -Wall
BINDIR = /usr/local/bin
INCLUDE = -I.

.SUFFIXES :.o .c
.c.o:
	@echo "		Compiling......$<"
	@$(CC) $(CFLAGS) -c $(INCLUDE) $<

OBJS = main.o comm.o  

Cali:  $(OBJS)
	@echo "		Linking......$<"
	@$(CC) -o $@ $(OBJS)
