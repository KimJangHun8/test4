CC = gcc
CFLAGS = -g -Wall
INCLUDE	= -I. -I/usr/src/rtlinux-3.2-pre2/include

.SUFFIXES :.o .c
.c.o:
	@echo "		Compiling.......$<"
	@$(CC) $(CFLAGS) -c $(INCLUDE) $<

OBJS = .setup.o

setup: $(OBJS)
	@echo "		Linking.........$<"
	@$(CC) -lm -o $@ $(OBJS) $(LIBNAME)

.c.o:
	@echo "		Compiling.......$<"
	@$(CC) $(CFLAGS) -c $(INCLUDE) $<


OBJS = .selectType.o

setup: $(OBJS)
	@echo "		Linking.........$<"
	@$(CC) -lm -o $@ $(OBJS) $(LIBNAME)

.c.o:
	@echo "		Compiling.......$<"
	@$(CC) $(CFLAGS) -c $(INCLUDE) $<


OBJS = .makeArray.o

setup: $(OBJS)
	@echo "		Linking.........$<"
	@$(CC) -lm -o $@ $(OBJS) $(LIBNAME)
#lint:
#	lint -I${INCLUDE} *.c
clean:
	rm -rf *.o
