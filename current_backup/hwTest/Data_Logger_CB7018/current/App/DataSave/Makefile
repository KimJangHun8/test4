CC = gcc
CFLAGS = -g -Wall
INCLUDE = -I. -I/usr/src/rtlinux-3.2-pre2/include

.SUFFIXES :.o .c
.c.o:
	@echo "		Compiling......$<"
	@$(CC) $(CFLAGS) -c $(INCLUDE) $<
	
OBJS = common_utils.o userlog.o \
	local_utils.o message.o main.o

DataSave: $(OBJS)
	@echo "		Linking........$<"
	@$(CC) -lm -o $@ $(OBJS) $(LIBNAME)

#lint:
#	lint -I${INCLUDE} *.c

clear:
	(rm -rf ./monitoringData/*)
