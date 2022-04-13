#include <curses.h>
#include <asm/io.h>
#include <unistd.h>
#include <termios.h>
#include "MainBd.h"
#include "Range.h"
#include "DA.h"
#include "AD.h"
#include "Menu.h"
#include "KeyBoardControl.h"

static struct termios initial_settings, new_settings;
static int peek_character = -1;


void Test_Main_Mother(void)
{
	Menu mainBdTestMenu;
	int number;
	int inputKey;

	Create_Main_Mother_Type_Board_Test_Menu(&mainBdTestMenu);

	while(inputKey != 'q'){
		inputKey = ReceiveOption(&mainBdTestMenu);

		if(inputKey == 10){
			number = mainBdTestMenu.current;
			switch(number){
				
				//	DA/AD
				case 1:
					 Test_32Ch_AD2_AD_DA(16, 5, FPGA_TYPE);
				break;
				
				//	RANGE
				case 2:
					Test_Main_BD_Range(16, FPGA_TYPE);
					inputKey = 0;
				break;
				
				//	PARALLEL
				case 3:
					Test_Parallel();
					inputKey = 0;
				break;
				
				//	FAULT_STATE
				case 4:
					Test_Board_Fault_State();
					inputKey = 0;
				break;
				
				case 5:
					//	AD TEST
					Test_32Ch_AD2_AD(16, FPGA_TYPE);
				break;
			}		
		}

	}		

}
void Test_Backplan_64Ch(void)
{
	Menu mainBdTestMenu;
	int number;
	int inputKey;

	Create_Backplan_Board_Test_Menu(&mainBdTestMenu);

	while(inputKey != 'q'){
		inputKey = ReceiveOption(&mainBdTestMenu);

		if(inputKey == 10){
			number = mainBdTestMenu.current;
			switch(number){
				
				//	DA/AD
				case 1:
					Test_Backplan_AD2_AD_DA(64, 5, BACKPLAN_TYPE);
				break;
				
				//	RANGE
				case 2:
					//Test_Range_FPGA();
					Test_Main_BD_Range(16, FPGA_TYPE);
				break;
				
				//	FAULT_STATE
				case 3:
				break;
				//	AD TEST
				case 4:
					Test_Backplan_AD2_AD(64, FPGA_TYPE);
				break;
			}
					
		}

	}		

}
void Test_Backplan_32Ch(void)
{
	Menu mainBdTestMenu;
	int number;
	int inputKey;

	Create_Backplan_Board_Test_Menu(&mainBdTestMenu);

	while(inputKey != 'q'){
		inputKey = ReceiveOption(&mainBdTestMenu);

		if(inputKey == 10){
			number = mainBdTestMenu.current;
			switch(number){
				//	DA/AD
				case 1:
					Test_Backplan_AD2_AD_DA(32, 5, BACKPLAN_TYPE);
				break;
				//	RANGE
				case 2:
					//Test_Range_FPGA();
					Test_Main_BD_Range(32, BACKPLAN_TYPE);
				break;
				//	FAULT_STATE
				case 3:
				
				break;
				//	AD TEST
				case 4:
					Test_Backplan_AD2_AD(32, FPGA_TYPE);
				break;
			}
					
		}

	}		

}
void Test_FPGA_8Ch(void)
{
	Menu mainBdTestMenu;
	int number;
	int inputKey;

	Create_Main_Type_Board_Test_Menu(&mainBdTestMenu);

	while(inputKey != 'q'){
		inputKey = ReceiveOption(&mainBdTestMenu);

		if(inputKey == 10){
			number = mainBdTestMenu.current;
			switch(number){
				
				//	DA/AD
				case 1:
					 Test_32Ch_AD2_AD_DA(8, 5, FPGA_TYPE);
				break;
				
				//	RANGE
				case 2:
					Test_Main_BD_Range(16, FPGA_TYPE);
				break;
			
				case 3:
					//	AD TEST
					Test_32Ch_AD2_AD(8, FPGA_TYPE);
				break;
		
			}		
		}

	}		

}
void Test_FPGA_16Ch(void)
{
	Menu mainBdTestMenu;
	int number;
	int inputKey;

	Create_Main_Type_Board_Test_Menu(&mainBdTestMenu);

	while(inputKey != 'q'){
		inputKey = ReceiveOption(&mainBdTestMenu);

		if(inputKey == 10){
			number = mainBdTestMenu.current;
			switch(number){
				
				//	DA/AD
				case 1:
					 Test_32Ch_AD2_AD_DA(16, 5, FPGA_TYPE);
				break;
				
				//	RANGE
				case 2:
					Test_Main_BD_Range(16, FPGA_TYPE);
				break;
			
				case 3:
					//	AD TEST
					Test_32Ch_AD2_AD(16, FPGA_TYPE);
				break;
			}		
					
		}

	}		

}


