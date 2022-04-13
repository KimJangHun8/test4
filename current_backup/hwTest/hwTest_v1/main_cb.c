#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <asm/io.h>

#include <stdlib.h>
#include <termios.h>
//#include <curses.h>
#include <unistd.h>


#define NONE	0
#define OUTPUT	1
#define INPUT	2
#define GET		3
#define AD		4
#define ADJ		5
#define DACHECK	6
#define AD_TEMP	7
#define MAX_AD_COUNT	10

static struct termios initial_settings, new_settings;
static int peek_character = -1;


void Init_Keyboard();
void Close_Keyboard();
int Keyboard_Hit();
int Read_Character();





short REFIC_P5V[MAX_AD_COUNT];
short REFIC_G0V[MAX_AD_COUNT];
short REFIC_N5V[MAX_AD_COUNT];
short REFIC_P5I[MAX_AD_COUNT];
short REFIC_G0I[MAX_AD_COUNT];
short REFIC_N5I[MAX_AD_COUNT];
short REFIC[MAX_AD_COUNT];

union tmpData {
	char	data[2];
	short int	val;
};

union ADData {
	unsigned char	data[2];
	short int	val;
};

int swapBit(int ch) {
  	int i, result, array[4];

	result &= 0x00;
	if(ch & 0x01)
		array[0] = 0x08;
	else
		array[0] = 0x00;
	if(ch & 0x02)
		array[1] = 0x04;
	else
		array[1] = 0x00;
	if(ch & 0x04)
		array[2] = 0x02;
	else
		array[2] = 0x00;
	if(ch & 0x08)
		array[3] = 0x01;
	else
		array[3] = 0x00;
	
	for(i=0; i<4; i++)
		result |= array[i];
	return result;
}

