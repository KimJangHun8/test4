#include <curses.h>
#include <asm/io.h>
#include <unistd.h>
#include <termios.h>
#include "ControlBd.h"
#include "BitMacro.h"
#include "Range.h"
#include "KeyBoardControl.h"

//static struct termios initial_settings, new_settings;
//static int peek_character = -1;


/*  date: 2013. 7. 2			name : pms
 *	parameter: 	value 	: 	read value
 *				x		:	horizentel value
 *				y		:	vertical value
 *				
 * discription:
 *	read 602 address and show the state.
 */ 

int Test_Control_Bd(void)
{	
	int key = 'i';
	int sameFlag = 0;
	int keyBuffer = 'i';
	short int tmp;
	inputValue input;
	inputValue inputBuffer;
	inputValue* pInputBuffer; 
	inputValue* pInput;
	
	if(iopl(3)) exit(1);
		
	pInput = &input;
	pInputBuffer = &inputBuffer;
	
	Init_Value(pInput);
	inputBuffer = input;

	clear();
	
	Draw_Input_State(pInput);
	
	Init_Keyboard();
	while(key != 'q'){
		clear();
		if(key == 'i'){
			Read_Value(pInputBuffer);
			sameFlag = Compare_Value(pInput, pInputBuffer);
			if(sameFlag != 1){
				input = inputBuffer;
				Draw_Input_State(pInput);
			}
		}else if( key == 'o'){
			clear();
			refresh(); 
			Draw_Output_State();
			Check_Output();
		}else if( key == 'c'){
			Draw_Connection_State();
			//Test_For_Range();
			Test_Main_BD_Range(16, CPLD_TYPE);
		}
		if(Keyboard_Hit()){
			key = Read_Character();
			if(key == 'i' && keyBuffer == 'o'){
				Draw_Input_State(pInput);
			}
				
			if(key == 'o'){
				keyBuffer = key;
			}
			if( key == 'q'){
				outb(0x00, 0x601);
				outb(0x00, 0x602);
				outb(0x00, 0x603);
				outb(0x00, 0x604);
				outb(0x00, 0x610);
				outb(0x00, 0x611);
			}
		}
		
			
	}
	Close_Keyboard();
	return 0;
}
void Check_Output()
{
	int runLedFlag = 0;
	int tempOutputValue, outputValue, index;
	sleep(1);
	//if the run led lamp off, 
	//the run led lamp will be on
	if(runLedFlag == 0){
		outb( 0x4, 0x604);
		runLedFlag = 1;
	}
	//ext & cal port on
	sleep(1);	
	outb(0xff, 0x601);
	outb(0xff, 0x602);
	outb(0x3, 0x603);
	//ext & cal port off
	sleep(1);
	outb(0x0, 0x601);
	outb(0x0, 0x602);
	outb(0x0, 0x603);
	//ext bit check
	tempOutputValue = 0x1;
	for(index = 0; index < 8; index++){
		outputValue = tempOutputValue << index;
		usleep(90000);
		outb(outputValue, 0x601);
		outb(outputValue, 0x602);
	}
								
	outb(outputValue, 0x603);

	tempOutputValue = tempOutputValue << 7;
	for(index = 0; index < 8; index++){
		outputValue = tempOutputValue >> index;
		usleep(90000);
		outb(outputValue, 0x601);
		outb(outputValue, 0x602);
	}
	outb(0x2, 0x603);
	sleep(1);
	tempOutputValue = outputValue;
	for(index = 0; index < 8; index ++){
		outputValue |= tempOutputValue << index;
		usleep(90000);
		outb(outputValue, 0x601);
		outb(outputValue, 0x602);
	}
	outb(0x3, 0x603);
	sleep(1);
	outb(0x0, 601);
	outb(0x0, 602);
	outb(0x0, 603);
	

}

void Init_Value(inputValue* value)
{
	value->display = 0x00;
	value->pSFail7V = 0x00;
	value->pSFail3V = 0x00;
	value->oT1 = 0x00;
	value->oT2 = 0x00;
}

void Read_Value(inputValue* buffer)
{

	buffer->display 	= inb(0x602);
	buffer->pSFail7V 	= inb(0x610);
	buffer->pSFail3V 	= inb(0x612);
	buffer->oT1 		= inb(0x611);
	buffer->oT2 		= inb(0x613);

}
	
int Compare_Value(inputValue* org, inputValue* other)
{
	int sameFlag = 0; 

	if(org->display == other->display
	&& org->pSFail7V == other->pSFail7V
	&& org->pSFail3V == other->pSFail3V
	&& org->oT1 == other->oT1
	&& org->oT2 == other->oT2){
		sameFlag = 1;
	}else{
		sameFlag = 0;
	}

	return sameFlag;

}



/*  date: 2013. 7. 2			name : pms
 *	parameter: 	value 	: 	read value
 *				x		:	horizentel value
 *				y		:	vertical value
 *				
 * discription:
 *	read 602 address and show the state.
 *
 */ 
void Draw_Connection_State()
{
	int x, y;
	clear();
	x = 1; y= 1;
	
	
	attrset(COLOR_PAIR(1) | A_BOLD );
	move(x, y);
	printw("Connection Check ");
	attroff(COLOR_PAIR(1) | A_BOLD );

	x += 20; y = 1;	
	move(x, y);

	attrset(COLOR_PAIR(6) | A_BOLD );
	printw("Input Check 'i' Output Check'o' Data bus check 'c'");
	attroff(COLOR_PAIR(6) | A_BOLD );

	refresh();

}


