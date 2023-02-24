#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#define BUF_SIZE 1024
#define PKT_SIZE ( BUF_SIZE * 4 )

/**
 * [find_and_replace_text - finds and replaces a block of text in a given buffer ]
 * @param  sbuf   [ source buffer ]
 * @param  buflen [ length of source buffer ]
 * @param  from   [ text that will be replaced ]
 * @param  to     [ text that be used for the replacement ]
 * @return        [ 0 ] TODO: add return erros codes
 */
int find_and_replace_text( char *sbuf, int buflen, char *from, char *to )
{
  char *buf1;
  char *startbuf;
  char *cstart;
  int offs = 0;
  int roffs = 0;
  int len = 0;

  startbuf = sbuf;
  buf1 = malloc( sizeof( char ) * buflen );
  if( buf1 == NULL ) {
    return -1;
  }

  memset( buf1, 0, buflen );

  cstart = strstr( startbuf, from );
  while( cstart != NULL ) {
    len = cstart - startbuf;
    roffs = cstart - sbuf;
    printf( "\n\rfound %s @ %d\n\r", from, roffs );
    memcpy( buf1 + offs, startbuf, len );
    offs += len;
    memcpy( buf1 + offs, to, strlen( to ) );
    offs += strlen( to );
    roffs += strlen( from );
    startbuf = sbuf + roffs;
    cstart = strstr( startbuf, from );
  }
  if( ( offs + strlen( startbuf ) ) > buflen ) {
    printf( "error - replace would overflow\n\r" );
  } else {
    //copy the rest of the buffer
    memcpy( buf1 + offs, sbuf + roffs, strlen( sbuf + roffs ) );
    memset( sbuf, 0, buflen );
    memcpy( sbuf, buf1, strlen( buf1 ) );
  }
  free( buf1 );
  return 0;
}
