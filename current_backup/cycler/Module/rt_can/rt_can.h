#ifndef RT_CAN_H
#define RT_CAN_H

extern int	rt_can_write(int, const char *);
extern int  rt_can_read(int, char *);

extern int	set_reset_mode(int);
extern int	set_normal_mode(int, unsigned char);
extern int	sja1000_baud_rate(int, int, int);

extern int  rt_can_set_param(int, int, int, int, int);
extern int  rt_can_setup(int, int, int, unsigned char);
extern int	init_module(void);
extern void cleanup_module(void);

#endif
