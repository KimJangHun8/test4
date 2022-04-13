#ifndef		__USERLOG_H__
#define		__USERLOG_H__

#define		LOG_OFF		0
#define		LOG_2FILE	1	// 0001
#define		LOG_2STDOUT	2	// 0010
#define		LOG_2SOCKET	4	// 0100
#define		LOG_2BOTH	3	// 0011
#define		LOG_2ALL	7	// 0111
#define		LOG_FNAME_ONLY	0
#define		LOG_FNAME_DATE	1

#ifdef		__cplusplus
extern "C" void		InitLogfile	(char *, int);
extern "C" void		logopen		(char *, char *, int, int);
extern "C" void		logdir		(int);
extern "C" void		userlog		(int, char *, char *, ...);
extern "C" void		userlog2	(int, char *, char *, ...);
#else
extern 	void		InitLogfile	(char *, int);
extern 	void		logopen		(char *, char *, int, int);
extern	void		logdir		(int);
extern	void		userlog		(int, char *, char *, ...);
extern	void		userlog2	(int, char *, char *, ...);
#endif
#endif