void Test_FPGA_32Ch(void)
{
	Menu mainBdTestMenu;
	int number;
	int inputKey;

	Create_Main_Type_Board_Test_Menu(&mainBdTestMenu);

	while(inputKey != 'q'){
		inputKey = ReceiveOption(&mainBdTestMenu);

		if(inputKey == 10){
			number = mainBdTestMenu.current;
			switch(number){
				
				//	DA/AD
				case 1:
					 Test_32Ch_AD2_AD_DA(32, 5, FPGA_TYPE);
				break;
				
				//	RANGE
				case 2:
					Test_Main_BD_Range(32, FPGA_TYPE);
				break;
			
				case 3:
					//	AD TEST
					Test_32Ch_AD2_AD(32, FPGA_TYPE);
				break;
			}		
		}

	}		

}

void Test_CPLD_8Ch_AD2(void)
{
	Menu mainBdTestMenu;
	int number;
	int inputKey;

	Create_Main_Type_Board_Test_Menu(&mainBdTestMenu);

	while(inputKey != 'q'){
		inputKey = ReceiveOption(&mainBdTestMenu);

		if(inputKey == 10){
			number = mainBdTestMenu.current;
			switch(number){
				
				//	DA/AD
				case 1:
					 Test_32Ch_AD2_AD_DA(8, 5, CPLD_TYPE);
				break;
				
				//	RANGE
				case 2:
					Test_Main_BD_Range(16, CPLD_TYPE);
				break;
			
				case 3:
					//	AD TEST
					Test_32Ch_AD2_AD(8, CPLD_TYPE);
				break;
		
			}		
		}

	}		

}

void Test_CPLD_16Ch_AD2(void)
{
	Menu mainBdTestMenu;
	int number;
	int inputKey;

	Create_Main_Type_Board_Test_Menu(&mainBdTestMenu);

	while(inputKey != 'q'){
		inputKey = ReceiveOption(&mainBdTestMenu);

		if(inputKey == 10){
			number = mainBdTestMenu.current;
			switch(number){
				
				//	DA/AD
				case 1:
					 Test_32Ch_AD2_AD_DA(16, 5, CPLD_TYPE);
				break;
				
				//	RANGE
				case 2:
					Test_Main_BD_Range(16, CPLD_TYPE);
				break;
			
				case 3:
					//	AD TEST
					Test_32Ch_AD2_AD(16, CPLD_TYPE);
				break;
			}		
		}

	}		

}
void Test_CPLD_32Ch_AD2(void)
{
	Menu mainBdTestMenu;
	int number;
	int inputKey;

	Create_Main_Type_Board_Test_Menu(&mainBdTestMenu);

	while(inputKey != 'q'){
		inputKey = ReceiveOption(&mainBdTestMenu);

		if(inputKey == 10){
			number = mainBdTestMenu.current;
			switch(number){
				
				//	DA/AD
				case 1:
					 Test_32Ch_AD2_AD_DA(32, 5, CPLD_TYPE);
				break;
				
				//	RANGE
				case 2:
					Test_Main_BD_Range(32, CPLD_TYPE);
				break;
			
				case 3:
					//	AD TEST
					Test_32Ch_AD2_AD(32, CPLD_TYPE);
				break;
				
			}		
		}

	}		

}
void Test_CPLD_8Ch_AD1(void)
{
	Menu mainBdTestMenu;
	int number;
	int inputKey;

	Create_Main_Type_Board_Test_Menu(&mainBdTestMenu);

	while(inputKey != 'q'){
		inputKey = ReceiveOption(&mainBdTestMenu);

		if(inputKey == 10){
			number = mainBdTestMenu.current;
			switch(number){
				
				//	DA/AD
				case 1:
					 Test_32Ch_AD1_AD_DA(8, 5, CPLD_TYPE);
				break;
				
				//	RANGE
				case 2:
					Test_Main_BD_Range(8, CPLD_TYPE);
				break;
			
				case 3:
					//	AD TEST
					Test_32Ch_AD1_AD(8, CPLD_TYPE);
				break;
				
			}		
		}

	}		

}
void Test_CPLD_16Ch_AD1(void)
{
	Menu mainBdTestMenu;
	int number;
	int inputKey;

	Create_Main_Type_Board_Test_Menu(&mainBdTestMenu);

	while(inputKey != 'q'){
		inputKey = ReceiveOption(&mainBdTestMenu);

		if(inputKey == 10){
			number = mainBdTestMenu.current;
			switch(number){
				
				//	DA/AD
				case 1:
					 Test_32Ch_AD1_AD_DA(16, 5, CPLD_TYPE);
				break;
				
				//	RANGE
				case 2:
					Test_Main_BD_Range(16, CPLD_TYPE);
				break;
			
				case 3:
					//	AD TEST
					Test_32Ch_AD1_AD(16, CPLD_TYPE);
				break;
			}		
		}

	}		

}

