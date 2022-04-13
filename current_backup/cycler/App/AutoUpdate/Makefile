CC = gcc
CFLAGS = -g -Wall
INCLUDE	= -I. -I/usr/src/rtlinux-3.2-pre2/include

.SUFFIXES :.o .c
.c.o:
	@echo "		Compiling.......$<"
	@$(CC) $(CFLAGS) -c $(INCLUDE) $<

OBJS = common_utils.o local_utils.o main.o userlog.o

AutoUpdate: $(OBJS)
	@echo "		Linking.........$<"
	@$(CC) -lm -o $@ $(OBJS) $(LIBNAME)

#lint:
#	lint -I${INCLUDE} *.c
clean:
	rm -rf *.o
