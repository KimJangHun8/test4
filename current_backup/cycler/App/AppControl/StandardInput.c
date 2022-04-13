#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "message.h"
#include "StandardInput.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_APP_CONTROL *myPs;
extern char psName[16];
char p_argv[8][32];
char *in_delimiter = " \t\n\r";

void StandardInput_Receive(void)
{
    char msg[256], *tmp;
    int	rtn, index;

	memset((char *)&p_argv[0][0], 0x00, (32 * 8));

	memset(msg, 0x00, sizeof(msg));
    tmp = fgets(msg, sizeof(msg), stdin);
    if(tmp == NULL) {
//	    userlog(DEBUG_LOG, psName, "error occurred reading from stdin\n");
		return;
    }

    rtn = Parsing_StandardInput(msg);
    if(rtn < 0) return;
	
	if(rtn >= 2) {
		index = 0;
	} else {
    	userlog(DEBUG_LOG, psName, "command > ", rtn);
		if(WaitKeyInput() < 0) return;

		memset(msg, 0x00, sizeof(msg));
    	tmp = fgets(msg, sizeof(msg), stdin);
	    if(tmp == NULL) {
//		    userlog(DEBUG_LOG, psName, "error occurred reading from stdin\n");
			return;
	    }
	
		rtn = Parsing_StandardInput(msg);
		if(rtn < 0) return;
		index = 0;
	}
	UserCommand_Parsing(rtn, index);
}

int WaitKeyInput(void)
{
	int 	rtn, count=0;
	struct	timeval	tv;
	fd_set	rfds;
	
	while(1) {
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		
		rtn = select(1, &rfds, NULL, NULL, &tv);
		if(rtn != 0) {
			break;
		} else {
			count++;
			if(count >= 20) {
				userlog(DEBUG_LOG, psName, "StandardInput timeover!!!\n");
				return -1; //20sec
			}
		}
	}
	return 0;
}

int Parsing_StandardInput(char *buf)
{
    char *token, tmpbuf[256];
    int	p_argc = 0;

    memset((char *)&p_argv[0][0], 0x00, 8*32);
	memset((char *)&tmpbuf[0], 0x00, sizeof(tmpbuf));

    strcpy((char *)&tmpbuf[0], buf);

    token = strtok(tmpbuf, in_delimiter);
    if(token == NULL) {
		*buf = 0;
        userlog(DEBUG_LOG, psName, "there are nothing to get\n");
		return -1;
    }
	
	while(1) {
	   	strncpy((char *)&p_argv[p_argc][0], token, 32);
	   	userlog(DEBUG_LOG, psName, "token is %s \n", p_argv[p_argc]);
		
	    p_argc++;
	   	token = strtok(NULL, in_delimiter);
		if(token) {
			if(p_argc >= 8) {
				userlog(DEBUG_LOG, psName, "Error!!! : p_argc over\n");
				return -2;
			}
		} else break;
	}
    return p_argc;
}

void UserCommand_Parsing(int p_argc, int index)
{
	int rtn;

    if(strcmp((char *)&p_argv[index][0], "sys") == 0
 		|| strcmp((char *)&p_argv[index][0], "SYS") == 0) {
		index++;
        rtn = SysProcessing(p_argc, index);
	} else if(strcmp((char *)&p_argv[index][0], "exec") == 0
		|| strcmp((char *)&p_argv[index][0], "EXEC") == 0) {
		index++;
        rtn = ExecProcessing(p_argc, index);
	} else if(strcmp((char *)&p_argv[index][0], "cali") == 0
		|| strcmp((char *)&p_argv[index][0], "CALI") == 0) {
		index++;
        rtn = CaliProcessing(p_argc, index);
	} else if(strcmp((char *)&p_argv[index][0], "analog") == 0
		|| strcmp((char *)&p_argv[index][0], "ANALOG") == 0) {
		index++;
        rtn = AnalogProcessing(p_argc, index);
	} else if(strcmp((char *)&p_argv[index][0], "analog2") == 0
		|| strcmp((char *)&p_argv[index][0], "ANALOG2") == 0) {
		index++;
        rtn = AnalogProcessing2(p_argc, index);
	} else if(strcmp((char *)&p_argv[index][0], "fadm") == 0
		|| strcmp((char *)&p_argv[index][0], "FADM") == 0) {
		index++;
        rtn = FADMProcessing(p_argc, index);
	} else {
		userlog(DEBUG_LOG, psName, "unknown command : %s\n", p_argv[index]);
    }
}