void Test_CPLD_32Ch_AD1(void)
{
	Menu mainBdTestMenu;
	int number;
	int inputKey;

	Create_Main_Type_Board_Test_Menu(&mainBdTestMenu);

	while(inputKey != 'q'){
		inputKey = ReceiveOption(&mainBdTestMenu);

		if(inputKey == 10){
			number = mainBdTestMenu.current;
			switch(number){
				
				//	DA/AD
				case 1:
					 Test_32Ch_AD1_AD_DA(32, 5, CPLD_TYPE);
				break;
				
				//	RANGE
				case 2:
					Test_Main_BD_Range(32, CPLD_TYPE);
				break;
			
				case 3:
					//	AD TEST
					Test_32Ch_AD1_AD(32, CPLD_TYPE);
				break;
			}
		}

	}		

}



int Test_Board_Fault_State(void)
{
	int key = 'i';
	int sameFlag = 0;
	int keyBuffer = 'i';

	mInputValue input;
	mInputValue inputBuffer;
	mInputValue* pInputBuffer;
	mInputValue* pInput;

	if(iopl(3)) exit(1);

	pInput = &input;
	pInputBuffer = &inputBuffer;

	Init_Fault_Value(pInput);
	inputBuffer = input;

	clear();

	Draw_Fault_State(pInput);

	Init_Keyboard();
	while(key != 'q'){
		clear();
		 Read_Fault_Value(pInputBuffer);
		 sameFlag = Compare_Board_Fault_Value(pInput, pInputBuffer);
		 if(sameFlag != 1){
			input = inputBuffer;
			Draw_Fault_State(pInput);
		 }
		 if(Keyboard_Hit()){
			key = Read_Character();
			if(key == 'q'){
			}
		 }
	}
	Close_Keyboard();
	
	return 0;		
					
}

int Compare_Board_Fault_Value(mInputValue* org, mInputValue* other)
{
	int sameFlag = 0;
			
	if( org->smpsFail7V == other->smpsFail7V
		&& org->smpsFail3V == other->smpsFail3V
		&& org->ot1 == other->ot1
		&& org->ot2 == other->ot2){
		sameFlag = 1;
	}else{
		sameFlag = 0;
	}
	
	return sameFlag;
					
}
void Draw_Fault_State(mInputValue* input)
{
	int x, y;

	int *pX, *pY;
	clear();
	
	x = 1; y= 1;
	pX = &x; 
	pY = &y;
	
	
	attrset(COLOR_PAIR(1) | A_BOLD );
	move(x, y);
	printw("Input state. ");
	attroff(COLOR_PAIR(1) | A_BOLD );
	
	x +=1; y = 1;
	move(x, y);
	
	printw("7V SMPS state.");
	x +=1; y = 1;
	Draw_SMPS_State(input->smpsFail7V, pX, pY);
	
	x += 2; y = 1;
	move(x, y);
	printw("3V SMPS state.");
	x +=1; y = 1;
	Draw_SMPS_State(input->smpsFail3V, pX, pY);
	
	x += 3; y = 1;
	move(x, y);
	printw("OT state.");
	x +=1; y = 1;
	Draw_OT_State(input->ot1, pX, pY, 0);
	
	x += 2; y = 1;
	x +=1; y = 1;
	Draw_OT_State(input->ot2, pX, pY, 1);
	x += 2; y = 1;
	move(x, y);

	attrset(COLOR_PAIR(6) | A_BOLD );
	printw("if you went to exit push 'q' button");
	attroff(COLOR_PAIR(6) | A_BOLD );

	refresh();

}

