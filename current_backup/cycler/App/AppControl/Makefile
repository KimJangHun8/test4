CC	= gcc
CFLAGS	= -g -Wall
INCLUDE	= -I. -I/usr/src/rtlinux-3.2-pre2/include

.SUFFIXES :.o .c
.c.o:
	@echo "		Compiling......$<"
	@$(CC) $(CFLAGS) -c $(INCLUDE) $<

OBJS = common_utils.o userlog.o \
	local_utils.o message.o StandardInput.o main.o

#Multi Channel Test System
mcts: $(OBJS)
	@echo "		Linking........$<"
	@$(CC) -lm -o $@ $(OBJS) $(LIBNAME)

#lint:
#	lint -I${INCLUDE} *.c

all:
	make clear_memory

	(cp -rf common_utils.* ../MainClient)
	(cp -rf common_utils.* ../ExtClient)
	(cp -rf common_utils.* ../DataSave)
	(cp -rf common_utils.* ../CaliMeter)
	(cp -rf common_utils.* ../CaliMeter2)
	(cp -rf common_utils.* ../AnalogMeter)
	(cp -rf common_utils.* ../AnalogMeter2)
	(cp -rf common_utils.* ../FndMeter)
	(cp -rf common_utils.* ../FADM)
	(cp -rf common_utils.* ../AutoUpdate)
	(cp -rf common_utils.* ../DumpMonitor)
	(cp -rf common_utils.* ../PSKill)
	(cp -rf common_utils.* ../DebugMonitor)
	(cp -rf common_utils.* ../DAQ_Client)
	(cp -rf common_utils.* ../CoolingControl)
	(cp -rf common_utils.* ../../Module/mControl)

	(cp -rf userlog.* ../MainClient)
	(cp -rf userlog.* ../ExtClient)
	(cp -rf userlog.* ../DataSave)
	(cp -rf userlog.* ../CaliMeter)
	(cp -rf userlog.* ../CaliMeter2)
	(cp -rf userlog.* ../AnalogMeter)
	(cp -rf userlog.* ../AnalogMeter2)
	(cp -rf userlog.* ../FndMeter)
	(cp -rf userlog.* ../FADM)
	(cp -rf userlog.* ../AutoUpdate)
	(cp -rf userlog.* ../PSKill)
	(cp -rf userlog.* ../DAQ_Client)
	(cp -rf userlog.* ../CoolingControl)
	
	(cd ../../Module/rt_can; make clean; make; make install)

	(touch *; rm -rf *.o; rm -rf mcts; make)
	(cd ../MainClient; touch *; rm -rf *.o; rm -rf MainClient; rm -rf .* ; make)
	(cd ../ExtClient; touch *; rm -rf *.o; rm -rf ExtClient; rm -rf .* ; make)
	(cd ../DataSave; touch *; rm -rf *.o; rm -rf DataSave; rm -rf .* ; make)
	(cd ../CaliMeter; touch *; rm -rf *.o; rm -rf CaliMeter; rm -rf .* ; make)
	(cd ../CaliMeter2; touch *; rm -rf *.o; rm -rf CaliMeter2; rm -rf .* ; make)
	(cd ../AnalogMeter; touch *; rm -rf *.o; rm -rf AnalogMeter; rm -rf .* ; make)
	(cd ../AnalogMeter2; touch *; rm -rf *.o; rm -rf AnalogMeter2; rm -rf .* ; make)
	(cd ../FndMeter; touch *; rm -rf *.o; rm -rf FndMeter; rm -rf .* ; make)
	(cd ../FADM; touch *; rm -rf *.o; rm -rf FADM; rm -rf .* ; make)
	(cd ../AutoUpdate; touch *; rm -rf *.o; rm -rf AutoUpdate; rm -rf .* ; make)
	(cd ../DumpMonitor; touch *; rm -rf *.o; rm -rf DumpMonitor; rm -rf .* ; make)
	(cd ../DebugMonitor; touch *; rm -rf *.o; rm -rf DebugMonitor; rm -rf .*; make)
	(cd ../PSKill; touch *; rm -rf *.o; rm -rf PSKill; rm -rf .*; make)
	(cd ../Setup; touch *; rm -rf *.o; rm -rf Setup; rm -rf .*; make)
	(cd ../DAQ_Client; touch *; rm -rf *.o; rm -rf DAQ_Client; rm -rf .* ; make)
		(cd ../CoolingControl; touch *; rm -rf *.o; rm -rf CoolingControl; rm -rf .*; make)
	(cd ../../Module/mControl; touch *; rm -rf *.o; rm -rf mControl; rm -rf .* ; make)

	(cd /; update -s1)
	(sleep 1)

