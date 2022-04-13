#ifndef __PCU_H_
#define __PCU_H__

unsigned char	SREG8_read1(unsigned long );
unsigned char	SREG8_read2(unsigned long );
unsigned short int	SREG16_read(unsigned long);
void	SREG8_write(unsigned long, unsigned char);
void	SREG16_write(unsigned long, unsigned short int);
unsigned char	ch_send_cmd(int, int, unsigned char, unsigned char, long, long);
unsigned char	ch_mode_set(int, int, unsigned char, unsigned char, long, long);
void	PCU_Relay_OnOff(int, int);
void	HVU_OnOff(int, int, int);
void 	PCU_ParallelSwitch_OnOff(int, int);
void 	PCU_Inverter_OnOff(int, int);	//180615 add
#endif
