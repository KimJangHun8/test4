#ifndef __DAQ_H__
#define __DAQ_H__

void    DAQ_Start(void);
void    DAQ_Read(int);
void    DAQ_Cali_Read(int,int,int);
void    DAQ_Cali_Update(void);
void    data_read(void);
void    aCalDaqAdAverage(int);
void    aCalDaqCh(void);
void    aDaq_ChData_Mapping(void);	//190807
void	CalDaqMux(void);
void	SWAP_DAQ(double *, double *);
#endif
