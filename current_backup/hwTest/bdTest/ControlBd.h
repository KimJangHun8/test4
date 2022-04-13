

#ifndef _CONTROL_BD_H
#define _CONTROL_BD_H


typedef struct _inputValue
{
	int	refreshFlag;
	unsigned char display;
	int pSFail7V;
	int pSFail3V;
	int oT1;
	int oT2;
}inputValue;

int		Test_Control_Bd(void);

void	Init_Value(inputValue* value);
void 	Read_Value(inputValue* buffer);
int 	Compare_Value(inputValue* org, inputValue* other);



void 	Draw_Input_State(inputValue* input);
void 	Draw_ETC_State(int value, int* x, int* y);
void 	Draw_SMPS_State(int value, int* x, int* y);
void 	Draw_OT_State(int value, int* x, int* y, int number);
void	Draw_Output_State();
void	Draw_Connection_State();

void	Check_Input();
void	Check_Output();
void	Check_MainBd_Bus();

#endif
