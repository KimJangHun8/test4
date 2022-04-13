#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include "../../INC/datastore.h"
#include "common_utils.h"

volatile S_SYSTEM_DATA *myData;
int bd, ch, mode;
int test;

void TestPrint(void)
{
	int i;
	char *asciitime;
	struct tm *tm;
	time_t t;

	time(&t);
	asciitime = ctime(&t);
	tm = localtime(&t);

	asciitime[24] = 0;
	printf("time %s, %d, %d, %d\n", asciitime, tm->tm_year, tm->tm_mon+1, tm->tm_mday);

	printf("voltage\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
		if(mode==0)
			printf("%ld ", myData->bData[bd].cData[i].op.Vsens);
		else
			printf("%ld ", myData->bData[bd].cData[i].opSave.Vsens);
	}
	printf("\n");
	
	printf("current\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
		if(mode==0) {
			printf("%ld ", myData->bData[bd].cData[i].op.Isens);
		} else
			printf("%ld ", myData->bData[bd].cData[i].opSave.Isens);
	}
	printf("\n");
	printf("\n");
}

int KeyInput(void)
{
	int i, rtn, fp;
	char cmd[128];
	rtn = 0;
	i = getc(stdin);
	switch(i) {
		case '1':	bd=0; mode=0; break;
		case '2':	bd=1; mode=0; break;
		case '3':	bd=2; mode=0; break;
		case '4':	bd=3; mode=0; break;
		case '5':	bd=4; mode=0; break;
		case '6':	bd=5; mode=0; break;
		case '7':	bd=6; mode=0; break;
		case '8':	bd=7; mode=0; break;
		case 'a':	bd=0; mode=1; break;
		case 'b':	bd=1; mode=1; break;
		case 'c':	bd=2; mode=1; break;
		case 'd':	bd=3; mode=1; break;
		case 'e':	bd=4; mode=1; break;
		case 'f':	bd=5; mode=1; break;
		case 'g':	bd=6; mode=1; break;
		case 'h':	bd=7; mode=1; break;
		case 'w':
			printf("before dumpData Delete...\n");
			strcpy(cmd,"rm -rf ./dumpData ; touch dumpData");
			system(cmd);
			if((fp = open("./dumpData", O_RDWR)) < 0) {
				printf("Can not open dumpData file(save)\n");
				close(fp);
				break;
			}
			printf("write dumpData\n");
			rtn = write(fp, (char *)myData, sizeof(S_SYSTEM_DATA));
			if(rtn != sizeof(S_SYSTEM_DATA)) {
				printf("error dumpData write\n");
			}
			close(fp);
			break;
		case 'r':
			if((fp = open("./dumpData", O_RDONLY)) < 0) {
				printf("Can not open dumpData file(load)\n");
				close(fp);
				break;
			}
			printf("read dumpData\n");
			rtn = read(fp, (char *)myData, sizeof(S_SYSTEM_DATA));
			if(rtn != sizeof(S_SYSTEM_DATA)) {
				printf("error dumpData read %d\n", rtn);
			}
			close(fp);
			break;
		case 'z':
			printf("before dumpData Delete...\n");
			strcpy(cmd,"rm -rf ./dumpData ; touch dumpData");
			system(cmd);
			break;
		case 'q': rtn=-1; break;
		default: break;
	}
	return rtn;
}

int main(void)
{
    int	retval, rtn;
    struct timeval tv;
    fd_set rfds;

	if(Open_SystemMemory(0) < 0) return -1;

	bd = 0;
	ch = 0;
	mode = 0;
	test = 0;

    while(1) {
	    tv.tv_sec = 1;
	    tv.tv_usec = 0;
	    FD_ZERO(&rfds);
	    FD_SET(0, &rfds);
		retval = select(1, &rfds, NULL, NULL, &tv);
	    if(retval == 0) {
			TestPrint();
		} else {
			printf("rtn %d\n", retval);
			rtn = KeyInput();
			if(rtn < 0) break;
		}
    }

	Close_SystemMemory();
    return 0;
}
