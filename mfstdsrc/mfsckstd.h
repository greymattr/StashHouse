/*****************************************************************************
*
*  filename: mfsckstd.h
*  desc    : header file for Matthew Fatheree std Socket programming.
*
*  Revison History:
* -----------------------------------------------------------------------------
* 04-26-2005    -    Adapted mfsckstd.h from mfstd.h              M.Fatheree
*
*
*
*
* (c) copyright 2005 Matthew Fatheree
*******************************************************************************/


#ifndef	__MFSTD_H__
#define	__MFSTD_H__

#include  <ctype.h>       /* for 'isalpha()' */

#include	<sys/types.h>	/* basic system data types */
#include	<sys/socket.h>	/* basic socket definitions */
#include	<sys/time.h>	/* timeval{} for select() */
#include	<time.h>		/* timespec{} for pselect() */
#include	<netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include	<arpa/inet.h>	/* inet(3) functions */
#include	<errno.h>
#include	<fcntl.h>		/* for nonblocking */
#include	<netdb.h>
#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/stat.h>	/* for S_xxx file mode constants */
#include	<unistd.h>
#include	<sys/wait.h>
#include	<sys/un.h>		/* for Unix domain sockets */
#include  <stdarg.h>
#ifdef	HAVE_SYS_SELECT_H
#include	<sys/select.h>	/* for convenience */
#endif

#ifdef	HAVE_POLL_H
#include	<poll.h>		/* for convenience */
#endif

#ifdef	HAVE_STRINGS_H
#include	<strings.h>		/* for convenience */
#endif

/* Three headers are normally needed for socket/file ioctl's:
 * <sys/ioctl.h>, <sys/filio.h>, and <sys/sockio.h>.
 */
#ifdef	HAVE_SYS_IOCTL_H
#include	<sys/ioctl.h>
#endif
#ifdef	HAVE_SYS_FILIO_H
#include	<sys/filio.h>
#endif
#ifdef	HAVE_SYS_SOCKIO_H
#include	<sys/sockio.h>
#endif

/* OSF/1 actually disables recv() and send() in <sys/socket.h> */
#ifdef	__osf__
#undef	recv
#undef	send
#define	recv(a,b,c,d)	recvfrom(a,b,c,d,0,0)
#define	send(a,b,c,d)	sendto(a,b,c,d,0,0)
#endif

#ifndef	INADDR_NONE
#define	INADDR_NONE	0xffffffff	/* should have been in <netinet/in.h> */
#endif

#ifndef	SHUT_RD				/* these three Posix.1g names are quite new */
#define	SHUT_RD		0	/* shutdown for reading */
#define	SHUT_WR		1	/* shutdown for writing */
#define	SHUT_RDWR	2	/* shutdown for reading and writing */
#endif

#ifndef INET_ADDRSTRLEN
#define	INET_ADDRSTRLEN		16	/* "ddd.ddd.ddd.ddd\0"
								    1234567890123456 */
#endif

/* Define following even if IPv6 not supported, so we can always allocate
   an adequately-sized buffer, without #ifdefs in the code. */
#ifndef INET6_ADDRSTRLEN
#define	INET6_ADDRSTRLEN	46	/* max size of IPv6 address string:
				   "xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx" or
				   "xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:ddd.ddd.ddd.ddd\0"
				    1234567890123456789012345678901234567890123456 */
#endif

/* Define bzero() as a macro if it's not in standard C library. */
#ifndef	HAVE_BZERO
#define	bzero(ptr,n)		memset(ptr, 0, n)
#endif


/* Posix.1g renames "Unix domain" as "local IPC".
   But not all systems DefinE AF_LOCAL and AF_LOCAL (yet). */
#ifndef	AF_LOCAL
#define AF_LOCAL	AF_UNIX
#endif
#ifndef	PF_LOCAL
#define PF_LOCAL	PF_UNIX
#endif


/* Following could be derived from SOMAXCONN in <sys/socket.h>, but many
   kernels still #define it as 5, while actually supporting many more */
#define	LISTENQ		1024	/* 2nd argument to listen() */

/* Miscellaneous constants */
#define	MAXLINE		4096	/* max text line length */
#define	MAXSOCKADDR  128	/* max socket address structure size */
#define	BUFFSIZE	8192	/* buffer size for reads and writes */

/* Following shortens all the type casts of pointer arguments */
#define	SA	    struct sockaddr
#define	SA_in	struct sockaddr_in

#ifndef HOSTENT
#define HOSTENT struct hostent
#endif

#define	min(a,b)	((a) < (b) ? (a) : (b))
#define	max(a,b)	((a) > (b) ? (a) : (b))

#define MFDEBUG         /* to make API verbose */

extern int is_ip_address(const char *ip);
extern int tcp_send_file(char *filed, char *ipaddr, int port);
extern int udp_send_file(char *fname, char *ipaddr, int portto, int portfrom);
extern int fdprintf(int fd, const char *fmt, ...);
extern int sdprintf(int fd, const char *fmt, ...);
extern int tcp_connect(char *host, int sock);
extern int udp_connect(char *host, int sock);
extern int tcp_connect_from(int locprt, char *host, int sock);
extern int udp_connect_from(int locprt, char *host, int sock);
extern int init_udp(int udp_port);
extern int init_tcp(int tcp_port);
extern int mf_tcp_server(int locprt, int (*callback)());

extern int mf_dprintf(const char *fmt, ...);

#define NEED_MF_CRC
#ifdef  NEED_MF_CRC
extern unsigned long crc32(const char *);
#endif

#endif	/* __mfstd_h */