int SysProcessing(int p_argc, int index)
{
	int rtn = 0;

    if(strcmp((char *)&p_argv[index][0], "quit") == 0
		|| strcmp((char *)&p_argv[index][0], "QUIT") == 0) {
		send_msg(APP_TO_MODULE, MSG_APP_MODULE_EXIT, 0, 0);
	} else if(strcmp((char *)&p_argv[index][0], "halt") == 0
		|| strcmp((char *)&p_argv[index][0], "HALT") == 0) {
		send_msg(APP_TO_MODULE, MSG_APP_MODULE_EXIT, 1, 0);
	} else {
	    userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
		rtn = -1;
    }
    return rtn;
}

int ExecProcessing(int p_argc, int index)
{
	char buf[12];
	int p_ch;
	
    if(strcmp((char *)&p_argv[index][0], "saveData") == 0) {
		memset(buf, 0x00, sizeof buf);
		memcpy((char *)&buf[0], (char *)&p_argv[index+1][0], sizeof buf);
		if(strcmp((char *)&buf[0], "start") == 0) {
			memset(buf, 0x00, sizeof buf);
			memcpy((char *)&buf[0], (char *)&p_argv[index+2][0], sizeof buf);
			p_ch = atoi(buf) - 1;
			if(p_ch >= 0 || p_ch < myData->mData.config.installedCh) {
				send_msg(APP_TO_MODULE, MSG_APP_MODULE_SAVE_START, p_ch, 0);
			}
		} else if(strcmp((char *)&buf[0], "stop") == 0) {
			memset(buf, 0x00, sizeof buf);
			memcpy((char *)&buf[0], (char *)&p_argv[index+2][0], sizeof buf);
			p_ch = atoi(buf) - 1;
			if(p_ch >= 0 || p_ch < myData->mData.config.installedCh) {
				send_msg(APP_TO_MODULE, MSG_APP_MODULE_SAVE_STOP, p_ch, 0);
			}
		}
	} else if(strcmp((char *)&p_argv[index][0], "AUXLOW") == 0
		|| strcmp((char *)&p_argv[index][0], "auxlow") == 0) {
		index++;
		send_msg(APP_TO_MODULE,MSG_APP_MODULE_AUX_CALI_LOW,atoi(p_argv[index]),
						atoi(p_argv[index+1]));
	} else if(strcmp((char *)&p_argv[index][0], "AUXHIGH") == 0
		|| strcmp((char *)&p_argv[index][0], "auxhigh") == 0) {
		index++;
		send_msg(APP_TO_MODULE,MSG_APP_MODULE_AUX_CALI_HIGH,atoi(p_argv[index]),
						atoi(p_argv[index+1]));
	} else if(strcmp((char *)&p_argv[index][0], "AUXUPDATE") == 0
		|| strcmp((char *)&p_argv[index][0], "auxupdate") == 0) {
		index++;
		send_msg(APP_TO_MODULE,MSG_APP_MODULE_AUX_CALI_UPDATE,0,0);
	} else {
	    userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
		return -10;
    }

	return 0;
}

