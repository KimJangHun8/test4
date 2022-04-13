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
			}
		} else {
	    	userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
			return -10;
		}
	}else if(strcmp((char *)&p_argv[index][0], "update") == 0
		|| strcmp((char *)&p_argv[index][0], "UPDATE") == 0) {
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
		} else {
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
		} else {
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
