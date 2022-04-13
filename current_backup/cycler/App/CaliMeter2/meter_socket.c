#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <bits/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include "../../INC/datastore.h"
#include "meter_socket.h"

volatile S_SYSTEM_DATA	*myData;
volatile S_CALI_METER	*myPs;
char	psName[PROCESS_NAME_SIZE];

int SetMeterSock(int port, char *addr)
{
	struct sockaddr_in meter_addr;
	int meterSD, meterFD;
	
	//Create Socket
	meterSD = socket(PF_INET, SOCK_STREAM, 0);
	if(meterSD < 0){
		printf("Error : Unable to create socket %i\n", errno);
	   	return -errno;
	}else{
		printf("Create Socket complete : [%d] \n", meterSD);
	}

	memset((char *)&meter_addr, 0x00, sizeof(struct sockaddr_in));
	meter_addr.sin_family = PF_INET;					//IPv4
	meter_addr.sin_port = htons(port);					//Port num
	meter_addr.sin_addr.s_addr = inet_addr(addr);	//CaliMeterIP
	
	//connect
	meterFD = connect(meterSD, (struct sockaddr *)&meter_addr,
		sizeof(struct sockaddr_in));
	if(meterFD < 0){
		printf("Error : Unable to Connection socket [%d:%s] : %i\n", port, addr, errno);
		myPs->signal[CALI_METER_SIG_LAN_CONNECT] = P0;
		return -errno;
	}else{
		myPs->signal[CALI_METER_SIG_LAN_USE] = P1;
		myPs->signal[CALI_METER_SIG_LAN_CONNECT] = P2;

		printf("Connect To Meter complete...\n");
	}
	return meterSD;
}
