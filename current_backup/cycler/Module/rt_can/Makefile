# Makefile for rt_can

SYSTEM			= RTLINUX_V3

INCLUDEDIR		= /usr/src/rtlinux-3.2-pre2/include

INSTALL 		= install -c -m 644

OBJS			= rt_can.o

VERSION			= kjg1

include rtl.mk
#include /project/system_mcts/current/Module/mControl/rtl.mk
#include /project/system_mcts/current/App/AppControl/kjg_cflags.mk

CFLAGS			+= -D$(SYSTEM) -DVERSION=\"$(VERSION)\"

.c.o:	Makefile
		@echo "		Compiling......$<"
		@$(CC) $(CFLAGS) -c $<
#		@$(CC) $(CFLAGS) $(KJG_CFLAGS) -c $<

all:			$(OBJS)

clean:
			rm -fr $(OBJS) rt_can-$(VERSION)
			rm -f *.o

install:	rt_can.o
			$(INSTALL) $(OBJS) /lib/modules/`uname -r`/misc/
			$(INSTALL) rt_can.h $(INCLUDEDIR)/rt_can.h
