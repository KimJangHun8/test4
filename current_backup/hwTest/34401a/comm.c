
#include <sys/time.h>
#include "comm.h"

//////////// Agilent 34401A Multimeter /////////////
extern CommParam comm_param; 
extern char value[256];
static struct termios oldtrm;

int opentty(void)
{
    int fd;
    struct termios trm;
	
    fd = open(comm_param.device, O_RDWR | O_NOCTTY);
    if(fd < 0) {
		perror("open");
		return -1;
    }
    tcgetattr(fd, &oldtrm);	// salva il setup attuale della porta
    bzero(&trm, sizeof(trm));
    switch(comm_param.bps) {
		case TMT_B3:
			trm.c_cflag = B300 | CS8 | CREAD; break;
		case TMT_B6:
			trm.c_cflag = B600 | CS8 | CREAD; break;
		case TMT_B12:
			trm.c_cflag = B1200 | CS8 | CREAD; break;
		case TMT_B24:
			trm.c_cflag = B2400 | CS8 | CREAD; break;
		case TMT_B48:
			trm.c_cflag = B4800 | CS8 | CREAD; break;
		case TMT_B96:
			trm.c_cflag = B9600 | CS7 | CREAD | CSTOPB | PARENB; break;
		case TMT_B192:
			trm.c_cflag = B19200 | CS8 | CREAD; break;
    }
    switch(comm_param.flow) {
		case TMT_FRTSCTS:
			trm.c_cflag |= CRTSCTS; break;
		case TMT_FXONXOFF:
			trm.c_iflag |= IXON | IXOFF; break;
		case TMT_FNONE:
			break;
    }
    trm.c_iflag |= IGNPAR;
    //trm.c_iflag |= INPCK;
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
    if(!len) len = strlen(msg);
    if(write(fd, msg, len) == -1) {
		perror("write");
		return -1;
    }
    return 0;
}

int rxsettings(char *buf)
{
    int fd, rtn, cnt=0;
	
    fd = opentty();
    if(fd < 0) {
		printf("Error: cannot open tty\n");
		return -1;
    }
    send(fd, buf, 0);
    tcdrain(fd);
	usleep(150000);
	memset(value, 0x00, sizeof value);
	do {
		rtn = read(fd, value, sizeof(value));
		//printf("kjg %s %d\n", value, rtn);
		if(rtn == -1) perror("read"); 
	    if(value[rtn - 1] == '\n') {
			rtn = 0;
			break;// ricevuto carattere End Of Line 
		}
		cnt++;
	    if(cnt >= 10) {
			rtn = -2;
			printf("comm fail\n");
			break;
		}
		usleep(50000);
	} while(1);
    closetty (fd);
    return rtn;
}

int txsettings(char *buf)
{
    int fd;
    fd = opentty();
    if(fd < 0) {
		printf("Error: cannot open tty\n");
		return -1;
    }
    send(fd, buf, 0);
    tcdrain(fd);
	usleep(50000);
    closetty(fd);
    return 0;
}

/*
//////////// PREMA 5017 Digital Multimeter /////////////
extern CommParam comm_param; 
extern char value[50];
static struct termios oldtrm;

int 
opentty ()
{
    int fd;
    struct termios trm;
	
    fd = open (comm_param.device, O_RDWR | O_NOCTTY);
    if(fd<0) {
	perror("open");
	return -1;
    }
    tcgetattr (fd, &oldtrm);	// salva il setup attuale della porta
    bzero (&trm, sizeof (trm));
    switch(comm_param.bps) {
	case TMT_B3:
		trm.c_cflag = B300 | CS8 | CREAD; break;
	case TMT_B6:
		trm.c_cflag = B600 | CS8 | CREAD; break;
	case TMT_B12:
		trm.c_cflag = B1200 | CS8 | CREAD; break;
	case TMT_B24:
		trm.c_cflag = B2400 | CS8 | CREAD; break;
	case TMT_B48:
		trm.c_cflag = B4800 | CS8 | CREAD; break;
	case TMT_B96:
		trm.c_cflag = B9600 | CS8 | CREAD; break;
	case TMT_B192:
		trm.c_cflag = B19200 | CS8 | CREAD; break;
    }
    switch(comm_param.flow) {
	case TMT_FRTSCTS:
		trm.c_cflag |= CRTSCTS ; break;
	case TMT_FXONXOFF:
		trm.c_iflag |= IXON | IXOFF ; break;
	case TMT_FNONE:
		break;
    }
    trm.c_iflag |= IGNPAR;
    trm.c_oflag = 0;
    trm.c_oflag |= NLDLY;
    trm.c_lflag = 0;
    trm.c_cc[VTIME] = 0;
    trm.c_cc[VMIN] = 0;
    tcflush (fd, TCIFLUSH);
    tcsetattr (fd, TCSANOW, &trm);
    return fd;
}

int 
closetty (int fd)
{
    tcsetattr (fd, TCSANOW, &oldtrm);
    if(close(fd) < 0) {
	perror("close");
	return -1;
    } 
    return 0;
}

int 
send (int fd, char *msg, int len)
{
    if(!len) len = strlen(msg);
    if(write(fd, msg, len) == -1) {
	perror ("write");
	return -1;
    }
    return 0;
}

int
rxsettings (char* cmd)
{
    int fd, r, retval, i=0;
    struct timeval tv;
    fd_set rfds;
	
    fd = opentty();
    if(fd < 0) {
	printf("Error: cannot open tty\n");
	return -1;
    }
    send(fd, cmd, 0);
    //tcdrain(fd);
    //for(j=0; j<100; j++) {
    //    for(i=0; i<100000; i++) {}
    //}
    while(1) {
	i++;
    	tv.tv_sec = 1;
	tv.tv_usec = 0;
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);
	retval = select(0, &rfds, NULL, NULL, &tv);
	if(retval == 0) {
	    if(i>1) {printf("comm fail\n"); break;}
	    r = read(fd, value, sizeof value);
	    if(r == -1) perror("read"); 
	    if(value[r-1] == '\n') break;// ricevuto carattere End Of Line 
	}
    }
    closetty (fd);
    return 0;
}

int
txsettings (char* buf)
{
    int fd;

    fd = opentty();
    if(fd < 0) {
	printf("Error: cannot open tty\n");
	return -1;
    }
    send(fd, buf, 0);
    tcdrain(fd);
    //for(j=0; j<100; j++) {
    //    for(i=0; i<100000; i++) {}
    //}
    closetty (fd);
    return 0;
}
*/
