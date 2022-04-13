CC	= gcc
CFLAGS	= -g
INCLUDE	= -I.

.SUFFIXES :.o .c
.c.o:
	@echo "		Comiling...........$<"
	@$(CC) $(CFLAGS) -O2 -c $(INCLUDE) $<

OBJS =  main.o Menu.o KeyBoardControl.o BitMacro.o ControlBd.o MainBd.o Range.o DA.o AD.o Debug.o

test: $(OBJS)
	@echo "		Linking............$<"
	@$(CC) -g -o $@ $(OBJS) -lcurses

clean:
	rm -rf *.o *.csv
