/*****************************************************************************
*
*  filename: mfsckstd.c
*  desc    : c function file for Matthew Fatheree std Socket programming.
*
*  Revison History:
* -----------------------------------------------------------------------------
* 04-26-2005    -    Adapted mfsckstd.c from mfstd.c              M.Fatheree
*
*
*
*
* (c) copyright 2005 Matthew Fatheree
*******************************************************************************/
#include "mfsckstd.h"

#ifdef NEED_MF_CRC
static unsigned long crc_table[256];
static int inited=0;
#endif

int is_ip_address(const char *ip)
{
	struct sockaddr_in sa;
	int result = inet_pton(AF_INET, ip, &(sa.sin_addr));
	if( result == 1 ) {
		return result;
	} else {
		return 0;
	}
}


/**
 * [init_tcp - open a tcp socket file descriptor ]
 * @param  tcp_port [ the tcp port number ]
 * @return          [ on success it will return the file descriptor, on error < 1]
 */
int init_tcp(int tcp_port)
{
	int sd, slen, i;	/*socket file descriptor and length */
	SA_in ssfd;

  if((sd=socket(AF_INET, SOCK_STREAM, 0))<0)
  {
		mf_dprintf("Could not open socket %d\n\r", sd);
  	return ( sd );
  }

  if(sd>0)
  {
    ssfd.sin_family=AF_INET;
    ssfd.sin_addr.s_addr=htonl(INADDR_ANY);
    ssfd.sin_port=htons(tcp_port);
    slen=sizeof(ssfd);
  }

  if((i=bind(sd, (SA *)&ssfd, slen))<0)
  {
		mf_dprintf("could not bind socket to file descriptor %d\n\r", i);
    close(sd);
    return ( -i );
  }
return sd;
}

/**
 * [init_udp - opens a udp socket descriptor ]
 * @param  udp_port [ the udp port number ]
 * @return          [ the socket descriptor, on failure < 1]
 */
int init_udp(int udp_port)
{

	int sd, slen = 0, i;	/*socket file descriptor and length */
	SA_in ssfd;

  if((sd=socket(AF_INET, SOCK_DGRAM, 0))<0)
  {
    mf_dprintf("Could not open socket %d\n\r", sd);
    return ( sd );
  }
  if(sd>0)
  {
	  ssfd.sin_family=AF_INET;
	  ssfd.sin_addr.s_addr=htonl(INADDR_ANY);
	  ssfd.sin_port=htons(udp_port);
	  slen=sizeof(ssfd);
  }
  if((i=bind(sd, (SA *)&ssfd, slen))<0)
  {
    mf_dprintf("could not bind socket to file descriptor %d\n\r", i);
    close(sd);
    return i;
  }
  return sd;
}

/**
 * [tcp_connect - connect a tcp socket file descriptor ]
 * @param  host [ ip or hostname of device to be connected to ]
 * @param  sock [ the tcp port number to connect to on the above host ]
 * @return      [ the socket file descriptor. On fail < 1]
 */
int tcp_connect(char *host, int sock)
{
	int	                sockfd;
	SA_in	        servaddr;       /* unp.h, in.h */
	struct in_addr	*pptr;  	/* in.h        */
	HOSTENT     	*hp;   		/* netdb.h     */
	int ok;

	/* look up using DNS or dotted quad */
	if ((hp = gethostbyname(host)) == NULL) {
		mf_dprintf("gethostbyname failure - host not found\n\r");
		return(-1);
	}
	/* make sure address is filled in structure */
	if ((pptr = (struct in_addr *)hp->h_addr) == NULL) {
		mf_dprintf("gethostbyname failure - no address\n\r");
		return(-1);
	}
	/* set up socket */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		mf_dprintf("socket failure %d\n\r", sockfd);
		printf("%s\n\r", strerror( errno ));
		return( -1 );
	}

	/* set up server address */
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(sock);
	memcpy(&servaddr.sin_addr, pptr, sizeof(struct in_addr));

	/* connect to server */
	if ((ok = connect(sockfd, (SA *) &servaddr, sizeof(servaddr))) < 0) {
		mf_dprintf("\n\rconnect failure %d\n\r", ok);
		close(sockfd);
		return(-1);
	}
	return(sockfd);
}
/**
 * [udp_connect - connect a udp socket file descriptor ]
 * @param  host [ ip or hostname of device to be connected to ]
 * @param  sock [ the udp port number to connect to on the above host ]
 * @return      [ the socket file descriptor. On fail < 1]
 */