int main(void)
{
    int		retval, addr, val, type, mode, phase;
	int		o_addr, i_addr, count, ch;
    int 	low,i,j;
	short int	tmpVal, tmpVal1;
    float	refVal,adV,adI;
    char	tmp[10];
	unsigned char	o_val, i_val, muxVal; 
	unsigned char muxVal1, muxVal2, muxVal3, muxVal4;
    unsigned char	adVal, adVal2;
	union	tmpData txData;
	union	ADData adData;
    struct	timeval tv;
    fd_set	rfds;
	FILE *fp;

	mode = NONE;
    if(iopl(3)) exit(1);

	while(1) {
		printf(">> ");
		scanf("%s", tmp);

		if(strcmp(tmp, "vref") == 0) {
			scanf("%x", &val);
			if(val == 0) {
				txData.val = 0;
			} else {
				refVal = (float)val;
				tmpVal = (short int)((refVal/10.0)*32767.0);
				printf("vref : %1.3f %02X\n", refVal, tmpVal);
				txData.val = tmpVal;
			}
			outb(0x01, 0x640);
			outb(txData.data[1], 0x641);	//REF_HIGH
			outb(txData.data[0], 0x642);
		}else if(strcmp( tmp, "addr") == 0){
				int ch = 'i';
				int data;
				int inputValue;
				int outputValue;
				int tempOutputValue;
				int runLedFlag = 0;
				int index;
				int outputFlag = 0;
				

				Init_Keyboard();
				while( ch != 'q'){
					//	printf("looping\n");
						if(ch == 'i'){
								if(outputFlag == 1){
										outb(0x0, 0x601);
										outb(0x0, 0x602);
										outb(0x0, 0x603);
										outb(0x0, 0x604);
										outputFlag = 0;
								}
								usleep(100000);
								system("clear");
								printf("PNE 200807-Control B/D REV01 test\n"); 
								printf("\n");
								printf("\n");
								inputValue = inb(0x602);
								printf("0x602 Address Data : %x\n", inputValue);
								printf("\n");
								printf("\n");
								inputValue = inb(0x610);
								printf("0x610 Address Data : %x\n", inputValue);
								printf("\n");
								printf("\n");
								inputValue = inb(0x612);
								printf("0x612 Address Data : %x\n", inputValue);
								printf("\n");
								printf("\n");
								inputValue = inb(0x611);
								printf("0x611 Address Data : %x\n", inputValue);
								printf("\n");
								printf("\n");
								inputValue = inb(0x613);
								printf("0x613 Address Data : %x\n", inputValue);
								printf("\n");
								printf("\n");
								printf("\n");
								printf("'o' - ot, smps 7, 3V fail, cal port test
										'q' - you can exit\n");
								printf("\n");
						}
						else if( ch == 'o'){
								sleep(1);
								//if the run led lamp off, 
								//the run led lamp will be on
								if(runLedFlag == 0){
										outb( 0x4, 0x604);
										runLedFlag = 1;
								}
								//ext & cal port on
								sleep(1);
								outb(0xff, 0x601);
								outb(0xff, 0x602);
								outb(0x3, 0x603);
								//ext & cal port off
								sleep(1);
								outb(0x0, 0x601);
								outb(0x0, 0x602);
								outb(0x0, 0x603);
								//ext bit check
								tempOutputValue = 0x1;
								for(index = 0; index < 8; index++){
										outputValue = tempOutputValue << index;
										usleep(90000);
										outb(outputValue, 0x601);
										outb(outputValue, 0x602);
								}
								
								outb(outputValue, 0x603);

								tempOutputValue = tempOutputValue << 7;
								for(index = 0; index < 8; index++){
										outputValue = tempOutputValue >> index;
										usleep(90000);
										outb(outputValue, 0x601);
										outb(outputValue, 0x602);
								}
								outb(0x2, 0x603);
								sleep(1);
								tempOutputValue = outputValue;
								for(index = 0; index < 8; index ++){
										outputValue |= tempOutputValue << index;
										usleep(90000);
										outb(outputValue, 0x601);
										outb(outputValue, 0x602);
								}
								outb(0x3, 0x603);
								sleep(1);
								outb(0x0, 601);
								outb(0x0, 602);
								outb(0x0, 603);
							
						}							


						if(Keyboard_Hit()){
								ch = Read_Character();
								
								printf("you hit %c\n", ch);
								if( ch == 'q'){
									outb(0x0, 0x601);
									outb(0x0, 0x602);
									outb(0x0, 0x603);
									outb(0x0, 0x604);
								}
	
										
						}
				}
				Close_Keyboard();
				//exit(0);
		}else if(strcmp(tmp, "iref") == 0) {
			scanf("%x", &val);
			if(val == 0) {
				txData.val = 0;
			} else {
				refVal = (float)val;
				tmpVal = (short int)(refVal/10.0*32768.0);
				printf("iref : %1.3f %02X\n", refVal, tmpVal);
				txData.val = tmpVal;
			}
			outb(0x01, 0x640);
			outb(txData.data[1], 0x641);	//REF_HIGH
			outb(txData.data[0], 0x646);
		} else if(strcmp(tmp, "lref") == 0) {
			scanf("%x", &val);
			if(val == 0) {
				txData.val = 0;
			} else {
				refVal = (float)val;
				tmpVal = (short int)(refVal/10.0*32768.0);
				printf("lref : %1.3f %02X\n", refVal, tmpVal);
				txData.val = tmpVal;
			}
			outb(0x01, 0x640);
			outb(txData.data[1], 0x641);	//REF_HIGH
			outb(txData.data[0], 0x64a);
		} else if(strcmp(tmp, "wr") == 0) {
			scanf("%x %x", &addr, &val);
			printf("wr %x %x\n", addr, val);
			mode = NONE;
			o_addr = addr;
			o_val = (unsigned char)val;
			outb(o_val, o_addr);
		} else if(strcmp(tmp, "read") == 0) {
			scanf("%x", &addr);
			printf("read %x\n", addr);
			i_addr = addr;
			mode = INPUT;
		} else if(strcmp(tmp, "get") == 0) {
			scanf("%x", &addr);
			printf("get %x\n", addr);
			mode = NONE;
			i_addr = addr;
			i_val = inb(i_addr);
			printf("get data %x %x\n", i_addr, i_val);
		} else if(strcmp(tmp, "getauto") == 0) {
			scanf("%x", &addr);
			printf("get %x\n", addr);
			while(1){
				mode = NONE;
				i_addr = addr;
				i_val = inb(i_addr);
				printf("get data %x %x\n", i_addr++, i_val);
				i_val = inb(i_addr);
				printf("get data %x %x\n", i_addr++, i_val);
				i_val = inb(i_addr);
				printf("get data %x %x\n", i_addr++, i_val);
				i_val = inb(i_addr);
				printf("get data %x %x\n\n", i_addr, i_val);
				usleep(1000000);
				i_val = inb(0x602);
				if(i_val&0x01 == 1)
						break;
			}
		} else if(strcmp(tmp, "mux") == 0) {
			scanf("%d %d", &addr, &val);
			printf("set mux to channel %d ", addr);
			if(val == 0) {
				type = 0; printf("voltage\n");
			} else {
				type = 1; printf("current\n");
			}
			if(addr < 1 || addr > 64) 
				printf("channel out of range\n");
			else {
				o_addr = addr - 1;
				o_val = val;
				muxVal1 = (o_addr%16) & 0x0F;
				if(o_val == 0) {
					muxVal1 |= 0x00;
				} else {
					muxVal1 |= 0x10;
				}
				switch(o_addr/16) {
					case 0 : muxVal2 = 0x01; break;
					case 1 : muxVal2 = 0x02; break;
					case 2 : muxVal2 = 0x04; break;
					case 3 : muxVal2 = 0x08; break;
				}
				outb(muxVal1, 0x527);	// MUX1_ADDR
				outb(muxVal2, 0x528);	// MUX2_ADDR
			}
		} else if(strcmp(tmp, "ad") == 0) {
			mode = AD;
			phase = 0;
		} else if(strcmp(tmp, "adj") == 0) {
			scanf("%x %x", &addr, &val);
			printf("adjust AD %x %x\n", addr, val);
			o_addr = addr;
			o_val = (unsigned char)val;
			mode = ADJ;
		} else if(strcmp(tmp, "aux") == 0) {
			scanf("%d %x", &addr, &val);
			printf("AUX DA %d %x\n", addr, val);
			o_addr = addr - 1;
			o_val = (unsigned char)val;
			muxVal = ((o_addr/12) << 4);
			low = swapBit(((o_addr%12)+1)&0x0F);
			muxVal = muxVal | low;
			outb(muxVal, 0x525);
			outb(o_val, 0x526);
			printf("mux value : %02X\n", muxVal);
		} else if(strcmp(tmp, "getad") == 0) {
		while(1)
		{	outb(0,0x621);
			usleep(1000);
			adVal = inb(0x621);	//AD_HADDR
			adVal2 = inb(0x620);//AD_LADDR
			tmpVal = 0;
			tmpVal = adVal;
			tmpVal = tmpVal << 8;
			tmpVal |= adVal2; 
			adVal = 0;
			adVal2 = 0;
			adVal = inb(0x624);	//AD_HADDR
			adVal2 = inb(0x623);//AD_LADDR
			tmpVal1 = 0;
			tmpVal1 = adVal;
			tmpVal1 = tmpVal1 << 8;
			tmpVal1 |= adVal2; 
			
			adI = (float)(10.0*tmpVal/32767.0);//V
			adV = (float)(10.0*tmpVal1/32767.0);//V
			printf(" AD V : %d , I : %d \n",tmpVal1,tmpVal);
			printf(" AD V : %f , I : %f\n \n",adV,adI);
			usleep(1000000);
		}	
		
		} else if(strcmp(tmp, "v") == 0) {
			scanf("%d %f", &ch, &refVal);
			tmpVal = (short int)(29491.0*5.0*refVal/(5.0*9.0));//V
			printf("V reference : %1.3f %02X\n", refVal, tmpVal);
			txData.val = tmpVal;
			ch = ch -1;
			o_addr = 0x01 << (ch/8);
			outb(o_addr, 0x624);	//BD_EN
			outb(txData.data[1], 0x623);	//REF_HIGH
			outb(0x02, 0x62f); // V_CS_EN
			outb(txData.data[0], 0x620 + (ch%8));	//REF_VLT
			outb(0x00, 0x62f); // V_CS_EN
		} else if(strcmp(tmp, "i") == 0) {
			scanf("%d %f", &ch, &refVal);
			tmpVal = (short int)(29491.0*5.0*refVal/(5.0*9.0));//V
//			tmpVal = (short int)(29491.0*8.0*refVal/(10000.0*9.0));//V
			printf("I reference : %1.3f %02X\n", refVal, tmpVal);
			txData.val = tmpVal;
			ch = ch -1;
			o_addr = 0x01 << (ch/8);
			outb(o_addr, 0x624);	//BD_EN
			outb(txData.data[1], 0x623);	//REF_HIGH
			outb(0x04, 0x62f); // I_CS_EN
			outb(txData.data[0], 0x620 + (ch%8));	//REF_VLT
			outb(0x00, 0x62F); // I_CS_EN
		} else if(strcmp(tmp, "daloop") == 0) {
			scanf("%d", &addr);
			printf("daloop %d\n", addr);
			o_addr = addr - 1;
			o_addr &= 0xFF;
			count = 0x00;
			mode = DACHECK;
		} else if(strcmp(tmp, "run") == 0) {
			scanf("%d %d", &addr, &val);	//addr : channel val : on off
			printf("relay %d %d\n", addr, val);
			o_addr = 0x620 + (addr - 1)/8;
			o_val = 0x01;
			if(val == 0) o_val = 0x00;
			else o_val = o_val << ((addr - 1)%8);
			outb(0x01, 0x62f);
			outb(o_val, o_addr);
			outb(0x00, 0x62f);
		} else if(strcmp(tmp, "range") == 0) {
			scanf("%d %d", &addr, &val);	//addr : channel val : on off
			printf("relay %d %d\n", addr, val);
			o_addr = 0x620 + (addr - 1)/8;
			o_val = 0x01;
			if(val == 0) o_val = 0x00;
			else o_val = o_val << ((addr - 1)%8);
			outb(0x02, 0x62f);
			outb(o_val, o_addr);
			outb(0x00, 0x62f);
		} else if(strcmp(tmp, "testdata") == 0){
			o_addr = 0x620;
			o_val = 0x01;
			for(i=0;i<8;i++)
			{
					outb(o_val, o_addr);
					printf("%x %x\n",o_val , o_addr);
					usleep(500000);
					o_val = o_val << 0x01;
			}
		} else if(strcmp(tmp, "auto") == 0){
			scanf("%x",&o_addr);
		//	o_addr =0x600+addr;
			o_val = 0xff;
			outb(o_val, o_addr);
			printf("%x %x\n",o_val , o_addr);
			usleep(200000);

			o_val = 0x01;
			for(i=0;i<8;i++)
			{
					outb(o_val, o_addr);
					printf("%x %x\n",o_val , o_addr);
					usleep(200000);
					o_val = o_val << 0x01;
			}
			outb(0x00, o_addr);

		} else if(strcmp(tmp, "testaddr") == 0){
			o_addr = 0x620;
			o_val = 0x00;
			outb(o_val, o_addr);
			printf("%x %x\n",o_val , o_addr);
			usleep(500000);
			
			o_addr = 0x621;
			outb(o_val, o_addr);
			printf("%x %x\n",o_val , o_addr);
			usleep(500000);
		
			o_addr = 0x622;
			outb(o_val, o_addr);
			printf("%x %x\n",o_val , o_addr);
			usleep(500000);

			o_addr = 0x624;
			outb(o_val, o_addr);
			printf("%x %x\n",o_val , o_addr);
			usleep(500000);
		
			o_addr = 0x628;
			outb(o_val, o_addr);
			printf("%x %x\n",o_val , o_addr);

		} else if(strcmp(tmp, "quit") == 0) exit(0);

		FD_ZERO(&rfds);
		tv.tv_sec = 0;
		tv.tv_usec = 100000;

		retval = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);
		if(retval == 0) {
			if(mode == OUTPUT)
				outb(o_val, o_addr);
			else if(mode == INPUT)
				i_val = inb(i_addr);
			else if(mode == AD) {
				switch(phase) {
					case 0:
						outb(0x00, 0x521);	//AD_START
						phase = 1;
						break;
					case 1:
						adVal = 0;
						adVal = inb(0x521);	//AD_HADDR
						adVal2 = inb(0x520);//AD_LADDR
						tmpVal = 0;
						tmpVal = adVal;
						tmpVal = tmpVal << 8;
						tmpVal |= adVal2;
						if(type == 0)
							printf("write %2.3fV %x\n", (tmpVal*9.0/29491),
								adVal);
						else
							printf("write %4.3fmA %x\n",
								(tmpVal*2000.0/16383.88), adVal);
						printf("ad value : %02X\n", adVal);
						mode = NONE;
						phase = 0;
						break;
				}
			} else if(mode == AD_TEMP) {
				switch(phase) {
					case 0:
						outb(0x00, 0x509);
						printf("kjg1\n");
						phase = 1;
						break;
					case 1:
						adVal = 0;
						adVal = inb(0x509);
						adVal2 = inb(0x508); 
						tmpVal &= 0x00;
						tmpVal |= adVal << 8;
						tmpVal |= adVal2;
						printf("Temperature AD value : %02X\n", tmpVal);
						mode = 0;
						printf("kjg2\n");
						phase = 0;
						break;
				}
			} else if(mode == ADJ) {
				outb(o_addr, 0x529);
				outb(o_val, 0x52a);
			} else if(mode == DACHECK) {
				outb(o_addr, 0x529);
				outb(count, 0x52a);
				count++;
				if(count >= 0xff) count = 0x00;
			}
		}
	}
    exit(0);
}


void Init_Keyboard()
{
		tcgetattr(0, &initial_settings);
		new_settings = initial_settings;
		new_settings.c_lflag &= ~ICANON;
		new_settings.c_lflag &= ~ECHO;
		new_settings.c_lflag &= ~ISIG;
		new_settings.c_cc[VMIN] = 1;
		new_settings.c_cc[VTIME] = 0;
		tcsetattr(0, TCSANOW, &new_settings);
}

void Close_Keyboard()
{
		tcsetattr(0, TCSANOW, &initial_settings);
}

int Keyboard_Hit()
{
		char ch;
		int nread;

		if(peek_character != -1){
				return 1;
		}

		new_settings.c_cc[VMIN] = 0;
		tcsetattr(0, TCSANOW, &new_settings);
		nread = read(0, &ch, 1);
		new_settings.c_cc[VMIN] = 1;
		tcsetattr(0, TCSANOW, &new_settings);

		if(nread == 1){
				peek_character = ch;
				return 1;
		}
		return 0;
}

int Read_Character()
{
		char ch;

		if(peek_character != -1){
				ch = peek_character;
				peek_character = -1;
				return ch;
		}
		read(0, &ch, 1);
		return ch;
}
		