clean:
	make clear_memory
	make clear_log

	rm -rf *.o	mcts
	(cd ../MainClient; rm -rf *.o	MainClient)
	(cd ../ExtClient; rm -rf *.o	ExtClient)
	(cd ../DataSave; rm -rf *.o		DataSave)
	(cd ../CaliMeter; rm -rf *.o	CaliMeter)
	(cd ../CaliMeter2; rm -rf *.o	CaliMeter2)
	(cd ../AnalogMeter; rm -rf *.o	AnalogMeter)
	(cd ../AnalogMeter2; rm -rf *.o	AnalogMeter2)
	(cd ../FADM; rm -rf *.o	FADM)
	(cd ../AutoUpdate; rm -rf *.o	AutoUpdate)
	(cd ../DumpMonitor; rm -rf *.o	DumpMonitor)
	(cd ../DebugMonitor; rm -rf *.o	DebugMonitor)
	(cd ../DAQ_Client; rm -rf *.o	DAQ_Client)
	(cd ../CoolingControl; rm -rf *.o	CoolingControl)
	(cd ../../Module/mControl; rm -rf *.o	mControl)
	(cd ../../Module/rt_can; rm -rf *.o	rt_can)

clear_memory:
	(cd /root/cycler_data/config/sharedMemory; rm -rf *; touch systemMemory)
	(cd /root/cycler_data/config/sharedMemory_dump; rm -rf *; touch systemMemory)

clear_log:
	(cd /root/cycler_data/log; rm -rf *)

clear_data:
	(cd /root/cycler_data/resultData; rm -rf *)
	(cd /root/cycler_data/resultData; \
	mkdir -p ch01 ch02 ch03 ch04 ch05 ch06 ch07 ch08 ch09 ch10; \
	mkdir -p ch11 ch12 ch13 ch14 ch15 ch16 ch17 ch18 ch19 ch20; \
	mkdir -p ch21 ch22 ch23 ch24 ch25 ch26 ch27 ch28 ch29 ch30; \
	mkdir -p ch31 ch32 ch33 ch34 ch35 ch36 ch37 ch38 ch39 ch40; \
	mkdir -p ch41 ch42 ch43 ch44 ch45 ch46 ch47 ch48 ch49 ch50; \
	mkdir -p ch51 ch52 ch53 ch54 ch55 ch56 ch57 ch58 ch59 ch60; \
	mkdir -p ch61 ch62 ch63 ch64;)
