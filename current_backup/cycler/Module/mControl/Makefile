include rtl.mk

.SUFFIXES :.o .c
.c.o:
	@echo "		Compiling......$<"
	@$(CC) $(CFLAGS) -c $(INCLUDE) $<

OBJ	= rtTask.o common_utils.o local_utils.o message.o \
	ModuleControl.o BoardControl.o Analog.o ChannelControl.o \
	DInOutControl.o CAN.o DAQ.o FAD.o \
	PCU_Control.o PCU.o ChannelControl_CAN.o FaultCond.o\
	

mControl: $(OBJ) $(HEADER)
	@echo "		Linking........$<"
	@$(LD) $(LDFLAGS) -o $@ $(OBJ)

unload:
	@echo "		remove real time task......"
	(./rmrtl)

load:
	@make unload
	@echo "		Before load  real time task......"
	(./insrtl)
clean:
	rm -rf *.o	mControl