void Test_32Ch_AD2_AD_DA(int installedCh, int refVal, int bdType)
{
	float vArray[72];
	int i;

	AdData data;
	
	DaAddr daAddr;
	AdAddr adAddr;
	
	DaAddr* pDaAddr;
	AdAddr* pAdAddr;

	pDaAddr = &daAddr;
	pAdAddr = &adAddr;

	if(iopl(3)) exit(1);

	switch(bdType){
		case FPGA_TYPE:
			Setup_Main_Bd_AD2_FPGA_Address(pAdAddr);
			Setup_Main_Bd_DA_FPGA_Address(pDaAddr);
			break;
		case CPLD_TYPE:
			Setup_Main_Bd_AD2_CPLD_Address(pAdAddr);
			Setup_Main_Bd_DA_CPLD_Address(pDaAddr);
			break;
		default:
			break;
	}
	//channel voltage
	for(i =1; i <= installedCh; i++){
		Output_Voltage_Channel(pDaAddr, i, refVal);

		Read_32Ch_AD2_Data(pAdAddr, installedCh, vArray);

		Print_32Ch_AD_Data(vArray, i, installedCh);
		
		Output_Voltage_Channel(pDaAddr, i, 0);
	}
	//channel current
	for(i =1; i <= installedCh; i++){
		Output_Current_Channel(pDaAddr, i, refVal);

		Read_32Ch_AD2_Data(pAdAddr, installedCh, vArray);

		Print_32Ch_AD_Data(vArray, i+32, installedCh);
		
		Output_Current_Channel(pDaAddr, i, 0);
	}
}

// 구현필요 


void Test_32Ch_AD1_AD_DA(int installedCh, int refVal, int bdType)
{
	float vArray[72];
	int i;

	AdData data;
	
	DaAddr daAddr;
	AdAddr adAddr;
	
	DaAddr* pDaAddr;
	AdAddr* pAdAddr;

	pDaAddr = &daAddr;
	pAdAddr = &adAddr;

	if(iopl(3)) exit(1);
			
	Setup_Main_Bd_AD1_CPLD_Address(pAdAddr);
	Setup_Main_Bd_DA_CPLD_Address(pDaAddr);
	

	//channel voltage
	for(i =1; i <= installedCh; i++){
		Output_Voltage_Channel(pDaAddr, i, refVal);

		Read_32Ch_AD1_Data(pAdAddr, installedCh, vArray);

		Print_32Ch_AD_Data(vArray, i, installedCh);
		
		Output_Voltage_Channel(pDaAddr, i, 0);
	}
	//channel current
	for(i =1; i <= installedCh; i++){
		Output_Current_Channel(pDaAddr, i, refVal);

		Read_32Ch_AD1_Data(pAdAddr, installedCh, vArray);

		Print_32Ch_AD_Data(vArray, i+32, installedCh);
		
		Output_Current_Channel(pDaAddr, i, 0);
	}
}

