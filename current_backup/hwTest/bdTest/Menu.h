#ifndef _MENU_H
#define _MENU_H

#define SUCCESS 1
#define FAIL	0
#define REFRESH 1
#define STOP	1
#define LOCAL_ESCAPE_KEY	27


typedef struct _Menu
{
	char* subject[20];			//항목을 집어넣는 공간
	int current;				//사용자가 선택한 항목
	int totalSubjectCount;		//총 항목개수
	
			
}Menu;

//top menu
int Create_Main_Menu(Menu* menu);

//control menu
int Create_Control_Board_Menu(Menu* menu);


//main bd menu
int Create_Main_Board_Menu(Menu* menu);

int Create_Main_Board_AD2_Test_Menu(Menu* menu);

int Create_Main_Board_AD1_Test_Menu(Menu* menu);

//main bd sub menu
int Create_Main_Mother_Type_Board_Test_Menu(Menu* menu);
int Create_Main_Type_Board_Test_Menu(Menu* menu);


//backplan menu
int Create_Backplan_Board_Menu(Menu* menu);
int Create_Backplan_Board_Test_Menu(Menu* menu);



//menu function
int Draw_Menu(Menu* menu);

int Receive_Option(Menu* menu);


#endif
