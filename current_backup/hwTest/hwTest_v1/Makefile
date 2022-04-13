CC	= gcc
CFLAGS	= -g
INCLUDE	= -I.

.SUFFIXES :.o .c
.c.o:
	@echo "		Comiling...........$<"
	@$(CC) $(CFLAGS) -O2 -c $(INCLUDE) $<

OBJS =  main.o

hwTest_v1: $(OBJS)
	@echo "		Linking............$<"
	@$(CC) -o $@ $(OBJS)

clean:
	rm -rf *.o *.csv
