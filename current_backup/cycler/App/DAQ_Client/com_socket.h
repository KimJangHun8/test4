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
extern	"C"	void	write_ACK(int);
extern	"C"	void	write_NAK(int);
extern	"C"	int		BuildMask(void);
extern	"C" int		GetStdInOutOK(void);
extern	"C" void	SetStdInOutOK(int);
extern	"C" void	InitFD(void);
extern	"C" int		SetFD(void);
extern	"C" char *	GetHostName(void);
extern	"C" struct server_status *	SetServerInfo(int);
extern	"C"	int		SetServerBindLoop(int);
extern	"C" int		GetServerSock(int);
extern	"C" int		DoAccept(void);
extern	"C" int		ProcessFD(int, int (*fp)(int));
extern	"C" int		ProcessMask(int *, fd_set *, int (*fp)(int));
extern	"C"	int		CloseSD(void);
extern	"C"	int		CloseFD(int);
extern	"C"	int		ServerEngine(int, int (*fd)(int));
extern	"C"	int		SetClientSock(int, char *);
extern	"C" int		UseSocket(int, char *);
#else
extern		void	write_ACK(int);
extern		void	write_NAK(int);
extern		int		BuildMask(void);
extern		int		GetStdInOutOK(void);
extern		void	SetStdInOutOK(int);
extern		void	InitFD(void);
extern		int		SetFD(void);
extern		char *	GetHostName(void);
extern		struct server_status *	SetServerInfo(int);
extern		int		SetServerBindLoop(int);
extern		int		GetServerSock(int);
extern		int		DoAccept(void);
extern		int		ProcessFD(int, int (*fp)(int));
extern		int		ProcessMask(int *, fd_set *, int (*fp)(int));
extern		int		CloseSD(void);
extern		int		CloseFD(int);
extern		int		ServerEngine(int, int (*fd)(int));
extern		int		SetClientSock(int, char *);
extern		int		UseSocket(int, char *);
#endif		/* __cplusplus */
#endif		/* __SOCKET_ENGINE_H__ */