void Test_Backplan_AD2_AD_DA(int installedCh, int refVal, int bdType)
{
	float vArray[72];
	float vArray64[136];
	int i;

	AdData data;
	
	DaAddr daAddr;
	AdAddr adAddr;
	
	DaAddr* pDaAddr;
	AdAddr* pAdAddr;

	pDaAddr = &daAddr;
	pAdAddr = &adAddr;

	if(iopl(3)) exit(1);
	
	Setup_Backplan_AD_Address(pAdAddr);
	Setup_Backplan_Bd_DA_Address(pDaAddr);

	
	if( installedCh > 32){
		//channel voltage
		for(i =1; i <= installedCh; i++){
			Output_Voltage_Channel_For_Backplan_64Ch(pDaAddr, i, refVal);
			Read_Backplan_64Ch_AD2_Data(pAdAddr, installedCh, vArray64);
			Print_64Ch_AD_Data(vArray64, i, installedCh);
			Output_Voltage_Channel_For_Backplan_64Ch(pDaAddr, i, 0);
		}	
		//channel current
		for(i =1; i <= installedCh; i++){
			Output_Current_Channel_For_Backplan_64Ch(pDaAddr, i, refVal);
			Read_Backplan_64Ch_AD2_Data(pAdAddr, installedCh, vArray64);
			Print_64Ch_AD_Data(vArray64, i+64, installedCh);
			Output_Current_Channel_For_Backplan_64Ch(pDaAddr, i, 0);
		}
	}//if end
	else{
		//channel voltage
		for(i =1; i <= installedCh; i++){
			Output_Voltage_Channel_For_Backplan_32Ch(pDaAddr, i, refVal);
			Read_Backplan_32Ch_AD2_Data(pAdAddr, installedCh, vArray);
			Print_32Ch_AD_Data(vArray, i, installedCh);
			Output_Voltage_Channel_For_Backplan_32Ch(pDaAddr, i, 0);
		}	
		//channel current
		for(i =1; i <= installedCh; i++){
			Output_Current_Channel_For_Backplan_32Ch(pDaAddr, i, refVal);
			Read_Backplan_32Ch_AD2_Data(pAdAddr, installedCh, vArray);
			Print_32Ch_AD_Data(vArray, i+32, installedCh);
			Output_Current_Channel_For_Backplan_32Ch(pDaAddr, i, 0);
		}

	}//else end
	
}

void Test_32Ch_AD2_AD(int installedCh, int bdType)
{
	int key = 'i';
	int sameFlag = 0;
	float readData[72];
	int i;
	int ch;
   	float val;

	DaAddr daAddr;
	AdAddr adAddr;
	
	DaAddr* pDaAddr;
	AdAddr* pAdAddr;

	pDaAddr = &daAddr;
	pAdAddr = &adAddr;


	if(iopl(3)) exit(1);
	
	clear();
	
	switch(bdType){
		case FPGA_TYPE:
			Setup_Main_Bd_AD2_FPGA_Address(pAdAddr);
			Setup_Main_Bd_DA_FPGA_Address(pDaAddr);
			break;
		case CPLD_TYPE:
			Setup_Main_Bd_AD2_CPLD_Address(pAdAddr);
			Setup_Main_Bd_DA_CPLD_Address(pDaAddr);
			break;
		default:
			break;
	}
	
	Init_Keyboard();
	while(key != 'q'){
		Read_32Ch_AD2_Data(pAdAddr, installedCh, readData);
		
		Print_32Ch_AD_Data(readData, 0, installedCh);

		move(24, 40);
		printw("(");	
		attrset(COLOR_PAIR(1) | A_BOLD);
		printw("v");
		attroff(COLOR_PAIR(1) | A_BOLD);
		printw(" or ");
		
		attrset(COLOR_PAIR(1) | A_BOLD);
		printw("i");
		attroff(COLOR_PAIR(1) | A_BOLD);

		printw(" key is output");
		printw(")");	
		refresh();
	
		if(Keyboard_Hit()){
			key = Read_Character();
			if(key == 'v' || key == 'V'){
				move(23, 25);
				printw("%s", "cmd is(ex, Channel Value) : ");
				refresh();
				echo();
				ch = 0; val = 0;
				fflush(stdin);
				scanw("%d %f", &ch, &val);
				noecho();
				move(23, 60);
				printw("V %d %1.3f ", ch, val);
				
				refresh();
				sleep(1);
				if( ch <= installedCh){
					if( val <= 10 && val >= -10){
						Output_Voltage_Channel(pDaAddr, ch, val);
					}
				}
				key = 0;
			}else if( key == 'i'){				
				move(23, 35);
				printw("%s", "cmd is(ex, Channel Value) : ");
				refresh();
				echo();
				ch = 0; val = 0;
				fflush(stdin);
				scanw("%d %f", &ch, &val);
				noecho();
				move(23, 60);
				printw("I %d %1.3f ", ch, val);
				
				refresh();
				sleep(1);
				if( ch <= installedCh){
					if( val <= 10 && val >= -10){
						Output_Current_Channel(pDaAddr, ch, val);
					}
				}
				key = 0;
			}

		}
	}
	Output_Voltage_All_Channel(pDaAddr, installedCh, 0);
	Output_Current_All_Channel(pDaAddr, installedCh, 0);
	
	Close_Keyboard();

}

