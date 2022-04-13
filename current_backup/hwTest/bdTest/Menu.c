#include "Menu.h"
#include <curses.h>


int Create_Main_Menu(Menu* menu)
{
	menu->subject[0] = "TEST MENU";
	menu->subject[1] = "1. CONTROL BD TEST MENU";
	menu->subject[2] = "2. MAIN BD TEST MENU";
	menu->subject[3] = "3. Backplan BD TEST MENU.";
	menu->subject[4] = "4. Debug Mode";
	menu->current = 0;
	menu->totalSubjectCount = 5;
	
	return SUCCESS;
}

int Create_Control_Board_Menu(Menu* menu)
{
	menu->subject[0] = "CONTROL BD  LIST";
	menu->subject[1] = "1. PNE201209_CONTROL BD REV02";
	menu->subject[2] = "2. PNE201304 SBC CONTROL CD REV04(FPGA)";
	menu->subject[3] = "3. PNE201005 CONTROL CD REV06(CPLD)";
	menu->current = 0;
	menu->totalSubjectCount = 4;

	return SUCCESS;
}

int Create_Main_Board_Menu(Menu* menu)
{
	menu->subject[0] = "MAIN BD LIST";
	menu->subject[1] = "1. PNE 201311-16CH MAIN(FPGA) BD REV11";
	menu->subject[2] = "2. PNE 201311-32CH MAIN(FPGA) BD REV11";
	menu->subject[3] = "3. AD 2";	
	menu->subject[4] = "4. AD 1";
	menu->current = 0;
	menu->totalSubjectCount = 5;

	return SUCCESS;

}
int Create_Main_Board_AD2_Test_Menu(Menu* menu)
{
	menu->subject[0] = " MAIN BD AD2 TEST LIST";
	menu->subject[1] = "1.  8 CH MAIN BD FPGA TYPE";
	menu->subject[2] = "2. 16 CH MAIN BD FPGA TYPE";
	menu->subject[3] = "3. 32 CH MAIN BD FPGA TYPE";
	menu->subject[4] = "4.  8 CH MAIN BD CPLD TYPE";
	menu->subject[5] = "5. 16 CH MAIN BD CPLD TYPE";
	menu->subject[6] = "6. 32 CH MAIN BD CPLD TYPE";
	menu->current = 0;
	menu->totalSubjectCount = 7;

	return SUCCESS;
}
int Create_Main_Board_AD1_Test_Menu(Menu* menu)
{
	menu->subject[0] = " MAIN BD AD1 TEST LIST";
	menu->subject[1] = "1.  8 CH MAIN BD CPLD TYPE";
	menu->subject[2] = "2. 16 CH MAIN BD CPLD TYPE";
	menu->subject[3] = "3. 32 CH MAIN BD CPLD TYPE";
	menu->current = 0;
	menu->totalSubjectCount = 4;

	return SUCCESS;
}

	
int Create_Main_Mother_Type_Board_Test_Menu(Menu* menu)
{
	menu->subject[0] = "MAIN BOARD TEST MENU";
	menu->subject[1] = "1. DA/AD";
	menu->subject[2] = "2. RANGE";
	menu->subject[3] = "3. PARALLEL";
	menu->subject[4] = "4. FAULT STATE";
	menu->subject[5] = "5. AD";
	menu->current = 0;
	menu->totalSubjectCount = 6;

	return SUCCESS;
}

int Create_Main_Type_Board_Test_Menu(Menu* menu)
{
	menu->subject[0] = "MAIN BOARD TEST MENU";
	menu->subject[1] = "1. DA/AD";
	menu->subject[2] = "2. RANGE";
	menu->subject[3] = "3. AD";
	menu->current = 0;
	menu->totalSubjectCount = 4;

	return SUCCESS;
}

int Create_Backplan_Board_Menu(Menu* menu)
{

	menu->subject[0] = "BACKPLAN BOARD LIST";
	menu->subject[1] = "1. 64 CH BACKPLAN";
	menu->subject[2] = "2. 32 CH BACKPLAN";
	menu->current = 0;
	menu->totalSubjectCount = 3;

	return SUCCESS;

}
int Create_Backplan_Board_Test_Menu(Menu* menu)
{	
	menu->subject[0] = "BACKPLAN BOARD TEST MENU";
	menu->subject[1] = "1. DA/AD";
	menu->subject[2] = "2. RANGE";
	menu->subject[3] = "3. FAULT STATE";
	menu->subject[4] = "4. AD";
	menu->current = 0;
	menu->totalSubjectCount = 5;

	return SUCCESS;
}





int DrawMenu(Menu* menu)
{
	int i, x, y;
	x =1; y = 25;

	clear();	
	attron(A_BOLD);
	for( i = 0; i < menu->totalSubjectCount; i++){
		move(x, y);
		if(menu->current != 0 && menu->current == i ){
			attron(A_STANDOUT);
		}else{
			attroff(A_STANDOUT);
		}	
		printw("%s", menu->subject[i]);
		x+=2;
	}
	attroff(A_STANDOUT);
	move(19, 25);
	printw("top menu 'q' ");
	move(20, 25);
	printw("%s", "select :");

}

int ReceiveOption(Menu* menu)
{
	int key = 0; 
	int tmp = 0;
	int keyTemp = 0;
	int stopFlag = 0;
	
	clear();
	crmode();
	keypad(stdscr, TRUE);
	noecho();
	move(20, 50);
	
	DrawMenu(menu);
	refresh();
	
	while( key != ERR 
		&& key != 'q' 
		&& stopFlag != STOP ){
		move(20,50);
		clrtoeol();
		key = getch();
		if(key >= 48 && key <= 57){
			tmp = ((int)key -48);
			menu->current = tmp;
			if(keyTemp != key ){
				DrawMenu(menu);
				printw("%d  ", tmp);
				refresh();
				keyTemp = key;
			}
		}
		if(key == 10){
			if(menu->current != 0 
			&& menu->current <= menu->totalSubjectCount){
				stopFlag = STOP;
			}
		}
	}
	return key;
}


