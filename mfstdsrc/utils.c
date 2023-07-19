/**************************************************************
 *
 * MFUTILS.C : mfutils.c - standard code utilites I use while
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

#include "mfutils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>
#include <pty.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/ioctl.h>

void cmd_wrapper( char *res, int res_len, char *cmd )
{
  FILE *f;
  int ok=0;

  f = popen( cmd, "r" );
  if ( f != NULL ) {
    memset( res, 0, res_len );
    ok = fread( res, 1, res_len-1, f );
    if( res[ok-1] == 0x0a ) {
      res[ok-1]=0;
    }
    pclose( f );
  }
  return ;
}


int read_word( int fd, char *buf, int maxlen )
{
  int i=0;
  int ok=1;
  int rd;
  char ch;
  int end = 0;

  while( ok ) {
    rd = read( fd, &ch, 1 );
    if( rd > 0 ) {
      switch( ch ) {
      case ' ':
      case 0x0d:
      case 0x0a:
        if( i > 0 ) {
          ok = 0;
          buf[i] = 0;
        }
        break;
      default:
        buf[i] = ch;
        i++;
      }
    } else {
      ok = 0;
    }
  }
  return i;
}

/* verifys a mac address array */
int verify_addr( char *s )
{
  int i;
  for( i = 0; i < 17; i++ ) {
    if( i % 3 != 2 && !isxdigit( s[i] ) ) {
      return 0;
    }
    if( i % 3 == 2 && s[i] != ':' ) {
      return 0;
    }
  }
  if( s[17] != '\0' ) {
    return 0;
  }
  return 1;
}

/* like fprintf but using a file descriptor */
int fdprintf( int fd, const char *fmt, ... )
{
  char buf[512];
  va_list args;
  if ( fd < 0 ) {
    return -1;
  }
  va_start( args, fmt );
  vsprintf( buf, fmt, args );
  va_end( args );
  return ( write( fd, buf, strlen( buf ) ) );
}

/* fdreadline - is used to read a line from a file descriptor
 * into buf, of max length len.  buf will be memset to 0 for
 * length len before use. fdrealine returns the length of the
 * data it read from the fd
 */
int fdreadline( int fd, char *buf, int len )
{
  int i=0;
  char ch;
  int ok;
  memset( buf, 0, len );
  while( i < len ) {
    if ( ( ok = read( fd, &ch, 1 ) ) == 1 ) {
      if( ( ch != '\n' ) && ( ch != '\r' ) ) {
        buf[i] = ch;
        i++;
      } else {
        return i;
      }
    } else {
      return ok;
    }
  }
  return i;
}

/* fdreadline_t - is like fdreadline but uses select as a timeout
 */
int fdreadline_t( int fd, char *buf, int len, int maxtime )
{
  fd_set set;
  struct timeval timeout;
  int rv;
  int ok = 0;
  int rlen = 0;

  while( 1 ) {
    FD_ZERO( &set );
    FD_SET( fd, &set );

    timeout.tv_sec = maxtime;
    timeout.tv_usec = 0;

    rv = select( fd + 1, &set, NULL, NULL, &timeout );
    if( rv == -1 ) {
      return rv;
    } else if ( rv == 0 ) {
      break;
    } else {
      ok = read( fd, buf+rlen, len-rlen );
      if( ok > 0 ) {
        rlen += ok;
      }
      if( rlen == len ) {
        break;
      }
    }
  }
  return rlen;
}

/* search_string - searches for string key, in buf
 * if string key exists in buf, it will return the
 * position in buf where string key starts.  else
 * returns 0;
 */
int search_string( char *key, char *buf )
{
  int ok = -1;
  char *tmp;
  if( ( tmp = strstr( buf, key ) ) != NULL ) {
    ok = ( tmp - buf );
  }
  return ok;
}

/* suspect - is kind of like expect, but less useful most of the time */
int suspect( int fd, char *key, int timeout )
{
  int timer = 0;
  char buf[2048];
  int ok;
  int len = 0;

  memset( buf, 0, sizeof( buf ) );
  fcntl( fd, F_SETFL, O_NONBLOCK );
  while ( 1 ) {
    ok = read ( fd, buf+len, sizeof( buf )-len );
    if( ok > 0 ) {
      if( ( len + ok ) < sizeof( buf ) ) {
        len += ok;
      } else {
        ok = SUSPECT_ERR_MAX_READ;
        break;
      }
      if( ( ok = search_string( key, buf ) ) >= 0 ) {
        break;
      }
    }
    sleep( 1 );
    timer++;
    //printf("timer = %d\n", timer);
    if( timer == timeout ) {
      ok = SUSPECT_ERR_TIMEOUT;
      break;
    }
  }
  //printf("\n\r->%s<-\n\r", buf);
  return ok;
}


/* I wanted a call like 'popen' that would allow for simultanous
 * reading and writting.  but much to my surprize, one does not exist,
 * and pipes by their nature are uni directional, so I searched around and
 * came up with this forkpty solution.  forkpty will create a parent
 * and a child process in a psuedo tty and assign a freakin' file descriptor
 * ( not file pointer ) to that pty. from there you can use system, or
 * an execp call from the parent process to start up another program
 * and then interact with it via read/writes to the file descriptor.
 * HAPPY DAY!. Although a convoluted solution.
 */
