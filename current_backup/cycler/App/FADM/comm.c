#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "comm.h"

CommParam comm_param;// = {"/dev/ttyS0", TMT_B96, TMT_FXONXOFF};
static struct termios	oldtrm;

int opentty(void)
{
	int fd;
	struct termios trm;
	
	fd = open(comm_param.device, O_RDWR | O_NOCTTY | O_NDELAY);
	if(fd < 0) {
		printf("open error %d : %s", fd, comm_param.device);
		return -1;
	}

	tcgetattr(fd, &oldtrm);	/* salva il setup attuale della porta */
	bzero(&trm, sizeof(struct termios));
	switch(comm_param.bps) {
		case TMT_B3:	trm.c_cflag = B300 | CS8 | CREAD;	break;
		case TMT_B6:	trm.c_cflag = B600 | CS8 | CREAD;	break;
		case TMT_B12:	trm.c_cflag = B1200 | CS8 | CREAD;	break;
		case TMT_B24:	trm.c_cflag = B2400 | CS8 | CREAD;	break;
		case TMT_B48:	trm.c_cflag = B4800 | CS8 | CREAD;	break;
		case TMT_B96:	trm.c_cflag = B9600 | CS8 | CREAD;	break;
		/*case TMT_B96:
			trm.c_cflag = B9600 | CS7 | CREAD | CSTOPB | PARENB;
			break;*/
		case TMT_B192:	trm.c_cflag = B19200 | CS8 | CREAD;	break;
		case TMT_B384:	trm.c_cflag = B38400 | CS8 | CREAD;	break;
	}
	switch(comm_param.flow) {
		case TMT_FRTSCTS:	trm.c_cflag |= CRTSCTS;			break;
		case TMT_FXONXOFF:	trm.c_iflag |= IXON | IXOFF;	break;
		case TMT_FNONE:	break;
	}
	trm.c_iflag |= IGNPAR;
	//trm.c_cflag |= (PARENB | CSTOPB);
	trm.c_oflag = 0;
	trm.c_oflag |= NLDLY;
	trm.c_lflag = 0;
	trm.c_cc[VTIME] = 0;
	trm.c_cc[VMIN] = 0;
	tcflush (fd, TCIFLUSH);
	tcsetattr (fd, TCSANOW, &trm);
	return fd;
}

int closetty(int fd)
{
	tcsetattr (fd, TCSANOW, &oldtrm);
	if(close(fd) < 0) {
		perror("close");
		return -1;
	} 
	return 0;
}

int send(int fd, char *msg, int len)
{
	if(len==0) len = strlen(msg);
	if(write(fd, msg, len) == -1) {
		printf("Error to write port at Meter\n");
		perror("write");
		return -1;
	}
	return 0;
}

int rxsettings(char *buf, int length)
{
    int fd, rtn, cnt, i;
	char value[256];
	
	rtn = 0; cnt = 0;	
    fd = opentty();
    if(fd < 0) {
		printf("Error: cannot open tty(rx)\n");
		return -1;
    }
    send(fd, buf, length);
	tcdrain(fd);
	memset(value, 0x00, sizeof value);
	usleep(150000);
    do {
		rtn = read(fd, value, sizeof value);
		if(rtn == -1) perror("read"); 
		if(value[rtn - 1] == '\n') {
			rtn = 0;
			break;// ricevuto carattere End Of Line
		}
		cnt++;
		if(cnt >= 5) {
			rtn = -2;
			printf("Read Error : tty(rx)\n");
			for(i=0; i < 256; i++)
				printf("%x ", value[i]);
			printf("\n");
			break;
		}
		printf("read count %d\n", cnt);
		usleep(50000);
    }
    while(1);
    closetty(fd);
    return rtn;
}

int txsettings(char *buf, int length)
{
	int fd;
	fd = opentty();
	if(fd < 0) {
		printf("Error: cannot open tty\n");
		return -1;
	}
	send(fd, buf, length);
	tcdrain(fd);
	usleep(50000);
	closetty(fd);
	return 0;
}

void delay(int sec)
{
	time_t t = time(NULL);
	while(time(NULL) - t < sec) {}
}
