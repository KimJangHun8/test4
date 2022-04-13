#ifndef __SERIAL_H__
#define __SERIAL_H__

#define MAX_SERIAL_PACKET_LENGTH	MAX_FADM_PACKET_LENGTH
#define MAX_SERIAL_PACKET_COUNT		MAX_FADM_PACKET_COUNT

int		ComPortInitialize(int, int);
int		SerialPacket_Receive(void);
void	SerialPacket_Parsing(void);
int		SerialCommand_Receive(void);
int		SerialCommand_Parsing(void);
int		Parsing_SerialEvent(void);

int		CmdHeader_Check(char *);
int		CheckSum_Check(void);

void	rcv_cmd_ad_reply(int, int);
void	rcv_cmd_ad_ref_reply(int, int, int);

void	send_cmd_request(int, int);
void	send_cmd_comm_check(int, int);
void	send_cmd_wr_base_addr(int, int);

void	make_header(char *, int, int);
void	make_check_sum(char *, int);
int		send_command(char *, int, int);

int		ComPortStateCheck(void);

#endif