int fdopenexec( char *executable )
{
  int ok = 0;
  int master;
  struct winsize win = { /* this structure is needed for forkpty */
    .ws_col = 80, .ws_row = 24,
    .ws_xpixel = 480, .ws_ypixel = 192,
  };
  pid_t child;
  child = forkpty( &master, NULL, NULL, &win );
  if ( child == -1 ) {
    perror( "forkpty failed" );
    return 0;
  }
  if ( child == 0 ) {
    /* start the executable that we need to control from the parent process */
    system( executable );
    exit( EX_OSERR );
  } else {
    /* this is the child process, attached to a psuedo tty,
     * running the exernal executable, which should allow low-level
     * read/write calls - YEA LINUX! ( fread and fwrite are for newbs )
     */
    ok = dup( master );
  }
  return ok;
}


char *recv_all(int sockfd) {
  // Create a buffer to store received data
  char buffer[1024];
  memset(buffer, 0, sizeof(buffer));
  
  // Create a string to store received data
  char* data = NULL;
  size_t data_len = 0;
  
  // Use select to wait for data to be available
  fd_set read_fds;
  FD_ZERO(&read_fds);
  FD_SET(sockfd, &read_fds);
  struct timeval tv;
  tv.tv_sec = 10; // timeout in 10 seconds
  tv.tv_usec = 0;
  int select_ret = select(sockfd + 1, &read_fds, NULL, NULL, &tv);
  if (select_ret < 0) {
    perror("select error");
    return NULL;
  } else if (select_ret == 0) {
    printf("timeout waiting for data\n");
    return NULL;
  }
  
  // Loop until there is no more data to receive
  while (1) {
    // Receive data from the socket
    ssize_t recv_ret = recv(sockfd, buffer, sizeof(buffer), 0);
    if (recv_ret < 0) {
      perror("recv error");
      free(data);
      return NULL;
    } else if (recv_ret == 0) {
      break; // no more data to receive
    }
    
    // Resize the data string to accommodate the new data
    data_len += recv_ret;
    data = realloc(data, data_len);
    if (data == NULL) {
      perror("realloc error");
      free(data);
      return NULL;
    }
    
    // Append the received data to the data string
    memcpy(data + data_len - recv_ret, buffer, recv_ret);
  }
  
  // Null-terminate the data string
  data = realloc(data, data_len + 1);
  if (data == NULL) {
    perror("realloc error");
    free(data);
    return NULL;
  }
  data[data_len] = '\0';
  
  return data;
}

char *read_all( int fd, unsigned int secs )
{
  ssize_t read_ret = 0;
  // Create a buffer to store received data
  char buffer[1024];
  
  memset( buffer, 0, sizeof( buffer ) );
  
  // Create a string to store received data
  char* data = NULL;
  size_t data_len = 0;
  
  // Use select to wait for data to be available
  fd_set read_fds;
  FD_ZERO( &read_fds );
  FD_SET( fd, &read_fds );
  struct timeval tv;
  tv.tv_sec = secs; // timeout
  tv.tv_usec = 0;
  
  
  int select_ret = select( fd + 1, &read_fds, NULL, NULL, &tv );
  if ( select_ret < 0 ) {
    perror( "select error" );
    return NULL;
  } else if ( select_ret == 0 ) {
    printf( "timeout waiting for data\n" );
    return NULL;
  }
  
  // Loop until there is no more data to receive
  while ( 1 ) {
    // Receive data from the socket
    read_ret = read( fd, buffer, sizeof( buffer ) );
    if ( read_ret < 0 ) {
      perror( "recv error" );
      free( data );
      return NULL;
    } else if ( read_ret == 0 ) {
      break; // no more data to receive
    }
    
    // Resize the data string to accommodate the new data
    data_len += read_ret;
    data = realloc( data, data_len );
    if ( data == NULL ) {
      perror( "realloc error" );
      free( data );
      return NULL;
    }
    
    // Append the received data to the data string
    memcpy( data + data_len - read_ret, buffer, read_ret );
  }
  
  // Null-terminate the data string
  data = realloc( data, data_len + 1 );
  if ( data == NULL ) {
    perror( "realloc error" );
    free( data );
    return NULL;
  }
  data[data_len] = '\0';
  
  return data;
}


char *get_cmd_output( char *cmd, unsigned int timeout )
{
  FILE *f = NULL;
  int fd = 0;
  char *output = NULL;
  
  f = popen( cmd, "r" );
  if ( f != NULL ) {
    fd = fileno( f );
    output = read_all(fd, timeout);
    pclose( f );
  }
  return output;
}

void hexdump(unsigned char *data, size_t length) 
{
  int column = 0;
  char ascii_buf[17] = { 0x00 };
  int a_count = 0;
  int left = 0;
  
  printf("\n\r========================== HEX DUMP =========================================\n\r");
  for (size_t i = 0; i < length; i++) {
    if (column == 0) {
      printf("%04x: ", (unsigned int)i);
    }
    printf("%02x ", data[i]);
    if( isprint(data[i]) ){
      ascii_buf[column] = data[i];
    } else {
      ascii_buf[column] = '.';
    }
    column++;
    if (column == 16) {
      printf("\t|  %s\n", ascii_buf);
      memset(ascii_buf, 0, sizeof(ascii_buf));
      column = 0;
    }
  }
  if (column != 0) {
    left = 16 - column;
    for(int i = 0; i <left; i++ ){
      printf("   "); 
    }
    printf("\t|  %s\n", ascii_buf);
    printf("\n");
  }
  printf("\n\r=============================================================================\n\r");
}
