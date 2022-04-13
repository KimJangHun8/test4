#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include "comm.h"
#include "main.h"

//CommParam comm_param = { "/dev/ttyS1", TMT_B96, TMT_FRTSCTS};
CommParam comm_param = { "/dev/ttyS1", TMT_B96, TMT_FNONE};

char value[256];
FILE *fp;

//////////////// Agilent 34401A Multimeter /////////////////
int main (int argc, char *argv[])
{
    long real;
    char sendData[12];
   // if(FileOpen() < 0) return -1;
		memset(sendData,0,12);
		sendData[0] = '%';
		sendData[1] = '0';
		sendData[2] = '1';
		sendData[3] = '0';
		sendData[4] = '2';
		sendData[5] = '0';
		sendData[6] = 'F';
		sendData[7] = '0';
		sendData[8] = '6';
		sendData[9] = '4';
		sendData[10] = '0';
		printf("Temperature Module setting!!!\n");
		printf("data format %??!!@@##$$ \n");
		printf("?? now module no \n");
		printf("!! new module no \n");
		printf("@@ new type setting K-type(0F) T-type(10) \n");
		printf("## baudrate setting (06)\n");
		printf("$$ data format setting checksum disable(00) checksum enable(40) \n");
		printf("ex) %s \n module no1 to no2 K-type,baud 9600 , checksum enable\n",sendData);
	while(1){
		
		scanf("%s",&sendData);
		
		sendData[11] = 0x0D;
//		printf("sendData\n%s\n\n",sendData);
//		rxsettings("%0002100640\r");
		rxsettings((char *)&sendData);
	}
   // txsettings("SYST:REM\n");
   // txsettings("CONF:VOLT:DC DEF, DEF\n");
//	txsettings("SAMPLE:COUNT 1\n");
	
//	real = RealData();
//	printf("test %s %ld\n", value);
//	SaveData();
//	CloseFile();
    return 0;
}

int FileOpen(void)
{
    if((fp = fopen("cal_data", "w")) == NULL)  return -1;
    return 0;
}

void CloseFile(void)
{
    fclose(fp);
    fp = NULL;
}

int SaveData(void)
{
    if(fp == NULL) return -1;
    fprintf(fp, "%s", value);
    return 0;
}

int RealData(void)
{
    int rtn;
	long real;

    rtn = rxsettings("READ?\n");
    if(value[11] == 'E') {
		real = Conversion();
		if(value[0] == '-') real *= -1L;
    } else real = 0L;
    return real;
}

long Conversion(void)
{
    int tmp, i, exp;
    long real, real1, temp;
	
	temp = 1000000L;
	tmp = CharToInt(value[1]);
	real1 = tmp * 10000000L;
	for(i=3; i <= 10; i++) {
		tmp = CharToInt(value[i]);
		real1 += tmp * temp;
		temp /= 10L;
	}
	
	tmp = CharToInt(value[13]);
	exp = tmp * 10;
	tmp = CharToInt(value[14]);
	exp += tmp;
	
	temp = 1L;
	for(i=0; i < exp; i++) {
		temp *= 10L;
	}
	if(value[12] == '-') {
		real = real1 / temp;
	} else real = real1 * temp;
	return real;
}

