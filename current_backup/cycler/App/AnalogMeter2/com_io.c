/*
 * Read "n" bytes from a descriptor.
 * Use in place of read() when fd is a stream socket.
 */
#include <errno.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include "com_io.h"

#define	TIMEOUT_READ	10
jmp_buf	read_jmp;

void signal4read(int signum)
{
    printf("Catch Signal [%d] for reading...\n", signum);
    longjmp(read_jmp, signum);
}

int readn(int fd, char *ptr, int nbytes)
{
    return readnt(fd, ptr, nbytes, TIMEOUT_READ);
}

int readnt(int fd, char *ptr, int nbytes, int timeout)
{
    static int nleft=0, nread=0;
    int	signum;

    if((signum = setjmp(read_jmp)) != 0) {
		printf("Catch Signal to escape function readn "
			"(%d)byte(s) wanted, (%d)bytes left\n", nbytes, nleft);
		errno = -99;
		return	-99;
    }
    signal(SIGALRM, signal4read);
    alarm(timeout);	/*** waiting seconds for limit ***/
    nleft=0; nread=0;
    nleft = nbytes;
    while(nleft > 0) {
		nread = read(fd, ptr, nleft);
		if(nread < 0) break;
		else if(nread == 0) break;
		nleft -= nread;
		ptr += nread;
    }
    alarm(0);
    return (nbytes - nleft);
}

/*
 * Write "n" bytes to a descriptor.
 * Use in place of write() when fd is a stream socket.
 */
int writen(int fd, char *ptr, int nbytes)
{
    int	nleft, nwritten;

    nleft = nbytes;
    while(nleft > 0) {
		nwritten = write(fd, ptr, nleft);
		if(nwritten <= 0) return (nwritten);
		nleft -= nwritten;
		ptr += nwritten;
    }
    return (nbytes - nleft);
}