void Test_32Ch_AD1_AD(int installedCh, int bdType)
{
	int key = 'i';
	int sameFlag = 0;
	float readData[72];
	int i;
	int ch;
   	float val;

	DaAddr daAddr;
	AdAddr adAddr;
	
	DaAddr* pDaAddr;
	AdAddr* pAdAddr;

	pDaAddr = &daAddr;
	pAdAddr = &adAddr;


	if(iopl(3)) exit(1);
	
	clear();
	
	Setup_Main_Bd_AD1_CPLD_Address(pAdAddr);
	Setup_Main_Bd_DA_CPLD_Address(pDaAddr);

	Init_Keyboard();
	while(key != 'q'){
		Read_32Ch_AD1_Data(pAdAddr, installedCh, readData);
		
		Print_32Ch_AD_Data(readData, 0, installedCh);

		move(24, 40);
		printw("(");	
		attrset(COLOR_PAIR(1) | A_BOLD);
		printw("v");
		attroff(COLOR_PAIR(1) | A_BOLD);
		printw(" or ");
		
		attrset(COLOR_PAIR(1) | A_BOLD);
		printw("i");
		attroff(COLOR_PAIR(1) | A_BOLD);

		printw(" key is output");
		printw(")");	
		refresh();
	
		if(Keyboard_Hit()){
			key = Read_Character();
			if(key == 'v' || key == 'V'){
				move(23, 25);
				printw("%s", "cmd is(ex, Channel Value) : ");
				refresh();
				echo();
				ch = 0; val = 0;
				fflush(stdin);
				scanw("%d %f", &ch, &val);
				noecho();
				move(23, 60);
				printw("V %d %1.3f ", ch, val);
				
				refresh();
				sleep(1);
				if( ch <= installedCh){
					if( val <= 10 && val >= -10){
						Output_Voltage_Channel(pDaAddr, ch, val);
					}
				}
				key = 0;
			}else if( key == 'i'){				
				move(23, 35);
				printw("%s", "cmd is(ex, Channel Value) : ");
				refresh();
				echo();
				ch = 0; val = 0;
				fflush(stdin);
				scanw("%d %f", &ch, &val);
				noecho();
				move(23, 60);
				printw("I %d %1.3f ", ch, val);
				
				refresh();
				sleep(1);
				if( ch <= installedCh){
					if( val <= 10 && val >= -10){
						Output_Current_Channel(pDaAddr, ch, val);
					}
				}
				key = 0;
			}

		}
	}
	Output_Voltage_All_Channel(pDaAddr, installedCh, 0);
	Output_Current_All_Channel(pDaAddr, installedCh, 0);
	
	Close_Keyboard();

}


