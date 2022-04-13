#ifndef __CALI_H__
#define __CALI_H__

int analog_cali_read(int,int,int);
int analog_cali_calc(int);
int analog_cali_calc_2(int);
int analog_cali_update(void);
int analog_cali_update_2(void);
void analog_cali_auto(void);
#endif
