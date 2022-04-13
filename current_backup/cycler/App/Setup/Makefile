CC	= gcc
CFLAGS	= -g -Wall
INCLUDE	= -I.

.SUFFIXES :.o.c
.c.o:
	@echo "		Compiling......$<"
	@$(CC) $(CFLAGS) -c $(INCLUDE) $<

OBJS = main.o

Setup: $(OBJS)
	@echo "		Linking........$<"
	@$(CC) -lm -o $@ $(OBJS) $(LIBNAME)
