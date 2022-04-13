#ifndef __SERIAL_H__
#define __SERIAL_H__

#define MAX_SERIAL_PACKET_LENGTH	MAX_CALI_METER_PACKET_LENGTH
#define MAX_SERIAL_PACKET_COUNT		MAX_CALI_METER_PACKET_COUNT

int		ComPortInitialize(int, int);
int		SerialPacket_Receive(void);
void	SerialPacket_Parsing(void);
int		SerialCommand_Receive(void);
int		SerialCommand_Parsing(void);
int		Parsing_SerialEvent(void);

int		CmdHeader_Check(char *);

int		rcv_cmd_answer(void);

void	send_cmd_initialize(int);
void	send_cmd_request(void);

void	make_header(char *, char, int, int, int);
void	make_check_sum(char *, int);
int		send_command(char *, int, int);

int		ComPortStateCheck(void);

#endif
