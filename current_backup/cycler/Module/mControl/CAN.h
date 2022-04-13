#ifndef __CAN_H__
#define __CAN_H__

void can_transmit1(int);
void CAN_Control(int, int);
void make_packet(S_RT_CAN_MSG *, char);
void can_aging_test(void);
void bCan_Each_Range_Out(int, int, int);
void bCan_Each_Run_Out(int, int, int);
void bCan_Each_Ref_Cmd_Out(char, int, int, int);
void bCan_Each_Link_Out(int, int);
void bCan_Full_Run_Range_Out(int, int, char, char);
void bCan_Full_Ref_Cmd_Out(char, int, int, int, long);
void can_transmit(int, int, U_CAN_FULL_SEND_MSG *);
void can_main_msg_parsing(unsigned int, int, char*);
void can_inv_msg_parsing(int, char*);
void can_read_msg(int, int);
void Can_Group_Select(int);
void Can_Main_Link_Check(int);
void Can_Inv_Link_Check(int);
void Can_Io_Link_Check(int);
void can_inv_send_cmd(unsigned char ,int);
void can_io_msg_parsing(unsigned int, int, char*);
#endif
