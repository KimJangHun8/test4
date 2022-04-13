#ifndef	__COM_SOCKET_H__
#define	__COM_SOCKET_H__

#define	FD_MAX 64		/* maximum user # */

struct	server_status {
	char	*host;		/* hostname */
	int		port;		/* port */
	int		user;		/* maximum user */
	int		maskbytes;
	long	st_time;	/* server start time */
	int		cur_fd;		/* current file discript */
	int		cur_user;	/* current user # */
};

extern	int	FD[FD_MAX];
#ifdef		__cplusplus
extern	"C"	int		SetServerBindLoop(int flag);
extern	"C"	int		ServerEngine(int port, int (*fd)(int));
extern	"C"	int		CloseSD();
extern	"C"	int		CloseFD(int fd);
extern	"C"	int		SetClientSock(int port, char *addr);
#else
extern		int		SetServerBindLoop(int flag);
extern		int		ServerEngine(int port, int (*fd)(int));
extern		int		CloseSD();
extern		int		CloseFD(int fd);
extern		int		SetClientSock(int port, char *addr);
#endif		/* __cplusplus */
#endif		/* __SOCKET_ENGINE_H__ */