void Test_Backplan_AD2_AD(int installedCh, int bdType)
{
	int key = 'i';
	int sameFlag = 0;
	float readData[72];
	float readData64[136];
	int i;
	int ch;
   	float val;

	DaAddr daAddr;
	AdAddr adAddr;
	
	DaAddr* pDaAddr;
	AdAddr* pAdAddr;

	pDaAddr = &daAddr;
	pAdAddr = &adAddr;


	if(iopl(3)) exit(1);
	
	clear();
	Setup_Backplan_AD_Address(pAdAddr);
	Setup_Backplan_Bd_DA_Address(pDaAddr);

	
	Init_Keyboard();
	while(key != 'q'){
		if( installedCh > 32){
			Read_Backplan_64Ch_AD2_Data(pAdAddr, installedCh, readData64);
			Print_64Ch_AD_Data(readData64, 0, installedCh);
		}else{
			Read_Backplan_32Ch_AD2_Data(pAdAddr, installedCh, readData);
			Print_32Ch_AD_Data(readData, 0, installedCh);
		}
		move(24, 40);
		printw("(");	
		attrset(COLOR_PAIR(1) | A_BOLD);
		printw("v");
		attroff(COLOR_PAIR(1) | A_BOLD);
		printw(" or ");
		
		attrset(COLOR_PAIR(1) | A_BOLD);
		printw("i");
		attroff(COLOR_PAIR(1) | A_BOLD);

		printw(" key is output");
		printw(")");	
		refresh();
	
		if(Keyboard_Hit()){
			key = Read_Character();
			if(key == 'v' || key == 'V'){
				move(23, 25);
				echo();
				printw("%s", "cmd is(ex, Channel Value) : ");
				refresh();
				ch = 0; val = 0;
				fflush(stdin);
				scanw("%d %f", &ch, &val);
				noecho();
				move(23, 60);
				printw("V %d %1.3f ", ch, val);
				
				refresh();
				sleep(1);
				if( ch <= installedCh){
					if( val <= 10 && val >= -10){						
						if( installedCh > 32){
							Output_Voltage_Channel_For_Backplan_64Ch(pDaAddr, ch, val);
						}else{
							Output_Voltage_Channel_For_Backplan_32Ch(pDaAddr, ch, val);
						}

					}
				}
				key = 0;
			}else if( key == 'i'){				
				move(23, 35);
				printw("%s", "cmd is(ex, Channel Value) : ");
				refresh();
				echo();
				ch = 0; val = 0;
				fflush(stdin);
				scanw("%d %f", &ch, &val);
				noecho();
				move(23, 60);
				printw("I %d %1.3f ", ch, val);
				
				refresh();
				sleep(1);
				if( ch <= installedCh){
					if( val <= 10 && val >= -10){
						if( installedCh > 32){
							Output_Current_Channel_For_Backplan_64Ch(pDaAddr, ch, val);
						}else{
							Output_Current_Channel_For_Backplan_32Ch(pDaAddr, ch, val);
						}


					}
				}
				key = 0;
			}

		}
	}
	if( installedCh > 33){
		Output_Voltage_All_Channel_For_Backplan_64Ch(pDaAddr, installedCh, 0);
		Output_Current_All_Channel_For_Backplan_64Ch(pDaAddr, installedCh, 0);
	}
	else{
		Output_Voltage_All_Channel_For_Backplan_32Ch(pDaAddr, installedCh, 0);
		Output_Current_All_Channel_For_Backplan_32Ch(pDaAddr, installedCh, 0);
	}
	Close_Keyboard();

}

void Init_Fault_Value(mInputValue* value)
{
	value->smpsFail7V = 0x00;
	value->smpsFail3V = 0x00;
	value->ot1 = 0x00;
	value->ot2= 0x00;
}

void Read_Fault_Value(mInputValue* buffer)
{
	unsigned char tmp1, tmp2;
	
	tmp1 = inb(0x629);
	usleep(10000);
	tmp2 = inb(0x62c);
	usleep(10000);
	tmp1 |= (tmp2 << 4);
	buffer->smpsFail7V = tmp1;
	
	usleep(10000);
	tmp1 = inb(0x62a);
	usleep(10000);
	tmp2 = inb(0x62d);
	tmp1 |= (tmp2 <<4);
	buffer->smpsFail3V = tmp1;
	
	usleep(10000);
	buffer->ot1 = inb(0x62b);
	usleep(10000);
	buffer->ot2 = inb(0x62e);
}


