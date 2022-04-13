//////////////////////////////////////////////////////////////////////
////	Library	: log writer
////	Date	: 2004-02-20
////	Composer: Jun-Gu Kwon
////	Comments: Write normal/error log to file/stdout
//////////////////////////////////////////////////////////////////////
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"

extern volatile S_SYSTEM_DATA *myData;

void InitLogfile(char *fname, int type)
{
	char path[128], pathname[128], file[32], filename[32], *token;
	
	memset(path, 0x00, sizeof(path));
	strcpy(path, (char *)&myData->AppControl.config.projectPath);
	strcat(path, "/config/log");
	
	memset(pathname, 0x00, sizeof pathname);
	token = strtok(path, " \t\n\r");
	if(token == NULL) strcpy(pathname, "./");
	else strcpy(pathname, token);
	
	memset(file, 0x00, sizeof file);
	strcpy(file, fname);

	memset(filename, 0x00, sizeof filename);
	token = strtok(file, " \t\n\r");
	if(token == NULL) strcpy(filename, "null");
	else strcpy(filename, token);

	memset((char *)&myData->log[type].LogPath, 0x00,
		sizeof(char)*128);
	strcpy((char *)&myData->log[type].LogPath, pathname);
	
	memset((char *)&myData->log[type].LogFile, 0x00,
		sizeof(char)*128);
	strcpy((char *)&myData->log[type].LogFile, filename);

    logopen(pathname, filename, LOG_FNAME_DATE, type);
    logdir(type);
}

/////////////////////////////////////////////////////////////
////	Declaration	: Set log-filename
////	Description	: You can manipulate log-filepath
////	  adding absolute path.
/////////////////////////////////////////////////////////////
void logopen(char *pathname, char *filename, int postfix, int type)
{
	int 	rtn, i;
	char	LogFile[256], buf[256];
	struct	tm *tm;
	time_t	t;

	if(postfix) {
		time(&t);
		tm = localtime(&t);
		memset(LogFile, 0x00, sizeof LogFile);
		//sprintf(LogFile, "%s/%s%02d%02d%02d", pathname, filename,
		//	tm->tm_year, tm->tm_mon+1, tm->tm_mday);
		sprintf(LogFile, "%s/%s%02d%02d", pathname, filename,
			tm->tm_mon+1, tm->tm_mday);
	} else {
		memset(LogFile, 0x00, sizeof LogFile);
		sprintf(LogFile, "%s/%s", pathname, filename);
	}
	strcat(LogFile, ".log");

	rtn = strcmp((char *)&myData->log[type].OpenLogFile, LogFile);
	if(rtn != 0) {//is not equal
		if(myData->log[type].OpenLogFile[0] == 0x00) {
			memset((char *)&myData->log[type].OpenLogFile, 0x00,
				sizeof(char)*128);
			strcpy((char *)&myData->log[type].OpenLogFile, LogFile);
		} else {
			for(i=1; i <= 12; i++) {
				memset(buf, 0x00, sizeof buf);
				sprintf(buf, "rm -rf %s/%s%02d%02d.log",
					pathname, filename, i, tm->tm_mday);
				system(buf);
			}
			memset((char *)&myData->log[type].OpenLogFile, 0x00,
				sizeof(char)*128);
			strcpy((char *)&myData->log[type].OpenLogFile, LogFile);
		}
	}
}

///////////////////////////////////////////////////////////////////////
////	Declaration	: Set log-ouput direction
////	Description	: Direction is defind in the "userlog.h" file.
////	  debuglogdir enable file,stdout outgoing
////	  syslogdir enable file,stdout outgoing
////	  netlogdir  enable file,stdout outgoing
///////////////////////////////////////////////////////////////////////
void logdir(int type)
{
	myData->log[type].LogDirection = LOG_2BOTH;
	//myData->log[type].LogDirection = LOG_2STDOUT;
	//myData->log[type].LogDirection = LOG_2FILE;
}

/////////////////////////////////////////////////////////////////////////
////	Declaration	: Write the message to destination
////	  (file|stdout)
////	Description	: The arguments and usage is equal
////	  to th function "printf".
////	  Only one difference is the prefix of datetime by each call.
/////////////////////////////////////////////////////////////////////////
void userlog(int type, char *psName, char *fmt, ...)
{
	char	*asciitime, tmp[128];
	time_t	curtime;
	va_list	args;
    FILE	*log_fp;

    logopen((char *)&myData->log[type].LogPath,
		(char *)&myData->log[type].LogFile, LOG_FNAME_DATE, type);

	time(&curtime);
	asciitime = ctime(&curtime);
	asciitime[24] = 0;

	va_start(args, fmt);
	if(myData->log[type].LogDirection & LOG_2FILE) {
		if(type == DEBUG_LOG && myData->AppControl.config.DebugLogFlag == 0) {
		} else {
			if((log_fp = fopen((char *)&myData->log[type].OpenLogFile, "a"))
				== NULL) return;
			memset(tmp, 0x00, sizeof(tmp));
			memcpy((char *)&tmp[0], psName, 16);
			fprintf(log_fp, "m%03d:%s:%s: ", myData->AppControl.config.moduleNo,
				asciitime, tmp);
			vfprintf(log_fp, fmt, args);
			fflush(log_fp);
			fclose(log_fp);
			chmod((char *)&myData->log[type].OpenLogFile, 0666);
		}
	}
	if(myData->log[type].LogDirection & LOG_2STDOUT) {
		memset(tmp, 0x00, sizeof(tmp));
		memcpy((char *)&tmp[0], psName, 16);
		printf("m%03d:%s:%s: ", myData->AppControl.config.moduleNo,
			asciitime, tmp);
		vprintf(fmt, args);
		fflush(stdout);
	}
	va_end(args);
}

void userlog2(int type, char *myPs, char *fmt, ...)
{
	char	*asciitime;
	time_t	curtime;
	va_list	args;
    FILE	*log_fp;

	time(&curtime);
	asciitime = ctime(&curtime);
	asciitime[24] = 0;
	va_start(args, fmt);

	if(myData->log[type].LogDirection & LOG_2FILE) {
		if((log_fp = fopen((char *)&myData->log[type].OpenLogFile, "a"))
			== NULL) return;
		vfprintf(log_fp, fmt, args);
		fflush(log_fp);
		fclose(log_fp);
		chmod((char *)&myData->log[type].OpenLogFile, 0666);
	}
	if(myData->log[type].LogDirection & LOG_2STDOUT) {
		vprintf(fmt, args);
		fflush(stdout);
	}
	va_end(args);
}