int CaliProcessing(int p_argc, int index)
{
	int rtn;
	
    if(strcmp((char *)&p_argv[index][0], "connect") == 0
		|| strcmp((char *)&p_argv[index][0], "CONNECT") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "v") == 0
			|| strcmp((char *)&p_argv[index][0], "V") == 0) {
			send_msg(APP_TO_METER, MSG_APP_METER_INITIALIZE, 0, 0);
		} else if(strcmp((char *)&p_argv[index][0], "i") == 0
			|| strcmp((char *)&p_argv[index][0], "I") == 0) {
			send_msg(APP_TO_METER, MSG_APP_METER_INITIALIZE, 0, 1);
		}
	} else if(strcmp((char *)&p_argv[index][0], "start") == 0
		|| strcmp((char *)&p_argv[index][0], "START") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "v") == 0
			|| strcmp((char *)&p_argv[index][0], "V") == 0) {
			myData->cali.tmpCond[0][0].type = 0;
			myData->cali.tmpCond[0][0].range = 0;
			myData->cali.tmpCond[0][0].mode = CALI_MODE_NORMAL;
			send_msg(APP_TO_MODULE, MSG_APP_MODULE_CH_CALI, 0, 0);
		} else if(strcmp((char *)&p_argv[index][0], "i") == 0
			|| strcmp((char *)&p_argv[index][0], "I") == 0) {
			myData->cali.tmpCond[0][0].type = 1;
			myData->cali.tmpCond[0][0].range = 0;
			myData->cali.tmpCond[0][0].mode = CALI_MODE_NORMAL;
			send_msg(APP_TO_MODULE, MSG_APP_MODULE_CH_CALI, 0, 1);
		}
	} else if(strcmp((char *)&p_argv[index][0], "read") == 0
		|| strcmp((char *)&p_argv[index][0], "READ") == 0) {
		send_msg(APP_TO_METER, MSG_APP_METER_REQUEST, 0, 0);
	} else if(strcmp((char *)&p_argv[index][0], "org") == 0
		|| strcmp((char *)&p_argv[index][0], "ORG") == 0) {
		rtn = Create_BdCaliData_Org(0);
	} else if(strcmp((char *)&p_argv[index][0], "d_update") == 0){
		send_msg(APP_TO_MODULE, MSG_APP_MODULE_D_CALI_UPDATE, 0,0);	
	} else {
	    userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
		return -10;
    }

	return 0;
}