int udp_connect(char *host, int sock)
{
	int	                sockfd;
	SA_in	        servaddr;       /* unp.h, in.h */
	struct in_addr	*pptr;  	/* in.h        */
	HOSTENT     	*hp;   		/* netdb.h     */
	int ok;

	/* look up using DNS or dotted quad */
	if ((hp = gethostbyname(host)) == NULL) {
		mf_dprintf("gethostbyname failure - host not found\n\r");
		return(-1);
	}
	/* make sure address is filled in structure */
	if ((pptr = (struct in_addr *)hp->h_addr) == NULL) {
		mf_dprintf("gethostbyname failure - no address\n\r");
		return(-1);
	}
	/* set up socket */
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		mf_dprintf("socket failure %d\n\r", sockfd);
		printf("%s\n\r", strerror( errno ));
		return( -1 );
	}
	/* set up server address */
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(sock);
	memcpy(&servaddr.sin_addr, pptr, sizeof(struct in_addr));

	/* connect to server */
	if ((ok = connect(sockfd, (SA *) &servaddr, sizeof(servaddr))) < 0) {
		mf_dprintf("\n\rconnect failure %d\n\r", ok);
		close(sockfd);
		return(-1);
	}
	return(sockfd);
}

int tcp_connect_from(int locprt, char *host, int sock)
{
	int	                sockfd;
	SA_in	        servaddr;       /* unp.h, in.h */
	struct in_addr	*pptr;  	/* in.h        */
	HOSTENT     	*hp;   		/* netdb.h     */
	int ok;

	/* look up using DNS or dotted quad */
	if ((hp = gethostbyname(host)) == NULL) {
		mf_dprintf("gethostbyname failure - host not found\n\r");
		return(-1);
	}
	/* make sure address is filled in structure */
	if ((pptr = (struct in_addr *)hp->h_addr) == NULL) {
		mf_dprintf("gethostbyname failure - no address\n\r");
		return(-1);
	}
	/* set up socket */
	if ((sockfd = init_tcp(locprt)) < 0) {
		mf_dprintf("socket failure %d\n\r", sockfd);
		printf("%s\n\r", strerror( errno ));
		return( -1 );
	}
	/* set up server address */
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(sock);
	memcpy(&servaddr.sin_addr, pptr, sizeof(struct in_addr));

	/* connect to server */
	if ((ok = connect(sockfd, (SA *) &servaddr, sizeof(servaddr))) < 0) {
		mf_dprintf("\n\rconnect failure %d\n\r", ok);
		close(sockfd);
		return(-1);
	}
	return(sockfd);
}

int udp_connect_from(int locprt, char *host, int sock)
{
	int	                sockfd;
	SA_in	        servaddr;       /* unp.h, in.h */
	struct in_addr	*pptr;  	/* in.h        */
	HOSTENT     	*hp;   		/* netdb.h     */
	int ok;

	/* look up using DNS or dotted quad */
	if ((hp = gethostbyname(host)) == NULL) {
		mf_dprintf("gethostbyname failure - host not found\n\r");
		return(-1);
	}
	/* make sure address is filled in structure */
	if ((pptr = (struct in_addr *)hp->h_addr) == NULL) {
		mf_dprintf("gethostbyname failure - no address\n\r");
		return(-1);
	}
	/* set up socket */
	if ((sockfd = init_udp(locprt)) < 0) {
		mf_dprintf("socket failure %d\n\r", sockfd);
		printf("%s\n\r", strerror( errno ));
		return( -1 );
	}
	/* set up server address */
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(sock);
	memcpy(&servaddr.sin_addr, pptr, sizeof(struct in_addr));

	/* connect to server */
	if ((ok = connect(sockfd, (SA *) &servaddr, sizeof(servaddr))) < 0) {
		mf_dprintf("\n\rconnect failure %d\n\r", ok);
		close(sockfd);
		return(-1);
	}
	return(sockfd);
}



/* like fprintf but using a filedescriptor */
int fdprintf(int fd, const char *fmt, ...)
{
	char buf[512];
	va_list args;

	if (fd < 0)
	return -1;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	return (write(fd, buf, strlen(buf)));
}

