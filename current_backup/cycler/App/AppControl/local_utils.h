#ifndef __LOCAL_UTILS_H__
#define __LOCAL_UTILS_H__

void	Kill_Process(char *);
void 	Check_Process(void);
int 	Read_LoadProcess(void);
int		DieCheck_Process(char *);
int		Load_Process(char *, char *, int);
void	Close_Process(char *, int, int);

void	Save_SystemMemory(void);
int		Read_SystemMemory(char *);
void	Init_SystemMemory();

int 	Read_AppControl_Config(int);
int 	Read_mControl_Config(void);
int		Read_DIO_Config(void);
int		Read_DIO_Setting(void);
int		Read_DIO_SignalNo(void);
int		Read_DIN_USE_FLAG(void);
int		Read_DOUT_USE_FLAG(void);
int		Read_PCU_INV_USE_FLAG(void);	//180627
int		Read_TempFault_Config(void);	//210303

int		Read_Calibration_Config(void);
int		Read_Calibration_Data(void);
int		Read_BdCaliData(int);
int		Read_BdCaliData_FAD(int);
int		Read_BdCaliData_I_Offset(int); //20160229
int		Read_CellArray_A(void);
int		Read_ChamArray_A(void);
int 	Read_CaliData_Line_Drop(void);// 20160315

int 	Create_BdCaliData_Org(int);
int		Read_Addr_Interface(void);
int		Read_Addr_Main(void);
int		Read_FaultBitSet_SMPS_OT(void);
int		Read_ChAttribute(void);
int		Read_ChCompData(void);
int		Write_ChAttribute(void);
int		Write_ChCompData(void);
int		Read_AuxSetData(void);
int		Write_AuxSetData(void);
int		Read_AuxCaliData(void);
int		Read_DaqArray(void);
int		Write_AuxCaliData(void);
int		Read_FadCaliData(void);
int 	Read_FadOffsetData(void);

int		Read_ChamberChNo(void);
int		Write_ChamberChNo(void);
int		Write_Default_ChamberChNo(void);	//hun_211020
//int		Read_Restore_Memory_Config(void);   //pms add for Restore Memory

int		ChData_Backup(int, int, int); //150512<-- oys add
int		ChData_Restore(int, int, int); //150512<-- oys add

int 	Read_Auto_Cali_Config(void);		//pms add for auto cali

int		Write_HwFault_Config(void); //210204 lyhw
int		Read_HwFault_Config(void);
int		Write_SwFault_Config(void); //210418 hun
int		Read_SwFault_Config(void);
int 	Read_Function_Flag(void);
int		Read_Chamber_Motion(void);

//20171008 sch add
int		Write_LimitUserVI(void);
int		Read_LimitUserVI(void);

//211025 hun
int		Read_LGES_Fault_Config_Version_Check(void);
int		Read_LGES_Fault_Config_Update(void);
int		Write_LGES_Fault_Config(void);
int		Read_LGES_Fault_Config(void);
int		Read_LGES_Ambient2(void);
int 	Read_LGES_code_buzzer_on(void);
int		Read_Semi_Autojig_DCR_Calculate(void);

int		Read_SDI_inv_fault_continue(void);
int		Read_SDI_CC_CV_hump_Config(void);
int		Write_SDI_CC_CV_hump_Config(void);
int		Read_SDI_Pause_save_Config(void);
int		Write_SDI_Pause_save_Config(void);

int 	Read_PCU_Config(void);	//180612 add
int 	Read_CAN_Config(void); //20190605 KHK

int		Read_SBC_IP_Address(void); //20190712 oys
void	Config_backup(void); //20190723 oys
void 	Update_bashrc(void); //20190910 oys
void 	Update_proftpd_config(void); //20191206 oys
int		unload_process_using_mbuff(void);	//20190823 oys
int		Read_Parameter_Update(void);	//190801 lyhw

int 	Read_DYSON_Maintenance_Config(void); //20220121 jsh
int		Write_DYSON_Maintenance_Config(void); //20220121 jsh
#endif
