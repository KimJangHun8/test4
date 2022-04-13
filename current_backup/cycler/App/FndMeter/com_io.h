#ifndef	__COM_IO_H__
#define	__COM_IO_H__

#ifdef		__cplusplus
extern	"C"	int	readn	(int, char *, int);
extern	"C"	int	readnt	(int, char *, int, int);
extern	"C"	int	writen	(int, char *, int);
#else
extern		int	readn	(int, char *, int);
extern		int	readnt	(int, char *, int, int);
extern		int	writen	(int, char *, int);
#endif
#endif

