#ifndef _COMM_H_
#define _COMM_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

typedef enum {
	TMT_FRTSCTS,
	TMT_FXONXOFF,
	TMT_FNONE
} Flow;

typedef enum {
	TMT_B3,
	TMT_B6,
	TMT_B12,
	TMT_B24,
	TMT_B48,
	TMT_B96,
	TMT_B192
} Bps;

typedef char   gchar;

typedef struct {
	gchar device[11];
	Bps   bps;
	Flow  flow;
} CommParam;

int opentty (void);
int closetty (int);
int send (int, char *, int);
int rxsettings (char *);
int txsettings (char *);
#endif
