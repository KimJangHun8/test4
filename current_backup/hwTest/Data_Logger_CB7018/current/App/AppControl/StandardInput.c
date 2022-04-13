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
    tmp = fgets(msg, sizeof(msg), stdin); //wkjg
    if(tmp == NULL) {
	    userlog(DEBUG_LOG, psName, "error occurred reading from stdin\n");
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
		    userlog(DEBUG_LOG, psName, "error occurred reading from stdin\n");
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
	   	token = strtok(NULL, in_delimiter); //kjgw NULL
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
	} else if(strcmp((char *)&p_argv[index][0], "log") == 0
		|| strcmp((char *)&p_argv[index][0], "LOG") == 0) {
		index++;
        rtn = LogProcessing(p_argc, index);
	} else {
		userlog(DEBUG_LOG, psName, "unknown command : %s\n", p_argv[index]);
    }
}

int SysProcessing(int p_argc, int index)
{
	int rtn = 0;

    if(strcmp((char *)&p_argv[index][0], "quit") == 0
		|| strcmp((char *)&p_argv[index][0], "QUIT") == 0) {
		myPs->signal[APP_SIG_DATA_SAVE_PROCESS] = PHASE2;
		myPs->signal[APP_SIG_METER1_PROCESS] = PHASE2;
		myPs->signal[APP_SIG_METER2_PROCESS] = PHASE2;
		myPs->signal[APP_SIG_QUIT] = PHASE3;
	} else if(strcmp((char *)&p_argv[index][0], "halt") == 0
		|| strcmp((char *)&p_argv[index][0], "HALT") == 0) {
		myPs->signal[APP_SIG_DATA_SAVE_PROCESS] = PHASE2;
		myPs->signal[APP_SIG_METER1_PROCESS] = PHASE2;
		myPs->signal[APP_SIG_METER2_PROCESS] = PHASE2;
		myPs->signal[APP_SIG_QUIT] = PHASE4;
	} else {
	    userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
		rtn = -1;
    }
    return rtn;
}

int ExecProcessing(int p_argc, int index)
{
/*	char buf[12];
	int bd, ch;
	
    if(strcmp((char *)&p_argv[index][0], "start") == 0
		|| strcmp((char *)&p_argv[index][0], "START") == 0) {
		memset(buf, 0x00, sizeof buf);
		memcpy((char *)&buf[0], (char *)&p_argv[index+1][0], sizeof buf);
		if(strncmp((char *)&buf[0], "cc", 2) == 0
			|| strncmp((char *)&buf[0], "CC", 2) == 0) {
			memset(buf, 0x00, sizeof buf);
			memcpy((char *)&buf[0], (char *)&p_argv[index+2][0], sizeof buf);
			myPs->selected_bd = atoi(buf) - 1;
			myPs->signal[APP_SIG_START_CC] = PHASE1;
			myPs->ch_count = 0;
			myPs->ch_step = 0;
		} else if(strncmp((char *)&buf[0], "cv", 2) == 0
			|| strncmp((char *)&buf[0], "CV", 2) == 0) {
			memset(buf, 0x00, sizeof buf);
			memcpy((char *)&buf[0], (char *)&p_argv[index+2][0], sizeof buf);
			myPs->selected_bd = atoi(buf) - 1;
			myPs->signal[APP_SIG_START_CV] = PHASE1;
			myPs->ch_count = 0;
			myPs->ch_step = 0;
		}
	} else if(strcmp((char *)&p_argv[index][0], "check") == 0
		|| strcmp((char *)&p_argv[index][0], "CHECK") == 0) {
		memset(buf, 0x00, sizeof buf);
		memcpy((char *)&buf[0], (char *)&p_argv[index+1][0], sizeof buf);
		if(strncmp((char *)&buf[0], "cc", 2) == 0
			|| strncmp((char *)&buf[0], "CC", 2) == 0) {
			memset(buf, 0x00, sizeof buf);
			memcpy((char *)&buf[0], (char *)&p_argv[index+2][0], sizeof buf);
			myPs->selected_bd = atoi(buf) - 1;
			myPs->signal[APP_SIG_CHECK_CC] = PHASE1;
			myPs->ch_count = 0;
			myPs->ch_step = 0;
		} else if(strncmp((char *)&buf[0], "cv", 2) == 0
			|| strncmp((char *)&buf[0], "CV", 2) == 0) {
			memset(buf, 0x00, sizeof buf);
			memcpy((char *)&buf[0], (char *)&p_argv[index+2][0], sizeof buf);
			myPs->selected_bd = atoi(buf) - 1;
			myPs->signal[APP_SIG_CHECK_CV] = PHASE1;
		}
	} else if(strcmp((char *)&p_argv[index][0], "update") == 0
		|| strcmp((char *)&p_argv[index][0], "UPDATE") == 0) {
		memset(buf, 0x00, sizeof buf);
		memcpy((char *)&buf[0], (char *)&p_argv[index+1][0], sizeof buf);
		myPs->selected_bd = atoi(buf) - 1;
		bd = myPs->selected_bd;
		if(WriteChCaliData(bd) < 0) {
    		userlog(DEBUG_LOG, psName,
				"Channel calibration data write fail(update %d)\n", bd+1);
		} else {
    		userlog(DEBUG_LOG, psName,
				"Channel calibration data update ok!!! %d\n", bd+1);
		}
	} else if(strcmp((char *)&p_argv[index][0], "clear") == 0
		|| strcmp((char *)&p_argv[index][0], "CLEAR") == 0) {
		memset(buf, 0x00, sizeof buf);
		memcpy((char *)&buf[0], (char *)&p_argv[index+1][0], sizeof buf);
		myPs->selected_bd = atoi(buf) - 1;
		bd = myPs->selected_bd;
		myData->chCaliData2[bd].V_DA_Offset = 0.0;
		for(ch=0; ch < myData->config.chPerBd; ch++) {
			myData->chCaliData2[bd].I_AD_slop[ch] = 1.0;
		}
		if(WriteChCaliData(bd) < 0) {
    		userlog(DEBUG_LOG, psName,
				"Channel calibration data write fail(clear %d)\n", bd+1);
		} else {
    		userlog(DEBUG_LOG, psName,
				"Channel calibration data clear ok!!! %d\n", bd+1);
		}
	} else {
	    userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
		return -10;
    }*/
	return 0;
}