int AnalogProcessing(int p_argc, int index)
{
    if(strcmp((char *)&p_argv[index][0], "connect") == 0
		|| strcmp((char *)&p_argv[index][0], "CONNECT") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			index++;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 1, 1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 1, 2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 1, 3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 1, 4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 1, 5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 1, 6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 1, 7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 1, 8);
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 1, 9);
			} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 1, 10);
			} else if(strcmp((char *)&p_argv[index][0], "31") == 0) { //ambientTemp
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 1, 31);
			} else if(strcmp((char *)&p_argv[index][0], "41") == 0) {	//gasVoltage
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 1, 41);
			}
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			index++;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 2, 1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 2, 2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 2, 3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 2, 4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 2, 5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 2, 6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 2, 7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 2, 8);
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 2, 9);
			} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 2, 10);
			} else if(strcmp((char *)&p_argv[index][0], "31") == 0) { //ambientTemp
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 2, 31);
			} else if(strcmp((char *)&p_argv[index][0], "41") == 0) {	//gasVoltage
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 2, 41);
			}
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			index++;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 3, 1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 3, 2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 3, 3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 3, 4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 3, 5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 3, 6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 3, 7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 3, 8);
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 3, 9);
			} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 3, 10);
			} else if(strcmp((char *)&p_argv[index][0], "31") == 0) { //ambientTemp
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 3, 31);
			} else if(strcmp((char *)&p_argv[index][0], "41") == 0) {	//gasVoltage
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 3, 41);
			}
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			index++;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 4, 1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 4, 2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 4, 3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 4, 4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 4, 5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 4, 6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 4, 7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 4, 8);
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 4, 9);
			} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 4, 10);
			} else if(strcmp((char *)&p_argv[index][0], "31") == 0) { //ambientTemp
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 4, 31);
			} else if(strcmp((char *)&p_argv[index][0], "41") == 0) {	//gasVoltage
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 4, 41);
			}
		} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
			index++;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 5, 1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 5, 2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 5, 3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 5, 4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 5, 5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 5, 6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 5, 7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 5, 8);
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 5, 9);
			} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 5, 10);
			} else if(strcmp((char *)&p_argv[index][0], "31") == 0) { //ambientTemp
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 5, 31);
			} else if(strcmp((char *)&p_argv[index][0], "41") == 0) {	//gasVoltage
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 5, 41);
			}
		} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
			index++;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 6, 1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 6, 2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 6, 3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 6, 4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 6, 5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 6, 6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 6, 7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 6, 8);
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 6, 9);
			} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 6, 10);
			} else if(strcmp((char *)&p_argv[index][0], "31") == 0) { //ambientTemp
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 6, 31);
			} else if(strcmp((char *)&p_argv[index][0], "41") == 0) {	//gasVoltage
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 6, 41);
			}
		} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
			index++;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 7, 1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 7, 2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 7, 3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 7, 4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 7, 5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 7, 6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 7, 7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 7, 8);
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 7, 9);
			} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 7, 10);
			} else if(strcmp((char *)&p_argv[index][0], "31") == 0) { //ambientTemp
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 7, 31);
			} else if(strcmp((char *)&p_argv[index][0], "41") == 0) {	//gasVoltage
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 7, 41);
			}
		} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
			index++;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 8, 1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 8, 2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 8, 3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 8, 4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 8, 5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 8, 6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 8, 7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 8, 8);
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 8, 9);
			} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 8, 10);
			} else if(strcmp((char *)&p_argv[index][0], "31") == 0) { //ambientTemp
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 8, 31);
			} else if(strcmp((char *)&p_argv[index][0], "41") == 0) {	//gasVoltage
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 8, 41);
			}
		} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
			index++;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 9, 1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 9, 2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 9, 3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 9, 4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 9, 5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 9, 6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 9, 7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 9, 8);
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 9, 9);
			} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 9, 10);
			} else if(strcmp((char *)&p_argv[index][0], "31") == 0) { //ambientTemp
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 9, 31);
			} else if(strcmp((char *)&p_argv[index][0], "41") == 0) {	//gasVoltage
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 9, 41);
			}
		} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
			index++;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 10, 1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 10, 2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 10, 3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 10, 4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 10, 5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 10, 6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 10, 7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 10, 8);
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 10, 9);
			} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 10, 10);
			} else if(strcmp((char *)&p_argv[index][0], "31") == 0) { //ambientTemp
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 10, 31);
			} else if(strcmp((char *)&p_argv[index][0], "32") == 0) { 
				//ambientTemp	hun_211019
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 10, 32);
			} else if(strcmp((char *)&p_argv[index][0], "41") == 0) {	//gasVoltage
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 10, 41);
			} else if(strcmp((char *)&p_argv[index][0], "42") == 0) {
				//gasVoltage	hun_211019
				send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 10, 42);
			}
		} else {
	    	userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
			return -10;
		}
	}else if(strcmp((char *)&p_argv[index][0], "update") == 0
		|| strcmp((char *)&p_argv[index][0], "UPDATE") == 0) {
		send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_UPDATE,1,0);
		return -10;
	}else if(strcmp((char *)&p_argv[index][0], "update2") == 0
		|| strcmp((char *)&p_argv[index][0], "UPDATE2") == 0) {
		send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_UPDATE,1,0);
		return -10;
	}else if(strcmp((char *)&p_argv[index][0], "calc") == 0
		|| strcmp((char *)&p_argv[index][0], "CALC") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_WRITE,1,0);
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_WRITE,2,0);
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_WRITE,3,0);
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_WRITE,4,0);
		} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_WRITE,5,0);
		} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_WRITE,6,0);
		} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_WRITE,7,0);
		} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_WRITE,8,0);
		} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_WRITE,9,0);
		} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_WRITE,10,0);
		} else if(strcmp((char *)&p_argv[index][0], "31") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_WRITE,31,0);
		} else if(strcmp((char *)&p_argv[index][0], "32") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_WRITE,32,0);//hun_211019
		} else if(strcmp((char *)&p_argv[index][0], "41") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_WRITE,41,0);
		} else if(strcmp((char *)&p_argv[index][0], "42") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_WRITE,42,0);//hun_211019
		}else {
	    	userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
			return -10;
		}
	}else if(strcmp((char *)&p_argv[index][0], "low") == 0
		|| strcmp((char *)&p_argv[index][0], "LOW") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_L,1,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_L,2,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_L,3,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_L,4,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_L,5,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_L,6,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_L,7,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_L,8,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_L,9,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_L,10,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "31") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_L,31,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "32") == 0) {
			index++;
			//hun_211019
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_L,32,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "41") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_L,41,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "42") == 0) {
			index++;
			//hun_211019
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_L,42,atoi(p_argv[index]));
		} else {
	    	userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
			return -10;
		}
	}else if(strcmp((char *)&p_argv[index][0], "high") == 0
		|| strcmp((char *)&p_argv[index][0], "HIGH") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_H,1,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_H,2,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_H,3,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_H,4,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_H,5,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_H,6,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_H,7,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_H,8,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_H,9,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_H,10,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "31") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_H,31,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "32") == 0) {
			index++;
			//hun_211019
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_H,32,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "41") == 0) {
			index++;
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_H,41,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "42") == 0) {
			index++;
			//hun_211019
			send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_H,42,atoi(p_argv[index]));
		}  else {
	    	userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
			return -10;
		}
	} else if(strcmp((char *)&p_argv[index][0], "read") == 0
		|| strcmp((char *)&p_argv[index][0], "READ") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 0, 1);
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 0, 2);
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 0, 3);
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 0, 4);
		} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 0, 5);
		} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 0, 6);
		} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 0, 7);
		} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 0, 8);
		} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 0, 9);
		} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 0, 10);
		}
	} else if(strcmp((char *)&p_argv[index][0], "read2") == 0
		|| strcmp((char *)&p_argv[index][0], "READ2") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 1, 1);
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 1, 2);
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 1, 3);
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 1, 4);
		} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 1, 5);
		} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 1, 6);
		} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 1, 7);
		} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 1, 8);
		} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 1, 9);
		} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 1, 10);
		}
	} else if(strcmp((char *)&p_argv[index][0], "start") == 0
		|| strcmp((char *)&p_argv[index][0], "START") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 0, 1);
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 0, 2);
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 0, 3);
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 0, 4);
		} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 0, 5);
		} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 0, 6);
		} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 0, 7);
		} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 0, 8);
		} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 0, 9);
		} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 0, 10);
		}
	} else if(strcmp((char *)&p_argv[index][0], "stop") == 0
		|| strcmp((char *)&p_argv[index][0], "STOP") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 1, 1);
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 1, 2);
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 1, 3);
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 1, 4);
		} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 1, 5);
		} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 1, 6);
		} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 1, 7);
		} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 1, 8);
		} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 1, 9);
		} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
			send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 1, 10);
		}
	} else if(strcmp((char *)&p_argv[index][0], "auto") == 0
		|| strcmp((char *)&p_argv[index][0], "AUTO") == 0) {
		index++;
		if(myData->AnalogMeter.misc.auto_cali_flag == P0){
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_AUTO,41,1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_AUTO,41,2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_AUTO,41,3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_AUTO,41,4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_AUTO,41,5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_AUTO,41,6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_AUTO,41,7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_CALI_AUTO,41,8);
			}
		}
	} else {
	    userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
		return -11;
    }

	return 0;
}

