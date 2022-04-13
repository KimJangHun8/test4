#ifndef __COMM_H__
#define __COMM_H__

#define COM_DEVICE_SIZE	16

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
	TMT_B192,
	TMT_B384
} Bps;

typedef struct {
	char device[COM_DEVICE_SIZE];
	Bps   bps;
	Flow  flow;
} CommParam;

int opentty(void);
int closetty(int);
int send(int, char *, int);
int rxsettings(char*, int);
int txsettings(char*, int);
void delay(int);
#endif
