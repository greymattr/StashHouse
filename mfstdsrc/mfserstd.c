/**********************************************************************
 *                  mfserstd.c  -  description:
 *                  ---------------------------
 *  begin           : Fri Aug 2 2002
 *  copyright       : (C)2002 by Matthew Fatheree of Lantronix
 *  email           : Matthewf@lantronix.com
 *
 **********************************************************************/

/**********************************************************************
 *
 *  Copyright 2001, Lantronix
 *
 *  All rights reserved, The contents of this file may not be used
 *  and/or copied in any form or by any means without the written
 *  permission of Lantronix.
 *
 **********************************************************************/
#include "mfserstd.h"


int init_ser( int p )
{
  int sd;

  switch( p ) {
  case 1:
    sd = open( "/dev/ttyE0", O_RDWR  );
    break;
  case 2:
    sd = open( "/dev/ttyE1", O_RDWR  );
    break;
  case 3:
    sd = open( "/dev/ttyE2", O_RDWR  );
    break;
  case 4:
    sd = open( "/dev/ttyE3", O_RDWR  );
    break;
  default:
    //  add debug here
    printf( "serial port number %d not supported\n\r", p );
    return -1;
  }

  if( sd < 0 ) {
    printf( "could not open serial port ( %d )\n\r", sd );
  } else {
    /*  set the port to blocking */
    fcntl( sd, F_SETFL, 0 );
  }
  return sd;
}

int init_file( char *name )
{
  int sd;

  sd = open( name, O_RDWR | O_CREAT | O_TRUNC, 0667  );
  if( sd < 0 ) {
    printf( "could not open file %s (%d)\n\r", name, sd );
  }

  return sd;
}

void non_block( int d )
{
  fcntl( d, F_SETFL, FNDELAY );
  return ;
}

void block( int d )
{
  fcntl( d, F_SETFL, 0 );
  return;
}

void set_baud( int sd, int baud )
{
  struct termios tmp;

  tcgetattr( sd, &tmp );

  cfsetispeed( &tmp, baud );
  cfsetospeed( &tmp, baud );

  tcsetattr( sd, TCSAFLUSH, &tmp );
  return;
}

void set_parity( int sd, int p )
{
  struct termios tmp;

  tcgetattr( sd, &tmp );
  switch( p ) {
  case MF_NOPARITY:
    tmp.c_cflag &= ~PARENB;
    break;
  case MF_EVPARITY:
    tmp.c_cflag |= PARENB;
    tmp.c_cflag &= ~PARODD;
    break;
  case MF_ODPARITY:
    tmp.c_cflag |= PARENB;
    tmp.c_cflag |= PARODD;
    break;
  }
  tcsetattr( sd, TCSAFLUSH, &tmp );
  return;
}


void set_maxread( int sd, int num )
{
  struct termios tmp;

  tcgetattr( sd, &tmp );
  tmp.c_cc[VTIME]    = 0;   /* inter-character timer unused */
  tmp.c_cc[VMIN]     = num;   /* blocking read until 5 chars received */

  tcflush( sd, TCIFLUSH );
  tcsetattr( sd,TCSANOW,&tmp );
  return;
}

void set_max_char_delay( int sd, int num )
{
  struct termios tmp;

  tcgetattr( sd, &tmp );
  tmp.c_cc[VTIME]    = num;   /* inter-character timer unused */
  tmp.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

  tcflush( sd, TCIFLUSH );
  tcsetattr( sd,TCSANOW,&tmp );
  return;
}


void set_charsize( int sd, int size )
{
  struct termios tmp;

  tcgetattr( sd, &tmp );
  switch( size ) {

  case MF_8BIT:
    tmp.c_cflag |= CS8;
    break;
  case MF_7BIT:
    tmp.c_cflag |= CS7;
    break;
  case MF_6BIT:
    tmp.c_cflag |= CS6;
    break;
  case MF_5BIT:
    tmp.c_cflag |= CS5;
    break;
  }
  tcsetattr( sd, TCSAFLUSH, &tmp );
  return;
}


void set_flowctrl( int sd, int flow )
{
  struct termios tmp;

  tcgetattr( sd, &tmp );
  switch( flow ) {
  case MF_NONE:
    tmp.c_cflag &= ~CRTSCTS;
    tmp.c_iflag &= ~( IXON | IXOFF | IXANY );
    break;
  case MF_HARD:
    tmp.c_cflag |= CRTSCTS;
    break;
  case MF_SOFT:
    tmp.c_iflag |= ( IXON | IXOFF | IXANY );
    break;
  }
  tcsetattr( sd, TCSAFLUSH, &tmp );
  return;
}


int init_serial( char *p )
{
  int sd;
  struct termios options;


  sd = open( p, O_RDWR | O_NDELAY );


  /* get the current options */
  tcgetattr( sd, &options );

  /* set raw input, 1 second timeout */
  options.c_cflag     |= ( CLOCAL | CREAD );
  options.c_lflag     &= ~( ICANON | ECHO | ECHOE | ISIG );
  options.c_oflag     &= ~OPOST;
  options.c_cc[VMIN]  = 0;
  options.c_cc[VTIME] = 10;

  /* set the options */
  tcsetattr( sd, TCSANOW, &options );

#ifdef _DEBUG
  if( sd < 0 ) {

    printf( "could not open serial port %s ( %d )\n\r",p, sd );
  }
#endif
  return sd;
}


int cd_hi( int sd )
{
  int status;

  ioctl( sd, TIOCMGET, &status );
  status &= ~TIOCM_DTR;

  return ( ioctl( sd, TIOCMSET, &status ) );

}

int rts_low( int sd )
{
  int status;

  ioctl( sd, TIOCMGET, &status );
  status =+ TIOCM_DTR;

  return ( ioctl( sd, TIOCMSET, &status ) );

}

int cts_low( int sd )
{
  int status;

  ioctl( sd, TIOCMGET, &status );
  status += TIOCM_DSR;
  //status &= ~TIOCM_RTS;
  return ( ioctl( sd, TIOCMSET, &status ) );

}


int cts_hi( int sd )
{
  int status;

  ioctl( sd, TIOCMGET, &status );
  status +=TIOCM_RTS;

  return ( ioctl( sd, TIOCMSET, &status ) );

}

int cd_low( int sd )
{
  int status;

  ioctl( sd, TIOCMGET, &status );
  status &= ~TIOCM_RTS;

  return ( ioctl( sd, TIOCMSET, &status ) );

}

unsigned int get_ser_stat( int ser )
{

  int st = 0;
  int status;

  ioctl( ser, TIOCMGET, &status );

  if( status & TIOCM_CTS ) {
    st += SIG_CTS;
  }
  if( status & TIOCM_DTR ) {
    st += SIG_DTR;
  }
  if( status & TIOCM_LE ) {
    st += SIG_DTS;
  }
  if( status & TIOCM_RTS ) {
    st += SIG_RTS;
  }
  if( status & TIOCM_CD ) {
    st += SIG_CD;
  }

  return st;
}


int mwrite( int fd, char *buf, int size )
{
  int mbo = 0;
  int i;

  //fcntl(fd, F_SETFL, 0);
  while( mbo < size ) {
    i = write( fd, buf + mbo, size - mbo );
    if( i > 0 ) {
      mbo += i;
    }
    //if( i < 0 ) return i;
  }
  //fcntl(fd, F_SETFL, O_NONBLOCK);

  return mbo;
}









































































































