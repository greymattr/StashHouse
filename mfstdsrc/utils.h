/**************************************************************
 *
 * MFUTILS.H : mfutils.h - standard code utilites I use while
 *             programming
 *
 *
 **************************************************************
 *
 * REVISION HISTORY:
 *
 * DATE                 .Comment                       .AUTHOR
 * ============================================================
 *
 * 12-04-2005           Initial revison  -           M.Fatheree
 *                  added read_word,
 **************************************************************/
#ifndef __MFUTILS_H__
#define __MFUTILS_H__


#include <stdio.h>
#include <stdlib.h>

void cmd_wrapper( char *res, int res_len, char *cmd );
int read_word( int fd, char *buf, int maxlen );
int verify_addr( char *s );
int fdprintf( int fd, const char *fmt, ... );
int fdreadline( int fd, char *buf, int len );
int fdreadline_t( int fd, char *buf, int len, int maxtime );
int search_string( char *key, char *buf );
int suspect( int fd, char *key, int timeout );
int fdopenexec( char *executable );
char* recv_all(int sockfd);
char *read_all( int fd, unsigned int secs );
char *get_cmd_output( char *cmd, unsigned int timeout );

#endif
