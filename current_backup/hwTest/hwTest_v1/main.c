#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <asm/io.h>

#include <stdlib.h>
#include <termios.h>
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
#define FAD_ADDR	0x700

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
    float	refVal,adV,adI,compPlus[16],compMinus[16],setPlus[16],setMinus[16]
			,comp5V[2],comp0V[2];
    char	tmp[10];
	unsigned char	o_val, i_val, muxVal; 
	unsigned char muxVal1, muxVal2, muxVal3, muxVal4;
    unsigned char	adVal, adVal2,inval;
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

		if(strcmp(tmp, "fad1") == 0){
			unsigned char fad[1000];
			for(i=0;i<700;i++){
				fad[i] =inb(FAD_ADDR);
			}
			for(i=0;i<5;i++){
				printf("%2x\t",fad[i]);
			}
			printf("\n");
			for(i=5;i<609;i++){
				printf("%2x\t",fad[i]);
			}
			printf("\n");
			for(i=609;i<700;i++){
				printf("%2x\t",fad[i]);
			}
		}else if(strcmp(tmp, "fad2") == 0){
			unsigned char fad[1000];
			for(i=0;i<700;i++){
				fad[i] =inb(FAD_ADDR+1);
			}
			for(i=0;i<5;i++){
				printf("%2x\t",fad[i]);
			}
			printf("\n");
			for(i=5;i<609;i++){
				printf("%2x\t",fad[i]);
			}
			printf("\n");
			for(i=609;i<700;i++){
				printf("%2x\t",fad[i]);
			}
		}else if(strcmp(tmp, "fad3") == 0){
			unsigned char fad[1000];
			for(i=0;i<700;i++){
				fad[i] =inb(FAD_ADDR+2);
			}
			for(i=0;i<5;i++){
				printf("%2x\t",fad[i]);
			}
			printf("\n");
			for(i=5;i<609;i++){
				printf("%2x\t",fad[i]);
			}
			printf("\n");
			for(i=609;i<700;i++){
				printf("%2x\t",fad[i]);
			}
		}else if(strcmp(tmp, "fad4") == 0){
			unsigned char fad[1000];
			for(i=0;i<700;i++){
				fad[i] =inb(FAD_ADDR+3);
			}
			for(i=0;i<5;i++){
				printf("%2x\t",fad[i]);
			}
			printf("\n");
			for(i=5;i<609;i++){
				printf("%2x\t",fad[i]);
			}
			printf("\n");
			for(i=609;i<700;i++){
				printf("%2x\t",fad[i]);
			}
		}else if(strcmp(tmp, "fad5") == 0){
			unsigned char fad[1000];
			for(i=0;i<700;i++){
				fad[i] =inb(FAD_ADDR+4);
			}
			for(i=0;i<5;i++){
				printf("%2x\t",fad[i]);
			}
			printf("\n");
			for(i=5;i<609;i++){
				printf("%2x\t",fad[i]);
			}
			printf("\n");
			for(i=609;i<700;i++){
				printf("%2x\t",fad[i]);
			}
		}else if(strcmp(tmp, "fad6") == 0){
			unsigned char fad[1000];
			for(i=0;i<700;i++){
				fad[i] =inb(FAD_ADDR+5);
			}
			for(i=0;i<5;i++){
				printf("%2x\t",fad[i]);
			}
			printf("\n");
			for(i=5;i<609;i++){
				printf("%2x\t",fad[i]);
			}
			printf("\n");
			for(i=609;i<700;i++){
				printf("%2x\t",fad[i]);
			}
		}else if(strcmp(tmp, "fad7") == 0){
			unsigned char fad[1000];
			for(i=0;i<700;i++){
				fad[i] =inb(FAD_ADDR+6);
			}
			for(i=0;i<5;i++){
				printf("%2x\t",fad[i]);
			}
			printf("\n");
			for(i=5;i<609;i++){
				printf("%2x\t",fad[i]);
			}
			printf("\n");
			for(i=609;i<700;i++){
				printf("%2x\t",fad[i]);
			}
		}else if(strcmp(tmp, "fad8") == 0){
			unsigned char fad[1000];
			for(i=0;i<700;i++){
				fad[i] =inb(FAD_ADDR+7);
			}
			for(i=0;i<5;i++){
				printf("%2x\t",fad[i]);
			}
			printf("\n");
			for(i=5;i<609;i++){
				printf("%2x\t",fad[i]);
			}
			printf("\n");
			for(i=609;i<700;i++){
				printf("%2x\t",fad[i]);
			}
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
					printf("PNE 200807-Control B/D REV01 DIO_TEST\n"); 
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
					printf("'o' - OT, SMPS 7, 3.3V Fail, Cal PORT TEST\n");
					printf("'q' - EXIT\n");
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
		}else if(strcmp(tmp, "vref") == 0) {
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
		} else if(strcmp(tmp, "iref") == 0) {
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
		} else if(strcmp(tmp, "ppp") == 0) {
			while(1){
				sleep(1);
				i_val = inb(0x628);
				printf("%d\n", i_val);
			}

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
			printf("wr %x %02x\n", addr, val);
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
			printf("get data %x %02x\n", i_addr, i_val);
		} else if(strcmp(tmp, "m") == 0) {
			scanf("%d", &ch);
			outb(ch-1 | 0x10,0x665);
		} else if(strcmp(tmp, "p") == 0) {
			scanf("%d", &ch);
			outb(ch-1,0x665);
		} else if(strcmp(tmp, "comp") == 0) {
		//	scanf("%x", &addr);
		//	printf("get %x\n", addr);
			addr = 0x600;
			while(1){
				mode = NONE;
			addr = 0x660;
				i_addr = addr;
				i_val = inb(i_addr);
				inval = 0x01;
				printf("comp plus fault : ");
				for(i = 0; i < 8 ; i++) {
					if(inval & i_val) printf("ch%d ",i+1);
					inval = inval << 1;
				}
				printf("\n");
				i_addr++;
				i_val = inb(i_addr);
				inval = 0x01;
				printf("comp minus fault : ");
				for(i = 0; i < 8 ; i++) {
					if(inval & i_val) printf("ch%d ",i+1);
					inval = inval << 1;
				}
				printf("\n");
				i_addr++;
				i_val = inb(i_addr);
				inval = 0x01;
				printf("ch comp fault : ");
				for(i = 0; i < 8 ; i++) {
					if(inval & i_val) printf("ch%d ",i+1);
					inval = inval << 1;
				}
				printf("\n");
				i_addr++;
				i_val = inb(i_addr);
				inval = 0x01;
				printf("ch ov falut : ");
				for(i = 0; i < 8 ; i++) {
					if(inval & i_val) printf("ch%d ",i+1);
					inval = inval << 1;
				}
				printf("\n");
				i_addr++;
				i_val = inb(i_addr);
				inval = 0x01;
				printf("ch oc falut : ");
				for(i = 0; i < 8 ; i++) {
					if(inval & i_val) printf("ch%d ",i+1);
					inval = inval << 1;
				}
				printf("\n");
				i_addr++;
				i_val = inb(i_addr);
				inval = 0x01;
				printf("ch ot falut : ");
				for(i = 0; i < 8 ; i++) {
					if(inval & i_val) printf("ch%d ",i+1);
					inval = inval << 1;
				}
				printf("\n");
				i_addr++;
				i_val = inb(i_addr);
				inval = 0x01;
				printf("ch meter high falut : ");
				for(i = 0; i < 8 ; i++) {
					if(inval & i_val) printf("ch%d ",i+1);
					inval = inval << 1;
				}
				printf("\n");
				i_addr++;
				i_val = inb(i_addr);
				inval = 0x01;
				printf("ch meter low falut : ");
				for(i = 0; i < 8 ; i++) {
					if(inval & i_val) printf("ch%d ",i+1);
					inval = inval << 1;
				}
				printf("\n");
				printf("\n\n");
				
				outb(0xff,0x666);
				for(i=0;i<8;i++) {
					outb(i,0x665);
					usleep(100000);
					adVal = inb(0x668);	//AD_HADDR
					adVal2 = inb(0x669);//AD_LADDR
					tmpVal = 0;
					tmpVal = adVal;
					tmpVal = tmpVal << 8;
					tmpVal |= adVal2; 
					adI = (float)(10.0*tmpVal/32767.0);//V
			//		printf("ch%d plus ad val : %d , %f\n",i+1,tmpVal,adI);
					compPlus[i] = adI;
					adVal = inb(0x66a);	//AD_HADDR
					adVal2 = inb(0x66b);//AD_LADDR
					tmpVal = 0;
					tmpVal = adVal;
					tmpVal = tmpVal << 8;
					tmpVal |= adVal2; 
					adI = (float)(10.0*tmpVal/32767.0);//V
					setPlus[i] = adI;
				}
				for(i=0;i<8;i++) {
					outb(0x10+i,0x665);
					usleep(100000);
					adVal = inb(0x668);	//AD_HADDR
					adVal2 = inb(0x669);//AD_LADDR
					tmpVal = 0;
					tmpVal = adVal;
					tmpVal = tmpVal << 8;
					tmpVal |= adVal2; 
					adI = (float)(10.0*tmpVal/32767.0);//V
		//			printf("ch%d minus ad val : %d , %f\n",i+1,tmpVal,adI);
					compMinus[i] = adI;
					adVal = inb(0x66a);	//AD_HADDR
					adVal2 = inb(0x66b);//AD_LADDR
					tmpVal = 0;
					tmpVal = adVal;
					tmpVal = tmpVal << 8;
					tmpVal |= adVal2; 
					adI = (float)(10.0*tmpVal/32767.0);//V
					setMinus[i] = adI;
				}
				outb(0x20,0x665);
				usleep(100000);
				adVal = inb(0x668);	//AD_HADDR
				adVal2 = inb(0x669);//AD_LADDR
				tmpVal = 0;
				tmpVal = adVal;
				tmpVal = tmpVal << 8;
				tmpVal |= adVal2; 
				adI = (float)(10.0*tmpVal/32767.0);//V
				comp5V[0] = adI;
				outb(0x30,0x665);
				usleep(100000);
				adVal = inb(0x668);	//AD_HADDR
				adVal2 = inb(0x669);//AD_LADDR
				tmpVal = 0;
				tmpVal = adVal;
				tmpVal = tmpVal << 8;
				tmpVal |= adVal2; 
				adI = (float)(10.0*tmpVal/32767.0);//V
				comp0V[0] = adI;
				for(i=0;i<8;i++) {
					printf("ch %d plus %1.3f minus %1.3f setPlus %1.3f setMinus %1.3f\n",i+1,compPlus[i],compMinus[i],setPlus[i],setMinus[i]);
				}
				printf("5V %1.3f 0V %1.3f \n",comp5V[0],comp0V[0]);

				i_val = inb(0x602);
				if(i_val&0x01 == 1)
						break;
			}
		} else if(strcmp(tmp, "comp1") == 0) {
		//	scanf("%x", &addr);
		//	printf("get %x\n", addr);
			addr = 0x600;
			while(1){
				mode = NONE;
			addr = 0x6a0;
				i_addr = addr;
				i_val = inb(i_addr);
				inval = 0x01;
				printf("comp plus fault : ");
				for(i = 0; i < 2 ; i++) {
					if(inval & i_val) printf("ch%d ",i+9);
					inval = inval << 1;
				}
				printf("\n");
				i_addr++;
				i_val = inb(i_addr);
				inval = 0x01;
				printf("comp minus fault : ");
				for(i = 0; i < 2 ; i++) {
					if(inval & i_val) printf("ch%d ",i+9);
					inval = inval << 1;
				}
				printf("\n");
				i_addr++;
				i_val = inb(i_addr);
				inval = 0x01;
				printf("ch comp fault : ");
				for(i = 0; i < 2 ; i++) {
					if(inval & i_val) printf("ch%d ",i+9);
					inval = inval << 1;
				}
				printf("\n");
				i_addr++;
				i_val = inb(i_addr);
				inval = 0x01;
				printf("ch ov falut : ");
				for(i = 0; i < 2 ; i++) {
					if(inval & i_val) printf("ch%d ",i+9);
					inval = inval << 1;
				}
				printf("\n");
				i_addr++;
				i_val = inb(i_addr);
				inval = 0x01;
				printf("ch oc falut : ");
				for(i = 0; i < 2 ; i++) {
					if(inval & i_val) printf("ch%d ",i+9);
					inval = inval << 1;
				}
				printf("\n");
				i_addr++;
				i_val = inb(i_addr);
				inval = 0x01;
				printf("ch ot falut : ");
				for(i = 0; i < 2 ; i++) {
					if(inval & i_val) printf("ch%d ",i+9);
					inval = inval << 1;
				}
				printf("\n");
				i_addr++;
				i_val = inb(i_addr);
				inval = 0x01;
				printf("ch meter high falut : ");
				for(i = 0; i < 2 ; i++) {
					if(inval & i_val) printf("ch%d ",i+9);
					inval = inval << 1;
				}
				printf("\n");
				i_addr++;
				i_val = inb(i_addr);
				inval = 0x01;
				printf("ch meter low falut : ");
				for(i = 0; i < 2 ; i++) {
					if(inval & i_val) printf("ch%d ",i+9);
					inval = inval << 1;
				}
				printf("\n");
				printf("\n\n");
				
			//	outb(0xff,0x6A6);
				for(i=0;i<2;i++) {
					outb(i,0x6a5);
					usleep(100000);
					adVal = inb(0x6a8);	//AD_HADDR
					adVal2 = inb(0x6a9);//AD_LADDR
					tmpVal = 0;
					tmpVal = adVal;
					tmpVal = tmpVal << 8;
					tmpVal |= adVal2; 
					adI = (float)(10.0*tmpVal/32767.0);//V
			//		printf("ch%d plus ad val : %d , %f\n",i+1,tmpVal,adI);
					compPlus[i+8] = adI;
					adVal = inb(0x6aa);	//AD_HADDR
					adVal2 = inb(0x6ab);//AD_LADDR
					tmpVal = 0;
					tmpVal = adVal;
					tmpVal = tmpVal << 8;
					tmpVal |= adVal2; 
					adI = (float)(10.0*tmpVal/32767.0);//V
					setPlus[i+8] = adI;
				}
				for(i=0;i<2;i++) {
					outb(0x10+i,0x6a5);
					usleep(100000);
					adVal = inb(0x6a8);	//AD_HADDR
					adVal2 = inb(0x6a9);//AD_LADDR
					tmpVal = 0;
					tmpVal = adVal;
					tmpVal = tmpVal << 8;
					tmpVal |= adVal2; 
					adI = (float)(10.0*tmpVal/32767.0);//V
		//			printf("ch%d minus ad val : %d , %f\n",i+1,tmpVal,adI);
					compMinus[i+8] = adI;
					adVal = inb(0x6aa);	//AD_HADDR
					adVal2 = inb(0x6ab);//AD_LADDR
					tmpVal = 0;
					tmpVal = adVal;
					tmpVal = tmpVal << 8;
					tmpVal |= adVal2; 
					adI = (float)(10.0*tmpVal/32767.0);//V
					setMinus[i+8] = adI;
				}
				outb(0x20,0x6a5);
				usleep(100000);
				adVal = inb(0x6a8);	//AD_HADDR
				adVal2 = inb(0x6a9);//AD_LADDR
				tmpVal = 0;
				tmpVal = adVal;
				tmpVal = tmpVal << 8;
				tmpVal |= adVal2; 
				adI = (float)(10.0*tmpVal/32767.0);//V
				comp5V[1] = adI;
				outb(0x30,0x6a5);
				usleep(100000);
				adVal = inb(0x6a8);	//AD_HADDR
				adVal2 = inb(0x6a9);//AD_LADDR
				tmpVal = 0;
				tmpVal = adVal;
				tmpVal = tmpVal << 8;
				tmpVal |= adVal2; 
				adI = (float)(10.0*tmpVal/32767.0);//V
				comp0V[1] = adI;
				for(i=0;i<2;i++) {
					printf("ch %d plus %1.3f minus %1.3f setPlus %1.3f setMinus %1.3f\n",i+8,compPlus[i+9],compMinus[i+8],setPlus[i+8],setMinus[i+8]);
				}
				printf("5V %1.3f 0V %1.3f \n",comp5V[1],comp0V[1]);

				
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
		while(1) {
			outb(0,0x621);
			usleep(100000);
			adVal = inb(0x621);	//AD_HADDR
			adVal2 = inb(0x620);//AD_LADDR
			printf("i high %x low %x\n",adVal,adVal2);
			tmpVal = 0;
			tmpVal = adVal;
			tmpVal = tmpVal << 8;
			tmpVal |= adVal2; 
			adVal = 0;
			adVal2 = 0;
			adVal = inb(0x624);	//AD_HADDR
			adVal2 = inb(0x623);//AD_LADDR
			printf("V high %x low %x\n",adVal,adVal2);
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
		/*
		//DA 712P Main BD Type refV
		} else if(strcmp(tmp, "v") == 0) {
			scanf("%d %f", &ch, &refVal);
			tmpVal = (short int)(29491.0*5.0*refVal/(5.0*9.0));//V
			printf("V reference : %1.3f %02X\n", refVal, tmpVal);
			txData.val = tmpVal;
			ch = ch -1;
			o_addr = 0x01 << (ch/8);
	//		outb(o_addr, 0x624);	//BD_EN
			outb(txData.data[1], 0x663);	//REF_HIGH
			outb(0x01, 0x67f); // V_CS_EN
			outb(txData.data[0], 0x670 + (ch%8));	//REF_VLT
			outb(0x00, 0x67f); // V_CS_EN
*/
		//DA 712P Mother BD Type refV	
		}else if(strcmp(tmp, "v") == 0){
			scanf("%d %f", &ch, &refVal);
	//		tmpVal = (short int)(29491.0 * refVal/(5.0 * 9.0));	//V
			tmpVal = (short int)(29491.0 * 5.0 * refVal/(5.0*9.0));//V
			printf("V reference : %1.3f %02X\n", refVal, tmpVal);
			txData.val =tmpVal;
			ch = ch -1;
	/*		o_addr = 0x01;
			outb(~o_addr, 0x624);					//BD_EN
			outb(txData.data[1], 0x623);			//REF_HIGH
			outb(0x02  , 0x63f);		//V_CS_EN
			outb(txData.data[0], 0x630 + (ch%8));	//REF_VLT
			outb(0x00, 0x63f);						//V_CS_EN
			outb(0xff, 0x624);				//V_CS_EN
			*/
			outb(txData.data[1],0x663);
			outb(0x01,0x67f);
			outb(txData.data[0],0x660 + (ch%8));
			outb(0x00,0x67f);
/*		
		//DA 7741 Main BD Type refV		
		} else if(strcmp(tmp, "v") == 0) {
			scanf("%d %f", &ch, &refVal);
			tmpVal = (short int)(29491.0*5.0*refVal/(5.0*9.0));//V
			printf("V reference : %1.3f %02X\n", refVal, tmpVal);
			txData.val = tmpVal ^ 0x8000;
			ch = ch -1;
			o_addr = 0x01 << (ch/8);
			outb(o_addr, 0x624);	//BD_EN
			outb(txData.data[1], 0x623);	//REF_HIGH
			outb(0x02, 0x62f); // V_CS_EN
			outb(txData.data[0], 0x620 + (ch%8));	//REF_VLT
			outb(0x00, 0x62f); // V_CS_EN
*/
			/*
		//DA 712P Main BD Type refI
		} else if(strcmp(tmp, "i") == 0) {
			scanf("%d %f", &ch, &refVal);
			tmpVal = (short int)(29491.0*5.0*refVal/(5.0*9.0));//V
//			tmpVal = (short int)(29491.0*8.0*refVal/(10000.0*9.0));//V
			printf("I reference : %1.3f %02X\n", refVal, tmpVal);
			txData.val = tmpVal;
			ch = ch -1;
			o_addr = 0x01 << (ch/8);
			outb(~o_addr, 0x624);	//BD_EN
			outb(txData.data[1], 0x663);	//REF_HIGH
			outb(0x02, 0x67f); // I_CS_EN
			outb(txData.data[0], 0x670 + (ch%8));	//REF_VLT
			outb(0x00, 0x67F); // I_CS_EN
			*/
		//DA 712P Mother BD Type refI			
		}else if(strcmp(tmp, "i") == 0){
			scanf("%d %f", &ch, &refVal);
	//		tmpVal = (short int)(29491.0 * refVal/(5.0 * 9.0));	//V
			tmpVal = (short int)(29491.0 * 5.0 * refVal/(5.0*9.0));//V
			printf("V reference : %1.3f %02X\n", refVal, tmpVal);
			txData.val =tmpVal;
			ch = ch -1;
			o_addr = 0x01;
			outb(~o_addr, 0x624);					//BD_EN
			outb(txData.data[1], 0x623);			//REF_HIGH
			outb(0x04  , 0x63f);		//V_CS_EN
			outb(txData.data[0], 0x630 + (ch%8));	//REF_VLT
			outb(0x00, 0x63f);						//V_CS_EN
			outb(0xff, 0x624);						//V_CS_EN
/*
		//DA 7741 Main BD Type refI	
		} else if(strcmp(tmp, "i") == 0) {
			scanf("%d %f", &ch, &refVal);
			tmpVal = (short int)(29491.0*5.0*refVal/(5.0*9.0));//V
//			tmpVal = (short int)(29491.0*8.0*refVal/(10000.0*9.0));//V
			printf("I reference : %1.3f %02X\n", refVal, tmpVal);
			txData.val = tmpVal ^ 0x8000;
			ch = ch -1;
			o_addr = 0x01 << (ch/8);
			outb(o_addr, 0x624);	//BD_EN
			outb(txData.data[1], 0x623);	//REF_HIGH
			outb(0x04, 0x62f); // I_CS_EN
			outb(txData.data[0], 0x620 + (ch%8));	//REF_VLT
			outb(0x00, 0x62F); // I_CS_EN
*/

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
					printf("%x %02x\n",o_addr, o_val);
					usleep(500000);
					o_val = o_val << 0x01;
			}
		} else if(strcmp(tmp, "autoget") == 0){
			scanf("%x", &o_addr);
			for(i = 0; i < 10000; i++){
				sleep(1);
				o_val = inb(o_addr);
				printf("%x %02x\n",o_addr, o_val);
				o_val = inb(o_addr+1);
				printf("%x %02x\n",o_addr+1, o_val);
				o_val = inb(o_addr+2);
				printf("%x %02x\n",o_addr+2, o_val);
				o_val = inb(o_addr+3);
				printf("%x %02x\n",o_addr+3, o_val);
				o_val = inb(o_addr+4);
				printf("%x %02x\n",o_addr+4, o_val);
				o_val = inb(o_addr+5);
				printf("%x %02x\n",o_addr+5, o_val);
				o_val = inb(o_addr+6);
				printf("%x %02x\n",o_addr+6, o_val);
				o_val = inb(o_addr+7);
				printf("%x %02x\n",o_addr+7, o_val);
				printf("\n");
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
					usleep(500000);
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