int AnalogProcessing2(int p_argc, int index)
{
    if(strcmp((char *)&p_argv[index][0], "connect") == 0
		|| strcmp((char *)&p_argv[index][0], "CONNECT") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			index++;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 1, 1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 1, 2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 1, 3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 1, 4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 1, 5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 1, 6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 1, 7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 1, 8);
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 1, 9);
			} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 1, 10);
			}
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			index++;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 2, 1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 2, 2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 2, 3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 2, 4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 2, 5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 2, 6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 2, 7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 2, 8);
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 2, 9);
			} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 2, 10);
			}
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			index++;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 3, 1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 3, 2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 3, 3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 3, 4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 3, 5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 3, 6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 3, 7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 3, 8);
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 3, 9);
			} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 3, 10);
			}
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			index++;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 4, 1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 4, 2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 4, 3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 4, 4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 4, 5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 4, 6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 4, 7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 4, 8);
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 4, 9);
			} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 4, 10);
			}
		} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
			index++;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 5, 1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 5, 2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 5, 3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 5, 4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 5, 5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 5, 6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 5, 7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 5, 8);
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 5, 9);
			} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 5, 10);
			}
		} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
			index++;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 6, 1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 6, 2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 6, 3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 6, 4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 6, 5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 6, 6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 6, 7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 6, 8);
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 6, 9);
			} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 6, 10);
			}
		} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
			index++;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 7, 1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 7, 2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 7, 3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 7, 4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 7, 5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 7, 6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 7, 7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 7, 8);
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 7, 9);
			} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 7, 10);
			}
		} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
			index++;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 8, 1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 8, 2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 8, 3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 8, 4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 8, 5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 8, 6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 8, 7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 8, 8);
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 8, 9);
			} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 8, 10);
			}
		} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
			index++;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 9, 1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 9, 2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 9, 3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 9, 4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 9, 5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 9, 6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 9, 7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 9, 8);
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 9, 9);
			} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 9, 10);
			}
		} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
			index++;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 10, 1);
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 10, 2);
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 10, 3);
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 10, 4);
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 10, 5);
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 10, 6);
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 10, 7);
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 10, 8);
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 10, 9);
			} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
				send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_INITIALIZE, 10, 10);
			}
		} else {
	    	userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
			return -10;
		}
	}else if(strcmp((char *)&p_argv[index][0], "update") == 0
		|| strcmp((char *)&p_argv[index][0], "UPDATE") == 0) {
		send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_UPDATE,1,0);
		return -10;
	}else if(strcmp((char *)&p_argv[index][0], "calc") == 0
		|| strcmp((char *)&p_argv[index][0], "CALC") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_WRITE,1,0);
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_WRITE,2,0);
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_WRITE,3,0);
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_WRITE,4,0);
		} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_WRITE,5,0);
		} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_WRITE,6,0);
		} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_WRITE,7,0);
		} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_WRITE,8,0);
		} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_WRITE,9,0);
		} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_WRITE,10,0);
		}else {
	    	userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
			return -10;
		}
	}else if(strcmp((char *)&p_argv[index][0], "low") == 0
		|| strcmp((char *)&p_argv[index][0], "LOW") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			index++;
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_L,1,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			index++;
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_L,2,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			index++;
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_L,3,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			index++;
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_L,4,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
			index++;
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_L,5,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
			index++;
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_L,6,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
			index++;
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_L,7,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
			index++;
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_L,8,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
			index++;
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_L,9,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
			index++;
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_L,10,atoi(p_argv[index]));
		} else {
	    	userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
			return -10;
		}
	}else if(strcmp((char *)&p_argv[index][0], "high") == 0
		|| strcmp((char *)&p_argv[index][0], "HIGH") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			index++;
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_H,1,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			index++;
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_H,2,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			index++;
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_H,3,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			index++;
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_H,4,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
			index++;
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_H,5,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
			index++;
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_H,6,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
			index++;
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_H,7,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
			index++;
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_H,8,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
			index++;
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_H,9,atoi(p_argv[index]));
		} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
			index++;
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_CALI_H,10,atoi(p_argv[index]));
		}  else {
	    	userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
			return -10;
		}
	} else if(strcmp((char *)&p_argv[index][0], "read") == 0
		|| strcmp((char *)&p_argv[index][0], "READ") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_REQUEST, 0, 1);
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_REQUEST, 0, 2);
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_REQUEST, 0, 3);
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_REQUEST, 0, 4);
		} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_REQUEST, 0, 5);
		} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_REQUEST, 0, 6);
		} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_REQUEST, 0, 7);
		} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_REQUEST, 0, 8);
		} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_REQUEST, 0, 9);
		} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_REQUEST, 0, 10);
		}
	} else if(strcmp((char *)&p_argv[index][0], "read2") == 0
		|| strcmp((char *)&p_argv[index][0], "READ2") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_REQUEST, 1, 1);
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_REQUEST, 1, 2);
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_REQUEST, 1, 3);
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_REQUEST, 1, 4);
		} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_REQUEST, 1, 5);
		} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_REQUEST, 1, 6);
		} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_REQUEST, 1, 7);
		} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_REQUEST, 1, 8);
		} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_REQUEST, 1, 9);
		} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_REQUEST, 1, 10);
		}
	} else if(strcmp((char *)&p_argv[index][0], "start") == 0
		|| strcmp((char *)&p_argv[index][0], "START") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_MEASURE, 0, 1);
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_MEASURE, 0, 2);
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_MEASURE, 0, 3);
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_MEASURE, 0, 4);
		} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_MEASURE, 0, 5);
		} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_MEASURE, 0, 6);
		} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_MEASURE, 0, 7);
		} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_MEASURE, 0, 8);
		} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_MEASURE, 0, 9);
		} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_MEASURE, 0, 10);
		}
	} else if(strcmp((char *)&p_argv[index][0], "stop") == 0
		|| strcmp((char *)&p_argv[index][0], "STOP") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_MEASURE, 1, 1);
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_MEASURE, 1, 2);
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_MEASURE, 1, 3);
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_MEASURE, 1, 4);
		} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_MEASURE, 1, 5);
		} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_MEASURE, 1, 6);
		} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_MEASURE, 1, 7);
		} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_MEASURE, 1, 8);
		} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_MEASURE, 1, 9);
		} else if(strcmp((char *)&p_argv[index][0], "10") == 0) {
			send_msg(APP_TO_ANALOGMETER2, MSG_APP_METER2_MEASURE, 1, 10);
		}
	} else {
	    userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
		return -11;
    }

	return 0;
}

