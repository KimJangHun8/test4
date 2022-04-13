CC = gcc
CFLAGS = -g -Wall
INCLUDE = -I.

.SUFFIXES :.o .c
.c.o:
	@echo "		Compiling......$<"
	@$(CC) $(CFLAGS) -c $(INCLUDE) $<

OBJS = common_utils.o userlog.o \
	com_io.o comm.o serial.o local_utils.o message.o main.o

Meter2: $(OBJS)
	@echo "		Linking........$<"
	@$(CC) -lm -o $@ $(OBJS) $(LIBNAME)

#lint:
#	lint -I${INCLUDE} *.c
