#include "BitMacro.h"



#define Macro_Set_Bit_Flag		0
#define Macro_Clear_Bit_Flag	0
#define Macro_Invert_Bit_Flag	0

#define Macro_Set_Area_Flag		0
#define Macro_Clear_Area_Flag 	0
#define Macro_Invert_Area_Flag	0

#define Macro_Wrtie_Block_Flag		0
#define Macro_Check_Bit_Set_Flag	0
#define Macro_Check_Bit_Clear_Flag	0

/*
int main(void)
{
	
	int i, j;
	unsigned char dest = 0x00;
	int pos = 0;
	int bits = 0x0;

#if Macro_Set_Bit_Flag
	for(i = 0; i < 8; i++){
		dest = 0x00;
		Macro_Set_Bit(dest, i);
		printf("%d is %x\n", dest);
	}
#endif	

#if Macro_Clear_Bit_Flag
	for(i = 0 ;i < 8; i ++){
	dest = 0xff;
		Macro_Clear_Bit(dest,i);
		printf("%d is %x\n", i, dest);
	}
#endif

#if Macro_Invert_Bit_Flag
	dest = 0xff;
	for(j = 0; j <2; j++){
		for(i = 0; i < 8; i++){
			Macro_Invert_Bit(dest, i);
			printf("%d is %x\n", j * 8 + i, dest);
		}
	}
#endif



#if Macro_Set_Area_Flag
	for(i = 0; i < 8; i++){
		dest = 0x00;
		Macro_Set_Area(dest, 0x7, i);
		printf("%d is %x\n", i, dest);
	}
#endif

#if Macro_Clear_Area_Flag
	for(i = 0; i < 8; i++){
		dest = 0xff;
		Macro_Clear_Area(dest, 0x07, i);
		printf("%d is %x\n", i, dest);
	}

#endif
	
#if  Macro_Invert_Area_Flag
	for(j = 0; j < 2; j++){
		for(i = 0; i < 8; i++){
			Macro_Invert_Area(dest, 0x03, i);
			printf("%d is %x\n", j * 8 +i, dest);
		}
	}
#endif	

	
	
*/
