CC = gcc
CFLAGS = -g -Wall
INCLUDE	= -I. -I/usr/src/rtlinux-3.2-pre2/include

.SUFFIXES :.o .c
.c.o:
	@echo "		Compiling......$<"
	@$(CC) $(CFLAGS) -c $(INCLUDE) $<

OBJS = common_utils.o userlog.o \
	com_io.o com_socket.o network.o local_utils.o message.o main.o

CoolingControl: $(OBJS)
	@echo "		Linking........$<"
	@$(CC) -lm -o $@ $(OBJS) $(LIBNAME)

clean:
	rm -rf *.o