int CharToInt(char cmp)
{
    int num;
    char character[10]={'0','1','2','3','4','5','6','7','8','9'};
    
    for(num=0; num<10; num++) {
		if(cmp == character[num]) break;
    }
    return num;
}
/*
//CommParam comm_param = { "/dev/ttyS1", TMT_B96, TMT_FRTSCTS};
CommParam comm_param = { "/dev/ttyS0", TMT_B96, TMT_FXONXOFF};

char value[50];
long value2;
FILE *fp;

//////////////// PREMA 5017 Digital Multimeter /////////////////
int
main (int argc, char *argv[])
{
    int retval, i=0;
    long real;
    struct timeval tv;
    fd_set rfds;
    
    FileOpen();
    
    txsettings("*RST\n");
    txsettings("*CLS\n");
    txsettings("T4IDR5A1L0\n");
    
    while(1) {
	i++;
	if(i>1) {
	    CloseFile();
	    break;
	}
    	tv.tv_sec = 1;
	tv.tv_usec = 0;
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);
	retval = select(0, &rfds, NULL, NULL, &tv);
	if(retval == 0) {
	    real = RealData();
	    printf("test %s %ld\n", value, real);
	    SaveData();
	}//if
    }//while
    return 0;
}

int
FileOpen()
{
    if((fp = fopen("cal_data", "w")) == NULL)  return -1;
    return 0;
}

void
CloseFile()
{
    fclose(fp);
    fp = NULL;
}

int
SaveData()
{
    if(fp == NULL) return -1;
    fprintf(fp, "%s", value);
    return 0;
}

int
Conversion(int dot)
{
    int tmp, i;
    long temp;
    switch(dot) {
       case 1:
	   temp = 10000000L;
	   for(i=0; i<8; i++) {
	       tmp=CharToInt(value[i+2]);
	       value2+=tmp*temp;
	       temp = temp/10;
	   }
	   if(value[11] == '+') {
	       switch(value[12]) {
		   case '0':
		       value2 = value2 / 10L;
		       break;
		   case '1':
		       break;
		   case '2':
		       value2 = value2 * 10L;
		       break;
		   case '3':
		       value2 = value2 * 100L;
		       break;
		   case '4':
		       break;
		   case '5':
		       value2 = value2 * 1000L;
		       break;
		   case '6':
		       value2 = value2 * 10000L;
		       break;
		   case '7':
		       value2 = value2 * 100000L;
		       break;
		   case '8':
		       value2 = value2 * 1000000L;
		       break;
		   case '9':
		       value2 = value2 * 10000000L;
		       break;
		   default:
		       break;
	       }
	   } else {// value[11] == '-'
	       switch(value[12]) {
		   case '1':
		       value2 = value2 / 100L;
		       break;
		   case '2':
		       value2 = value2 / 1000L;
		       break;
		   case '3':
		       value2 = value2 / 10000L;
		       break;
		   case '4':
		       value2 = value2 / 100000L;
		       break;
		   case '5':
		       value2 = value2 / 1000000L;
		       break;
		   case '6':
		       value2 = value2 / 10000000L;
		       break;
		   case '7':
		       value2 = value2 / 100000000L;
		       break;
		   case '8':
		       value2 = value2 / 1000000000L;
		       break;
		   case '9':
		       value2 = 0;//value2 / 10000000000L;
		       break;
		   default:
		       break;
	       }
	   }
	   break;
       case 2:
	   tmp=CharToInt(value[1]);
	   value2 = tmp*10000000L;
	   temp = 1000000L;
	   for(i=0; i<7; i++) {
	       tmp=CharToInt(value[i+3]);
	       value2+=tmp*temp;
	       temp = temp/10;
	   }
	   if(value[11] == '+') {
	       switch(value[12]) {
		   case '0':
		       break;
		   case '1':
		       value2 = value2 * 10L;
		       break;
		   case '2':
		       value2 = value2 * 100L;
		       break;
		   case '3':
		       value2 = value2 * 1000L;
		       break;
		   case '4':
		       value2 = value2 * 10000L;
		       break;
		   case '5':
		       value2 = value2 * 100000L;
		       break;
		   case '6':
		       value2 = value2 * 1000000L;
		       break;
		   case '7':
		       value2 = value2 * 10000000L;
		       break;
		   case '8':
		       value2 = value2 * 100000000L;
		       break;
		   case '9':
		       value2 = value2 * 1000000000L;
		       break;
		   default:
		       break;
	       }
	   } else {// value[11] == '-'
	       switch(value[12]) {
		   case '1':
		       value2 = value2 / 10L;
		       break;
		   case '2':
		       value2 = value2 / 100L;
		       break;
		   case '3':
		       value2 = value2 / 1000L;
		       break;
		   case '4':
		       value2 = value2 / 10000L;
		       break;
		   case '5':
		       value2 = value2 / 100000L;
		       break;
		   case '6':
		       value2 = value2 / 1000000L;
		       break;
		   case '7':
		       value2 = value2 / 10000000L;
		       break;
		   case '8':
		       value2 = value2 / 100000000L;
		       break;
		   case '9':
		       value2 = value2 / 1000000000L;
		       break;
		   default:
		       break;
	       }
	   }
	   break;
       case 3:
	   tmp=CharToInt(value[1]);
	   value2 = tmp*10000000L;
	   tmp=CharToInt(value[2]);
	   value2 += tmp*1000000L;
	   temp = 100000L;
	   for(i=0; i<6; i++) {
	       tmp=CharToInt(value[i+4]);
	       value2 += tmp*temp;
	       temp = temp/10;
	   }
	   if(value[11] == '+') {
	       switch(value[12]) {
		   case '0':
		       value2 = value2 * 10L;
		       break;
		   case '1':
		       value2 = value2 * 100L;
		       break;
		   case '2':
		       value2 = value2 * 1000L;
		       break;
		   case '3':
		       value2 = value2 * 10000L;
		       break;
		   case '4':
		       value2 = value2 * 100000L;
		       break;
		   case '5':
		       value2 = value2 * 1000000L;
		       break;
		   case '6':
		       value2 = value2 * 10000000L;
		       break;
		   case '7':
		       value2 = value2 * 100000000L;
		       break;
		   case '8':
		       value2 = value2 * 1000000000L;
		       break;
		   case '9':
		       value2 = 0;//value2 * 10000000000L;
		       break;
		   default:
		       break;
	       }
	   } else {// value[11] == '-'
	       switch(value[12]) {
		   case '1':
		       break;
		   case '2':
		       value2 = value2 / 10L;
		       break;
		   case '3':
		       value2 = value2 / 100L;
		       break;
		   case '4':
		       value2 = value2 / 1000L;
		       break;
		   case '5':
		       value2 = value2 / 10000L;
		       break;
		   case '6':
		       value2 = value2 / 100000L;
		       break;
		   case '7':
		       value2 = value2 / 1000000L;
		       break;
		   case '8':
		       value2 = value2 / 10000000L;
		       break;
		   case '9':
		       value2 = value2 / 100000000L;
		       break;
		   default:
		       break;
	       }
	   }
	   break;
       case 4:
	   tmp=CharToInt(value[1]);
	   value2 = tmp*10000000L;
	   tmp=CharToInt(value[2]);
	   value2 += tmp*1000000L;
	   tmp=CharToInt(value[3]);
	   value2 += tmp*100000L;
	   temp = 10000L;
	   for(i=0; i<5; i++) {
	       tmp=CharToInt(value[i+5]);
	       value2 += tmp*temp;
	       temp = temp / 10;
	   }
	   if(value[11] == '+') {
	       switch(value[12]) {
		   case '0':
		       value2 = value2 * 100L;
		       break;
		   case '1':
		       value2 = value2 * 1000L;
		       break;
		   case '2':
		       value2 = value2 * 10000L;
		       break;
		   case '3':
		       value2 = value2 * 100000L;
		       break;
		   case '4':
		       value2 = value2 * 1000000L;
		       break;
		   case '5':
		       value2 = value2 * 10000000L;
		       break;
		   case '6':
		       value2 = value2 * 100000000L;
		       break;
		   case '7':
		       value2 = value2 * 1000000000L;
		       break;
		   case '8':
		       value2 = 0;//value2 * 10000000000L;
		       break;
		   case '9':
		       value2 = 0;//value2 * 100000000000L;
		       break;
		   default:
		       break;
	       }
	   } else {// value[11] == '-'
	       switch(value[12]) {
		   case '1':
		       value2 = value2 * 10L;
		       break;
		   case '2':
		       break;
		   case '3':
		       value2 = value2 / 10L;
		       break;
		   case '4':
		       value2 = value2 / 100L;
		       break;
		   case '5':
		       value2 = value2 / 1000L;
		       break;
		   case '6':
		       value2 = value2 / 10000L;
		       break;
		   case '7':
		       value2 = value2 / 100000L;
		       break;
		   case '8':
		       value2 = value2 / 1000000L;
		       break;
		   case '9':
		       value2 = value2 / 10000000L;
		       break;
		   default:
		       break;
	       }
	   }
	   break;
       case 5:
       case 6:
       case 7:
       case 8:
       case 9:
	   break;
       default: dot=-1; value2=0; break;
   }
   return dot;
}

int
CharToInt(char cmp)
{
    int num;
    char character[10]={'0','1','2','3','4','5','6','7','8','9'};
    
    for(num=0; num<10; num++) {
	if(cmp==character[num]) break;
    }
    return num;
}

int
RealData()
{
    int rtn, div, i;
    
    rtn = rxsettings("RD?\n");
    value2 = 0;
    if(value[10]=='E') {
	for(i=1; i<10; i++) {
	    if(value[i] == '.') {
		div=Conversion(i);
		break;
	    }
	}
	if(value[0] == '-') value2 *= -1L;
    } else { div=-1; value2=0; }
    return value2;
}
*/