/* like fprintf but using a socketdescriptor */
int sdprintf(int fd, const char *fmt, ...)
{
	char buf[512];
	va_list args;

	if (fd < 0)
	return -1;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	return (send(fd, buf, strlen(buf), 0));
}

int udp_send_file(char *fname, char *ipaddr, int portto, int portfrom)
{

	int udpsock, fd, udplen;
	char buff[250];
	struct in_addr *udps;
	SA_in udpsd;

	struct hostent			*hp;

	if((hp=gethostbyname(ipaddr))==NULL){
		mf_dprintf("could not find host name\n\r");
		return -1;
	}
	if((udps=(struct in_addr *)hp->h_addr)==NULL){
		mf_dprintf("get hostname failure, no address\n\r");
		return -1;
	}

	if((fd=open(fname, O_RDWR))<0){
		mf_dprintf("could not open file %s\n\r", fname);
		return (fd);
	}
	if((udpsock=init_udp(portfrom))<0){
		mf_dprintf("could not open UDP socket\n\r");
		return (udpsock);
	}

	bzero(&udpsd, sizeof(udpsd));
	udpsd.sin_family=AF_INET;
	udpsd.sin_port=htons(portto);
	memcpy(&udpsd.sin_addr, udps, sizeof(struct in_addr));
	udplen=sizeof(udpsd);
	while(read(fd, buff, sizeof(buff))>0){
		sendto(udpsock, buff, strlen(buff), 0, (SA *)&udpsd, udplen);
	  bzero(buff, sizeof(buff));
	}
	close(fd);
	close(udpsock);
	return 0;
}

int tcp_send_file(char *filed, char *ipaddr, int port)
{
	int tcpsock, fd;
	char buff[16];
	struct in_addr *tcps;
	struct hostent			*hp;

	if((hp=gethostbyname(ipaddr))==NULL){
		mf_dprintf("could not find host name\n\r");
		return -1;
	}
	if((tcps=(struct in_addr *)hp->h_addr)==NULL){
		mf_dprintf("get hostname failure, no address\n\r");
		return -1;
	}

	if((fd=open(filed, O_RDWR))<0){
		mf_dprintf("could not open file %s\n\r", filed);
		return (fd);
	}
	if((tcpsock=tcp_connect(ipaddr, port))<0){
		mf_dprintf("could not connect socket\n\r");
		return (tcpsock);
	}

	while(read(fd, buff, sizeof(buff))>0){
		fdprintf(tcpsock, "%s", buff);
		bzero(buff, sizeof(buff));
	}
	close(fd);
	close(tcpsock);
	return 0;
}

int mf_tcp_server(int locprt, int (*callback)())
{
	int ret;
	int serv;
	int cli;
	int ok;

	serv = init_tcp( locprt );
	if( serv < 0 ){
	  printf("could not open serv socket\n\r");
	}
	//fcntl(serv, F_SETFL, O_NONBLOCK);
	listen(serv, 2);

	cli = accept(serv, NULL, NULL);
	if( cli > 0 ){
	  ok = callback ();
	}
	close(cli);
	close(serv);

	return 0;
}

int mf_dprintf(const char *fmt, ...)
{
#ifdef MFDEBUG
  char msg[512];
  va_list args;

  va_start(args, fmt);
  vsprintf(msg, fmt, args);
  va_end(args);

  return  printf("%s", msg);
#endif
return 0;
}


#ifdef NEED_MF_CRC
static void crc32_init(void)                /* build the crc table */
{
	unsigned long crc, poly;
	int i, j;

	poly = 0xEDB88320L;
	for (i = 0; i < 256; i++)
  {
  crc = i;
  for (j = 8; j > 0; j--)
  {
  	if (crc & 1)
      crc = (crc >> 1) ^ poly;
  	else
      crc >>= 1;
  }
  crc_table[i] = crc;
  }
  inited=1;
}

unsigned long crc32(const char *str)    /* calculate the crc value */
{
  register unsigned long crc;
  unsigned int c,i;

  if (!inited)  crc32_init();

  i=0;
  crc = 0xFFFFFFFF;
  while ((c=(unsigned int)str[i++])!=0)
  crc = ((crc>>8) & 0x00FFFFFF) ^ crc_table[ (crc^c) & 0xFF ];

  return( crc^0xFFFFFFFF );
}

#endif
