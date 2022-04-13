#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <curses.h>
#include "main.h"
#include "Menu.h"
#include "ControlBd.h"
#include "MainBd.h"
#include "Debug.h"
int main(void)
{
	Menu mainMenu, controlBdTestMenu, mainBdTestMenu, backplanBdTestMenu ;
	Menu mainBdAd2TestMenu, mainBdAd1TestMenu;
	int inputKey;
	int mainMenuFlag =0;
	int number = 0;

	

	Create_Main_Menu(&mainMenu);
	Create_Backplan_Board_Menu(&backplanBdTestMenu);
	
	initscr();		//basic frame
	Set_Font_Style();
	
	while(inputKey != 'q'){
		inputKey = ReceiveOption(&mainMenu);
		
		if(inputKey == 10){
			number = mainMenu.current;
			switch(number){
				//Control bd
				case 1:
					Create_Control_Board_Menu(&controlBdTestMenu);
					inputKey = ReceiveOption(&controlBdTestMenu);
					if(inputKey == 10)
						switch(controlBdTestMenu.current){
							case 1:
								Test_Control_Bd();	//PNE201209 CONTROL BD REV02
							break;
							case 2:
								// slot type control bd
							break;
					inputKey = 0;
					}

					break;
				//Main bd
				case 2:
					Create_Main_Board_Menu(&mainBdTestMenu);
					inputKey = ReceiveOption(&mainBdTestMenu);
					if(inputKey == 10){
						switch(mainBdTestMenu.current){
							case 1:
								Test_Main_Mother();
								break;
							case 2:
								//32ch new
								break;
							case 3:
								//ad2
								Create_Main_Board_AD2_Test_Menu(&mainBdAd2TestMenu);
								inputKey = ReceiveOption(&mainBdAd2TestMenu);
								if(inputKey == 10){
									switch(mainBdAd2TestMenu.current){
										case 1:
											Test_FPGA_8Ch();
											break;
										case 2:
											Test_FPGA_16Ch();
											break;
										case 3:
											Test_FPGA_32Ch();
											break;
										case 4:
											Test_CPLD_8Ch_AD2();
											break;
										case 5:
											Test_CPLD_16Ch_AD2();
											break;
										case 6:
											Test_CPLD_32Ch_AD2();
											break;
											
										difult :

											break;
											
									}
								}
								break;
							case 4:
								//ad1
								Create_Main_Board_AD1_Test_Menu(&mainBdAd1TestMenu);
								inputKey = ReceiveOption(&mainBdAd1TestMenu);
								if(inputKey == 10){
									switch(mainBdAd1TestMenu.current){
										case 1:
											Test_CPLD_8Ch_AD1();
											break;
										case 2:
											Test_CPLD_16Ch_AD1();
											break;
										case 3:
											Test_CPLD_32Ch_AD1();
											break;
	

									}
								}
								break;
							case 5:
								//16ch cpld
								break;
							case 6:
								//32ch cpld
							break;
							
						}
						inputKey = 0;
					}
								
					break;
				//Backplan bd
				case 3:
					Create_Backplan_Board_Menu(&mainBdTestMenu);
					inputKey = ReceiveOption(&mainBdTestMenu);
					if(inputKey == 10){
						switch(mainBdTestMenu.current){
							case 1:
								Test_Backplan_64Ch();
								break;
							case 2:
								Test_Backplan_32Ch();
								break;
						}
								
					}
					break;
				//Debug
				case 4:
					Print_Debug();
											
	
			}
		}
			
	}
		



		
	
	endwin();		//basic frame
	exit(EXIT_SUCCESS);


	return 0;
}


int Set_Font_Style()
{
	start_color();
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_GREEN);
	init_pair(3, COLOR_GREEN, COLOR_RED);
	init_pair(4, COLOR_YELLOW, COLOR_BLUE);
	init_pair(5, COLOR_BLACK, COLOR_WHITE);
	init_pair(6, COLOR_YELLOW, COLOR_BLACK);
	init_pair(7, COLOR_CYAN, COLOR_WHITE);
	init_pair(8, COLOR_WHITE, COLOR_BLACK);

}
