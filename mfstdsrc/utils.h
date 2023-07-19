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

// The following macros are useful for making values from smaller smarts, such as making
// a four-byte integer from the four individual bytes.

#ifndef MAKELONG // makes a long from 4 bytes
#define MAKELONG(b0, b1, b2, b3)  ((((uint32_t)(b0)) << 24) | (((uint32_t)(b1)) << 16) | (((uint32_t)(b2)) << 8) | ((uint32_t)(b3)))
#endif 

#ifndef MAKELONG2 // makes a long from 4 bytes
#define MAKELONG2(a, o)  ((((uint32_t)(a[o])) << 24) | (((uint32_t)(a[o+1])) << 16) | (((uint32_t)(a[o+2])) << 8) | ((uint32_t)(a[o+3])))
#endif 

#ifndef MAKEFLOAT // makes a float from 4 bytes
#define MAKEFLOAT(pf, b0, b1, b2, b3)  ((uint8_t*)pf)[0] = b3 ; ((uint8_t*)pf)[1] = b2 ; ((uint8_t*)pf)[2] = b1 ; ((uint8_t*)pf)[3] = b0 ;
#endif

#ifndef MAKEWORD // makes a word from 2 bytes
#define MAKEWORD(b0, b1)  (((uint16_t)((uint8_t)(b0))) << 8) | (uint16_t)(((uint8_t)(b1)))
#endif 

#ifndef MAKEWORD2 // makes a word from 2 bytes
#define MAKEWORD2(a, o)  (((uint16_t)((uint8_t)(a[o]))) << 8) | (uint16_t)(((uint8_t)(a[o+1])))
#endif 

#ifndef MAKEBYTE2 // makes a byte from 1 byte, for consistency with other macros
#define MAKEBYTE2(a, o)  a[o]
#endif 

#ifndef MAKEDWORD // makes a dwords from 2 words
#define MAKEDWORD(w0, w1) (((uint32_t)((uint16_t)(w0))) << 16) | (uint32_t)(((uint16_t)(w1)))
#endif

// The following macros are useful for breaking up a value into smaller pieces, such as 
// getting a third byte out of a 32-bit integer

#ifndef LOWORD // gets the low word from a dword
#define LOWORD(l) ((uint16_t)(l))
#endif

#ifndef HIWORD // gets the high word from a dword
#define HIWORD(l) ((uint16_t)(((uint32_t)(l) >> 16) & 0xFFFF))
#endif

#ifndef LOBYTE // gets the low byte from a word
#define LOBYTE(w) ((uint8_t)(w))
#endif

#ifndef HIBYTE // gets the high byte from a word
#define HIBYTE(w) ((uint8_t)(((uint16_t)(w) >> 8) & 0xFF))
#endif

#ifndef BYTE0 // gets the first byte from a dword
#define BYTE0(l) ((uint8_t)(((uint32_t)(l) >> 24) & 0xFF))
#endif

#ifndef BYTE1 // gets the second byte from a dword
#define BYTE1(l) ((uint8_t)(((uint32_t)(l) >> 16) & 0xFF))
#endif

#ifndef BYTE2 // gets the third byte from a dword
#define BYTE2(l) ((uint8_t)(((uint32_t)(l) >> 8) & 0xFF))
#endif

#ifndef BYTE3 // gets the fourth byte from a dword
#define BYTE3(l) ((uint8_t)(l))
#endif

#ifndef SWAP_BYTES_LONG // swap the bytes in a long
#define SWAP_BYTES_LONG(l)  MAKELONG(BYTE3(l), BYTE2(l), BYTE1(l), BYTE0(l))
#endif

#define array_length( a ) (sizeof(a)/sizeof(a[0]))


void cmd_wrapper( char *res, int res_len, char *cmd );
int read_word( int fd, char *buf, int maxlen );
int verify_addr( char *s );
int fdprintf( int fd, const char *fmt, ... );
int fdreadline( int fd, char *buf, int len );
int fdreadline_t( int fd, char *buf, int len, int maxtime );
int search_string( char *key, char *buf );
int suspect( int fd, char *key, int timeout );
int fdopenexec( char *executable );
char *recv_all(int sockfd);
char *read_all( int fd, unsigned int secs );
char *get_cmd_output( char *cmd, unsigned int timeout );
void hexdump(unsigned char *data, size_t length);

#endif