void Draw_Output_State()
{
	int x, y;
	int *pX, *pY;
	clear();
	x = 1; y= 1;
	
	
	attrset(COLOR_PAIR(1) | A_BOLD );
	move(x, y);
	printw("Output state. ");
	attroff(COLOR_PAIR(1) | A_BOLD );

	x += 20; y = 1;	
	move(x, y);

	attrset(COLOR_PAIR(6) | A_BOLD );
	printw("Input Check 'i' Output Check'o' Data bus check 'c'");
	attroff(COLOR_PAIR(6) | A_BOLD );

	refresh();

}
void Draw_Input_State(inputValue* input)
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
	
	Draw_ETC_State(input->display, pX,  pY);	
	x += 3; y = 1;
	
	move(x, y);
	printw("7V SMPS state.");
	x +=1; y = 1;
	Draw_SMPS_State(input->pSFail7V, pX, pY);
	
	x += 2; y = 1;
	move(x, y);
	printw("3V SMPS state.");
	x +=1; y = 1;
	Draw_SMPS_State(input->pSFail3V, pX, pY);
	
	x += 3; y = 1;
	move(x, y);
	printw("OT state.");
	x +=1; y = 1;
	Draw_OT_State(input->oT1, pX, pY, 0);
	
	x += 2; y = 1;
	x +=1; y = 1;
	Draw_OT_State(input->oT2, pX, pY, 1);
	x += 2; y = 1;
	move(x, y);

	attrset(COLOR_PAIR(6) | A_BOLD );
	printw("Input Check 'i' Output Check'o' Data bus check 'c'");
	attroff(COLOR_PAIR(6) | A_BOLD );

	refresh();

}
/*  date: 2013. 7. 2			name : pms
 *	parameter: 	value 	: 	read value
 *				x		:	horizentel value
 *				y		:	vertical value
 *				
 * discription:
 *	read 602 address and show the state.
 */ 
void Draw_ETC_State(int value, int* x, int* y)
{
	int i, tmp;
	int a, b;
	char* pInfo;
	int checkBit;

	char info[6][16] = {	" PS_IN ",
							"  EMG  ",
							"PW_FAIL",
							"BAT_LOW",
							"PW_HOLD",
							"PS_FAIL"
						};
	*y = 70;
	for(i = 0; i < 6; i++){
		move(*x, *y);
		attrset(COLOR_PAIR(5));
		pInfo = info[i];
		printw(" %s", pInfo);
		attroff(COLOR_PAIR(5));
		*y =  *y - 10;
	}
	*x += 2;
	*y = 70;
	checkBit = 0;
	for(i =6; i > 0; i--){
		move(*x, *y);
		attroff(COLOR_PAIR(4) | A_BOLD);
		if(1 == Macro_Check_bit_Set(value, checkBit)){
				attrset(COLOR_PAIR(4) | A_BOLD);
			printw("  O  N  ");
		}else{
			printw("  OFF  ");
		} *y -= 10;
		checkBit++;
	}
	attroff(COLOR_PAIR(4) | A_BOLD);
}
/*  date: 2013. 7. 2			name : pms
 *	parameter: 	value 	: 	read value
 *				x		:	horizentel value
 *				y		:	vertical value
 *				
 * discription:
 *	read 610 & 612 address and show smps state.
 *
 */ 
void Draw_SMPS_State(int value, int* x, int* y)
{
	int i, tmp;
	int a, b;
		
	for(i = 0; i < 8; i++){
		move(*x, *y);
		attrset(COLOR_PAIR(5));
		printw("  %d Ch  ", i+1);
		attroff(COLOR_PAIR(5));
		*y =  *y + 10;
	}
	*x += 1;
	*y = 1;
	for(i =0; i < 8; i++){
		move(*x, *y);
		attroff(COLOR_PAIR(4) | A_BOLD);
		if(1 == Macro_Check_bit_Set(value, i)){
			attrset(COLOR_PAIR(4) | A_BOLD);
			printw("  O  N  ");
		}else{
			printw("  OFF  ");
		} *y += 10;
	}
	attroff(COLOR_PAIR(4) | A_BOLD);
}
/*  date: 2013. 7. 2			name : pms
 *	parameter: 	value 	: 	read value
 *				x		:	horizentel value
 *				y		:	vertical value
 *				
 * discription:
 *	read 611 & 613 address and show the state.
 *
 */ 
void Draw_OT_State(int value, int* x, int* y, int number)
{
	int i, tmp;
	int a, b;
	
	for(i = 0; i < 8; i++){
		move(*x, *y);
		attrset(COLOR_PAIR(5));
		switch(number){
			case 0:
				printw("  %d Ch  ", i+1);
				break;
			case 1:
				printw(" %d Ch  ", i+9);
				break;
		}
		attroff(COLOR_PAIR(5));
		*y =  *y + 10;
	}
	*x += 1;
	*y = 1;
	for(i =0; i < 8; i++){
		move(*x, *y);
		attroff(COLOR_PAIR(4) | A_BOLD);
		if(1 == Macro_Check_bit_Set(value, i)){
			attrset(COLOR_PAIR(4) | A_BOLD);
			printw("  O  N  ");
		}else{
			printw("  OFF  ");
		} *y += 10;
	}
	attroff(COLOR_PAIR(4) | A_BOLD);
}

