#ifndef __SERIAL_H__
#define __SERIAL_H__

#define MAX_SERIAL_PACKET_LENGTH	MAX_ANALOG_METER_PACKET_LENGTH
#define MAX_SERIAL_PACKET_COUNT		MAX_ANALOG_METER_PACKET_COUNT

int		ComPortInitialize(void);
int		SerialPacket_Receive(void);
void	SerialPacket_Parsing(void);
int		SerialCommand_Receive(void);
int		SerialCommand_Parsing(void);
int		Parsing_SerialEvent(void);

int		CmdHeader_Check(char *);
int		CmdHeader_Check2(void);

int		rcv_cmd_answer(void);
int		rcv_cmd_open_close(void);
int		rcv_cmd_answer2(void);

void	send_cmd_initialize(int, int, int);
void	send_cmd_initialize2(int, int);
void	send_cmd_request(int, int);
void	send_cmd_request1(int, int);
void	send_cmd_request2(int, int);
void	send_cmd_open(int);
void	send_cmd_close(int);

void	make_header(char *, char, int, int, int);
void	make_check_sum(char *, int);
int		send_command(char *, int, int);

int		ComPortStateCheck(void);
void 	next_module_request(void);

#endif