int FADMProcessing(int p_argc, int index)
{
	char buf[12];
	int val1, val2, i;
	unsigned long chFlag;
	S_MSG_CH_FLAG ch_flag;

	if(strcmp((char *)&p_argv[index][0], "measure") == 0
		|| strcmp((char *)&p_argv[index][0], "MEASURE") == 0) {
		index++;
		memset(buf, 0, sizeof buf);
		memcpy((char *)&buf[0], (char *)&p_argv[index][0], sizeof buf);
		val1 = atoi(buf);

		chFlag = 0x00000001;
		memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
		for(i=0; i < myData->mData.config.installedCh; i++) {
			if(i < 32) {
				if(val1 == i) {
					ch_flag.bit_32[0] = (chFlag << i);
					break;
				}
			} else {
				if(val1 == i) {
					ch_flag.bit_32[1] = (chFlag << i);
					break;
				}
			}
		}
		send_msg_ch_flag(APP_TO_METER2, (char *)&ch_flag);
		send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 0, 0);
	} else if(strcmp((char *)&p_argv[index][0], "read") == 0
		|| strcmp((char *)&p_argv[index][0], "READ") == 0) {
		index++;
		memset(buf, 0, sizeof buf);
		memcpy((char *)&buf[0], (char *)&p_argv[index][0], sizeof buf);
		val1 = atoi(buf);

		index++;
		memset(buf, 0, sizeof buf);
		memcpy((char *)&buf[0], (char *)&p_argv[index][0], sizeof buf);
		val2 = atoi(buf);

		send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, val1, val2);
	} else if(strcmp((char *)&p_argv[index][0], "comm_check") == 0
		|| strcmp((char *)&p_argv[index][0], "COMM_CHECK") == 0) {
		index++;
		memset(buf, 0, sizeof buf);
		memcpy((char *)&buf[0], (char *)&p_argv[index][0], sizeof buf);
		val1 = atoi(buf);

		index++;
		memset(buf, 0, sizeof buf);
		memcpy((char *)&buf[0], (char *)&p_argv[index][0], sizeof buf);
		val2 = atoi(buf);

		send_msg(APP_TO_METER2, MSG_APP_METER2_COMM_CHECK, val1, val2);
	} else if(strcmp((char *)&p_argv[index][0], "wr_base_addr") == 0
		|| strcmp((char *)&p_argv[index][0], "WR_BASE_ADDR") == 0) {
		index++;
		memset(buf, 0, sizeof buf);
		memcpy((char *)&buf[0], (char *)&p_argv[index][0], sizeof buf);
		val1 = atoi(buf);

		index++;
		memset(buf, 0, sizeof buf);
		memcpy((char *)&buf[0], (char *)&p_argv[index][0], sizeof buf);
		val2 = atoi(buf);

		send_msg(APP_TO_METER2, MSG_APP_METER2_WR_BASE_ADDR, val1, val2);
	} else {
	    userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
		return -10;
    }

	return 0;
}