dir:
	(mkdir -p /mnt/d/resultData)
	(mkdir -p /root/cycler_data/log)
	(cd /root/cycler_data; rm -rf resultData)
	(cd /root/cycler_data; ln -s /mnt/d/resultData resultData)
	(cd /root/cycler_data/resultData; \
	rm -rf * ;\
	mkdir -p ch01 ch02 ch03 ch04 ch05 ch06 ch07 ch08 ch09 ch10; \
	mkdir -p ch11 ch12 ch13 ch14 ch15 ch16 ch17 ch18 ch19 ch20; \
	mkdir -p ch21 ch22 ch23 ch24 ch25 ch26 ch27 ch28 ch29 ch30; \
	mkdir -p ch31 ch32 ch33 ch34 ch35 ch36 ch37 ch38 ch39 ch40; \
	mkdir -p ch41 ch42 ch43 ch44 ch45 ch46 ch47 ch48 ch49 ch50; \
	mkdir -p ch51 ch52 ch53 ch54 ch55 ch56 ch57 ch58 ch59 ch60; \
	mkdir -p ch61 ch62 ch63 ch64;)
	
	(cd /root/cycler_data;mkdir -p  pattern)
	(cd /root/cycler_data/pattern;\
	mkdir -p ch01 ch02 ch03 ch04 ch05 ch06 ch07 ch08 ch09 ch10; \
	mkdir -p ch11 ch12 ch13 ch14 ch15 ch16 ch17 ch18 ch19 ch20; \
	mkdir -p ch21 ch22 ch23 ch24 ch25 ch26 ch27 ch28 ch29 ch30; \
	mkdir -p ch31 ch32 ch33 ch34 ch35 ch36 ch37 ch38 ch39 ch40; \
	mkdir -p ch41 ch42 ch43 ch44 ch45 ch46 ch47 ch48 ch49 ch50; \
	mkdir -p ch51 ch52 ch53 ch54 ch55 ch56 ch57 ch58 ch59 ch60; \
	mkdir -p ch61 ch62 ch63 ch64;)

	(cd /root/cycler_data;mkdir -p  map)
	(cd /root/cycler_data/map;\
	mkdir -p ch01 ch02 ch03 ch04 ch05 ch06 ch07 ch08 ch09 ch10; \
	mkdir -p ch11 ch12 ch13 ch14 ch15 ch16 ch17 ch18 ch19 ch20; \
	mkdir -p ch21 ch22 ch23 ch24 ch25 ch26 ch27 ch28 ch29 ch30; \
	mkdir -p ch31 ch32 ch33 ch34 ch35 ch36 ch37 ch38 ch39 ch40; \
	mkdir -p ch41 ch42 ch43 ch44 ch45 ch46 ch47 ch48 ch49 ch50; \
	mkdir -p ch51 ch52 ch53 ch54 ch55 ch56 ch57 ch58 ch59 ch60; \
	mkdir -p ch61 ch62 ch63 ch64;)

	(cd /mnt/d;mkdir -p tmpData)
	(cd /root/cycler_data; rm -rf tmpData)
	(cd /root/cycler_data; ln -s /mnt/d/tmpData tmpData)
	(cd /root/cycler_data/tmpData;\
	rm -rf * ;\
	mkdir -p ch01 ch02 ch03 ch04 ch05 ch06 ch07 ch08 ch09 ch10; \
	mkdir -p ch11 ch12 ch13 ch14 ch15 ch16 ch17 ch18 ch19 ch20; \
	mkdir -p ch21 ch22 ch23 ch24 ch25 ch26 ch27 ch28 ch29 ch30; \
	mkdir -p ch31 ch32 ch33 ch34 ch35 ch36 ch37 ch38 ch39 ch40; \
	mkdir -p ch41 ch42 ch43 ch44 ch45 ch46 ch47 ch48 ch49 ch50; \
	mkdir -p ch51 ch52 ch53 ch54 ch55 ch56 ch57 ch58 ch59 ch60; \
	mkdir -p ch61 ch62 ch63 ch64;)

	(cd /mnt/d;mkdir -p userData)
	(cd /root/cycler_data; rm -rf userData)
	(cd /root/cycler_data; ln -s /mnt/d/userData userData)
	(cd /root/cycler_data/userData;\
	rm -rf * ;\
	mkdir -p 01 02 03 04 05 06 07 08 09 10; \
	mkdir -p 11 12 13 14 15 16 17 18 19 20; \
	mkdir -p 21 22 23 24 25 26 27 28 29 30; \
	mkdir -p 31 32 33 34 35 36 37 38 39 40; \
	mkdir -p 41 42 43 44 45 46 47 48 49 50; \
	mkdir -p 51 52 53 54 55 56 57 58 59 60; \
	mkdir -p 61 62 63 64;)

	(cd /mnt/d;mkdir -p trackingData)
	(cd /root/cycler_data; rm -rf trackingData)
	(cd /root/cycler_data; ln -s /mnt/d/trackingData trackingData)
	(cd /root/cycler_data/trackingData;\
	rm -rf * ;\
	mkdir -p ch01 ch02 ch03 ch04 ch05 ch06 ch07 ch08 ch09 ch10; \
	mkdir -p ch11 ch12 ch13 ch14 ch15 ch16 ch17 ch18 ch19 ch20; \
	mkdir -p ch21 ch22 ch23 ch24 ch25 ch26 ch27 ch28 ch29 ch30; \
	mkdir -p ch31 ch32 ch33 ch34 ch35 ch36 ch37 ch38 ch39 ch40; \
	mkdir -p ch41 ch42 ch43 ch44 ch45 ch46 ch47 ch48 ch49 ch50; \
	mkdir -p ch51 ch52 ch53 ch54 ch55 ch56 ch57 ch58 ch59 ch60; \
	mkdir -p ch61 ch62 ch63 ch64;)


	(cd /root/cycler_data/config/parameter; rm -rf Setup;\
	ln -s /project/Cycler/current/cycler/App/Setup/Setup Setup)

#	/project/Cycler/current/cycler/App/Scripts/SysConfig_copy.sh

clear_program:
	rm -rf *.o; rm -rf *.c; rm -rf *.h
	(cd ../MainClient; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../ExtClient; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../DataSave; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../CaliMeter; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../CaliMeter2; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../AnalogMeter; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../AnalogMeter2; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../FndMeter; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../FADM; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../AutoUpdate; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../PSKill; rm -rf *.o; rm -rf *.c; rm -rf *.h)
#	(cd ../DumpMonitor; rm -rf *.o; rm -rf *.c; rm -rf *.h)
#	(cd ../DebugMonitor; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../DAQ_Client; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../CoolingControl; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../../Module/mControl; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../../Module/rt_can; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../../INC; rm -rf *)
