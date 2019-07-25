/*
 * The MIT License (MIT)
 *
 Copyright (c) 2019 Matthew Fatheree greymattr(at)gmail.com

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "at_parse.h"


static struct ctrl_cfg CFG;									// global control configuration
static struct kv_pair_list *llist;		// global key value pair list
static int loop = 0;									// global loop var for main
//static int fd = STDIN_FILENO;					// cmd processing file descriptor

// call back function prototypes for lat_cmd table callbacks
void dump_av_pairs( void );
void count_av_pairs( void );
void exit_program( void );
void print_help( void );
void update_cfg( void );
void print_main_help( void );

struct lat_cmd cmd_table[] = {
	{"AT+", "AT+<Key>=<Value> add key value pair", NULL},
	{"AT-", "AT-<Key> remove key value pair by key", NULL},
	{"AT?", "AT?<Key> get key value", NULL},
	{"DUMP", "dump av pair list", dump_av_pairs},
	{"COUNT", "get number of av pairs in list", count_av_pairs },
	{"CFGUPDATE", "write out the configuration to a file", update_cfg},
	{"EXIT", "exit program", exit_program },
	{"HELP", "print command help", print_help},
	{NULL, NULL, NULL}
};



int main( int argc, char *argv[] )
{
	int ok = 0;
	char buf[ MAX_BUF_SIZE ];
	int opts;
	char device[ 64 ];
	unsigned int baud_rate;
	char kv_file[ 64 ];
	int verbose = 0;


	memset( buf, 0, sizeof( buf ) );
	memset( device, 0, sizeof( device ) );
	memset( kv_file, 0, sizeof( kv_file ) );
	/* options followed by ':'  will need to have arguments */
	/* -d = device
	 * -b = baud rate
	 * -f = key value file to load at start up
	 * -v verbose
	 * -h help
	 */
	while( ( opts = getopt( argc, argv, "d:b:f:vh" ) ) != -1 ) {
		switch( opts ) {
		case 'd':
			if( strlen( optarg ) < sizeof( device ) ) {
				memcpy( device, optarg, strlen( optarg ) );
			} else {
				printf( "Error device name too long\n\r%s\n\r", optarg );
				return -1;
			}
			break;
		case 'b':
			baud_rate = atoi( optarg );
			switch( atoi( optarg ) ) {
			case 300:
			case 600:
			case 1200:
			case 2400:
			case 4800:
			case 9600:
			case 14400:
			case 19200:
			case 28800:
			case 38400:
			case 56700:
			case 115200:
				baud_rate = atoi( optarg );
				break;
			default:
				printf( "Invalid baud rate %s\n\r", optarg );
				return -1;
				break;  // code never reaches here but keep for formatting
			}
			break;
		case 'f':
			if( strlen( optarg ) < sizeof( kv_file ) ) {
				memcpy( kv_file, optarg, strlen( optarg ) );
			} else {
				printf( "Error kv_file name too long\n\r%s\n\r", optarg );
				return -1;
			}
			break;
		case 'v':
			verbose = 1;
			break;
		case 'h':
			print_main_help();
			return 0;
			break;
		}
	}/*  end of while statement  */

	ctrl_cfg_init( &CFG );
	if( baud_rate != 0 ) {
		CFG.baud_rate = baud_rate;
	}
	if( device[0] != '\0' ) {
		CFG.iofd = open( device, O_RDWR  );
		if( CFG.iofd < 0 ) {
			printf( "could not open device %s (%d)\n\r", device, CFG.iofd );
			return -1;
		} else {
			set_baud( CFG.iofd, baud_rate );
		}
		//fcntl(CFG.iofd, F_SETFL, O_NONBLOCK);
	}
	llist = init_items();
	if( kv_file[0] != '\0' ) {
		if( verbose == 1 ) {
			printf( "populating kv list with items from %s\n\r", kv_file );
		}
		parse_kv_file( llist, kv_file, CFG_MODE_ADD );
	}

	fdprintf( CFG.iofd, "Initializing...\n\r" );

	memset( buf, 0, sizeof( buf ) );
	while( loop == 0 ) {
		//ok = read_cmd_line( CFG.iofd, buf, sizeof( buf ) );
		ok = fdreadline_t( CFG.iofd, buf, sizeof(buf), 0);
		if( ok > 0 ){
			// printf("read %d bytes\n\r[ %s ]\n\r", ok, buf);
			// process_cmd_buf will handle command in cmd_table with callback functions
			if ( ! process_cmd_buf( buf, ok, cmd_table ) ) {
				if ( ok ) {
					// parse more complex AT style commands, or those in cmd_table without callback functions
					parse_at_command( &CFG, &llist, buf );
				}
			}
			memset( buf, 0, sizeof( buf ) );
		}
	}
	if( llist != NULL ) {
		del_all_kv_pairs( &llist );
		free( llist );
	}
	fdprintf( CFG.iofd, "exiting...\n\r" );
	if( device[0] != '\0' ) {
		close( CFG.iofd );
	}
	return 0;
}

void dump_av_pairs( void )
{
	print_kv_pair_lists( &CFG, llist );
	return;
}

void count_av_pairs( void )
{
	fdprintf( CFG.iofd, "%d\n\r", kv_pair_lists_count() );
	return;
}

void exit_program( void )
{
	loop = 1;
	return;
}

void print_help( void )
{
	unsigned int i = 0;
	while ( cmd_table[i].name != NULL ) {
		fdprintf( CFG.iofd, "\t%s:\t%s\n\r", cmd_table[i].name, cmd_table[i].help );
		i++;
	}
	return;
}

void update_cfg ( void )
{
	write_kv_file( llist, "linksysat.cfg", CFG_MODE_INIT );
	return;
}

void print_main_help( void )
{
	printf( "HELP!" );
	return ;
}

