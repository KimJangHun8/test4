#include "Debug.h"
#include <stdio.h>
#include <asm/io.h>


void Print_Debug()
{
	
	int x, y;
	char buffer[32];
	int stopFlag = 0;
	int val1, val2;
	unsigned char returnValue = 0x00;
	clear();

	if(iopl(3)) exit(1);
	
	x = 1; y = 1;
	
	while(stopFlag != 1){
		move(x, y);
		printf("echo print>>");
		refresh();
		sleep(1);
		echo();
		fflush(stdin);
		memset(buffer, 0x00, sizeof buffer);
		val1 = 0; val2 = 0;
		scanw("%s %x %x", buffer, &val1, &val2);
		noecho();

		if(strcmp(buffer, "quit") == 0){
			clear();
			printw("quit have a good day");
			refresh();
			stopFlag = 1;
			sleep(3);
		}else if(strcmp(buffer, "wr") == 0){
			outb(val1, val2);
			printw("%s %x %x", buffer, val1, val2);
		}else if(strcmp(buffer, "get") == 0){
			returnValue = inb(val1);
			printw("%s %x %02x", buffer, val1, returnValue);
		}else{
			y = 1;
			move( x, y);
		}
		

		x += 2; y = 1;
		if( x >24){
			x = 1;
			clear();
		}
	}
	

	sleep(3);

}
