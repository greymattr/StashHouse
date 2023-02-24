/**********************************************************************
 *                  mfserstd.h  -  description:
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
#ifndef __MFSERSTD_H__
#define __MFSERSTD_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <time.h>

//#include <asm/ioctls.h>
#include <sys/ioctl.h>


/*  defines for parity settings  */
#define MF_NOPARITY	0
#define MF_EVPARITY 1
#define MF_ODPARITY 2

/*  defines for character size settings */
#define MF_8BIT		0
#define MF_7BIT		1
#define MF_6BIT		2
#define MF_5BIT		3

/*  defines for flow control  */
#define MF_NONE		0
#define MF_HARD		1
#define MF_SOFT		2

#define SIG_CTS         1
#define SIG_RTS         2
#define SIG_DTS         4
#define SIG_DTR         8
#define SIG_CD         16


extern int init_ser( int p );
extern int init_file( char *name );
extern void non_block( int d );
extern void block( int d );
extern int init_serial( char *p );

extern void set_baud( int sd, int baud );
extern void set_parity( int sd, int p );
extern void set_flowctrl( int sd, int flow );
extern void set_charsize( int sd, int size );

extern int cd_hi( int sd );
extern int cd_low( int sd );
extern int cts_low( int sd );
extern int rts_low( int sd );
extern int cts_hi( int sd );
extern unsigned int get_ser_stat( int ser );
extern int mwrite( int fd, char *buf, int size );
extern void set_max_char_delay( int sd, int num );
extern void set_maxread( int sd, int num );


#endif