int LogProcessing(int p_argc, int index)
{
    if(strcmp((char *)&p_argv[index][0], "connect") == 0
		|| strcmp((char *)&p_argv[index][0], "CONNECT") == 0) {
		index++;
		/*if(strcmp((char *)&p_argv[index][0], "v") == 0
			|| strcmp((char *)&p_argv[index][0], "V") == 0) {
			send_msg(APP_TO_METER1, MSG_APP_METER1_INITIALIZE, 0, 0);
		} else if(strcmp((char *)&p_argv[index][0], "i") == 0
			|| strcmp((char *)&p_argv[index][0], "I") == 0) {
			send_msg(APP_TO_METER1, MSG_APP_METER1_INITIALIZE, 0, 1);
		} //for ducksung*/
		//send_msg(APP_TO_METER1, MSG_APP_METER1_INITIALIZE, 0, 1);
		//send_msg(APP_TO_METER1, MSG_APP_METER1_INITIALIZE, 0, 0);
		send_msg(APP_TO_METER2, MSG_APP_METER2_INITIALIZE, 0, 0);
	} else if(strcmp((char *)&p_argv[index][0], "read") == 0
		|| strcmp((char *)&p_argv[index][0], "READ") == 0) {
		//send_msg(APP_TO_METER1, MSG_APP_METER1_REQUEST, 0, 0);
		send_msg(APP_TO_METER2, MSG_APP_METER2_REQUEST, 0, 0);
	} else if(strcmp((char *)&p_argv[index][0], "start") == 0
		|| strcmp((char *)&p_argv[index][0], "START") == 0) {
		//send_msg(APP_TO_METER1, MSG_APP_METER1_LOG_START, 0, 0);
		send_msg(APP_TO_METER2, MSG_APP_METER2_LOG_START, 0, 0);
	} else if(strcmp((char *)&p_argv[index][0], "stop") == 0
		|| strcmp((char *)&p_argv[index][0], "STOP") == 0) {
		//send_msg(APP_TO_METER1, MSG_APP_METER1_LOG_STOP, 0, 0);
		send_msg(APP_TO_METER2, MSG_APP_METER2_LOG_STOP, 0, 0);
	} else if(strcmp((char *)&p_argv[index][0], "clear") == 0
		|| strcmp((char *)&p_argv[index][0], "CLEAR") == 0) {
		send_msg(APP_TO_DATASAVE, MSG_APP_DATASAVE_LOG_CLEAR, 0, 0);
	} else {
	    userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
		return -10;
    }

	return 0;
}

