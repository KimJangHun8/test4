CC	= gcc
CFLAGS	= -g -Wall
INCLUDE	= -I. -I/usr/src/rtlinux-3.2-pre2/include

.SUFFIXES :.o .c
.c.o:
	@echo "		Compiling......$<"
	@$(CC) $(CFLAGS) -c $(INCLUDE) $<

OBJS = common_utils.o userlog.o \
	local_utils.o message.o StandardInput.o main.o

Data_Logger: $(OBJS)
	@echo "		Linking........$<"
	@$(CC) -lm -o $@ $(OBJS) $(LIBNAME)

#lint:
#	lint -I${INCLUDE} *.c

all:
	make clear_memory
	make clear_log

	(cp -rf common_utils.* ../DataSave)
	(cp -rf common_utils.* ../Meter1)
	(cp -rf common_utils.* ../Meter2)

	(cp -rf userlog.* ../DataSave)
	(cp -rf userlog.* ../Meter1)
	(cp -rf userlog.* ../Meter2)

	(touch *; rm -rf *.o; rm -rf Data_Logger; make)
	(cd ../DataSave; touch *; rm -rf *.o; rm -rf DataSave; make)
	(cd ../Meter1; touch *; rm -rf *.o; rm -rf Meter1; make)
	(cd ../Meter2; touch *; rm -rf *.o; rm -rf Meter2; make)
	(cd /; update -s1)
	(sleep 1)

clean:
	(touch *; rm -rf *.o; rm -rf Data_Logger)
	(cd ../DataSave; touch *; rm -rf *.o; rm -rf DataSave)
	(cd ../Meter1; touch *; rm -rf *.o; rm -rf Meter1)
	(cd ../Meter2; touch *; rm -rf *.o; rm -rf Meter2)
	(cd /; update -s1)
	(sleep 1)

clear_memory:
	(cd ../../../config/sharedMemory; rm -rf *; touch sharedMemory)
	(cd ../../../config/sharedMemory_dump; rm -rf *; touch sharedMemory)

clear_log:
	(cd ../../../config/log; rm -rf *)

clear_program:
	rm -rf *.o; rm -rf *.c; rm -rf *.h
	(cd ../DataSave; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../Meter1; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../Meter2; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../../INC; rm -rf *)