void Print_32Ch_AD_Data(float* vArray, int current, int installedCh)
{
	int x,y;
	int i, j;
	int compareValue;
	int max;
	
	clear();
	x = 0;
	y = 6;
	move(x, y);
	attrset(COLOR_PAIR(6) | A_BOLD);
	printw("Voltage AD");
	attroff(COLOR_PAIR(6) | A_BOLD);
	x += 1;
	max = installedCh / 8;
	for( i = 0; i < max; i ++){
		for( j = 0; j < 8; j++){
			if( current < 33){
				compareValue =(i * 8) + j + 1;
				if(compareValue == current){
					attrset(COLOR_PAIR(4) | A_BOLD);
				}else{
					attroff(COLOR_PAIR(4) | A_BOLD);
				}
			}
			move(x, y);
			printw("%1.3f", vArray[j + (i * 8)]);
			y += 9;
		}
		attroff(COLOR_PAIR(4) | A_BOLD);
		x += 2;
		y = 6;
	}
	switch(max){
		case 1:	
			x += 6;
			break;
		case 2:
			x += 4;
			break;
		case 3:
			x += 2;
			break;
	}
			
	y = 6;
	move(x, y);	
	attrset(COLOR_PAIR(6) | A_BOLD);
	printw("Current AD");
	attroff(COLOR_PAIR(6) | A_BOLD);

	x += 1;
	for( i = 0; i < max; i ++){
		for( j = 0; j < 8; j++){
			if( current > 32){
				compareValue = (i * 8) + j +33;
				if(compareValue == current){
					attrset(COLOR_PAIR(4) | A_BOLD);
				}else{
					attroff(COLOR_PAIR(4) | A_BOLD);
				}
			}
			move(x, y);
			printw("%1.3f", vArray[j + ((i * 8) +32) ]);
			y += 9;
		}
		attroff(COLOR_PAIR(4) | A_BOLD);
		x += 2;
		y = 6;
	}	
	switch(max){
		case 1:	
			x += 6;
			break;
		case 2:
			x += 4;
			break;
		case 3:
			x += 2;
			break;
	}

	y = 6;
	move(x, y);
	printw("V reference");
	x++;
	for( i = 0; i < 4; i++){
		move(x, y);
		printw("%1.3f", vArray[i + 64 ]);
		y += 9;
	}
	x += 2;
	y = 6;
	move(x, y);
	printw("I reference");
	x++;
	for( i = 0; i < 4; i++){
		move(x, y);
		printw("%1.3f", vArray[i + 68]);
		y += 9;
	}
	x += 2;
	y = 6;
	move(x, y);
	printw("if you want to exit push");
	attrset(COLOR_PAIR(1) | A_BOLD);
	printw(" q ");
	attroff(COLOR_PAIR(6) | A_BOLD);
	printw("button");

	refresh();
	usleep(500000);
}


void Print_64Ch_AD_Data(float* vArray, int current, int installedCh)
{
	int x,y;
	int i, j;
	int compareValue;
	int max;
	
	clear();
	x = 0;
	y = 6;
	move(x, y);
	attrset(COLOR_PAIR(6) | A_BOLD);
	printw("Voltage AD");
	attroff(COLOR_PAIR(6) | A_BOLD);
	x += 1;
	max = installedCh / 8;
	for( i = 0; i < max; i ++){
		for( j = 0; j < 8; j++){
			if( current < 65){
				compareValue =(i * 8) + j + 1;
				if(compareValue == current){
					attrset(COLOR_PAIR(4) | A_BOLD);
				}else{
					attroff(COLOR_PAIR(4) | A_BOLD);
				}
			}
			move(x, y);
			printw("%1.3f", vArray[j + (i * 8)]);
			y += 9;
		}
		attroff(COLOR_PAIR(4) | A_BOLD);
		x += 1;
		y = 6;
	}
	switch(max){
		case 1:	
			x += 6;
			break;
		case 2:
			x += 4;
			break;
		case 3:
			x += 2;
			break;
	}
			
	y = 6;
	move(x, y);	
	attrset(COLOR_PAIR(6) | A_BOLD);
	printw("Current AD");
	attroff(COLOR_PAIR(6) | A_BOLD);

	x += 1;
	for( i = 0; i < max; i ++){
		for( j = 0; j < 8; j++){
			if( current > 64){
				compareValue = (i * 8) + j +65;
				if(compareValue == current){
					attrset(COLOR_PAIR(4) | A_BOLD);
				}else{
					attroff(COLOR_PAIR(4) | A_BOLD);
				}
			}
			move(x, y);
			printw("%1.3f", vArray[j + ((i * 8) +64) ]);
			y += 9;
		}
		attroff(COLOR_PAIR(4) | A_BOLD);
		x += 1;
		y = 6;
	}	
	switch(max){
		case 1:	
			x += 6;
			break;
		case 2:
			x += 4;
			break;
		case 3:
			x += 2;
			break;
	}

	y = 6;
	move(x, y);
	printw("V reference");
	x++;
	for( i = 0; i < 4; i++){
		move(x, y);
		printw("%1.3f", vArray[i + 128]);
		y += 9;
	}
	x += 2;
	y = 6;
	move(x, y);
	printw("I reference");
	x++;
	for( i = 0; i < 4; i++){
		move(x, y);
		printw("%1.3f", vArray[i + 132]);
		y += 9;
	}
	x += 2;
	y = 6;
	move(x, y);
	printw("if you want to exit push");
	attrset(COLOR_PAIR(1) | A_BOLD);
	printw(" q ");
	attroff(COLOR_PAIR(6) | A_BOLD);
	printw("button");

	refresh();
	usleep(500000);
}


