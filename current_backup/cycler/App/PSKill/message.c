#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "message.h"

extern volatile S_SYSTEM_DATA *myData;

void send_msg(int toPs, int msg, int ch, int val)
{
    int idx;
    idx = myData->msg[toPs].write_idx;
    idx++;
    if(idx >= MAX_MSG) idx = 0;

	myData->msg[toPs].msg_val[idx].msg = msg;
    myData->msg[toPs].msg_val[idx].ch = ch;
    myData->msg[toPs].msg_val[idx].val = val;
    myData->msg[toPs].write_idx = idx;
}
