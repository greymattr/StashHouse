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


struct ctrl_cfg CFG;					// global control configuration
static struct kv_pair_list *llist;		// global key value pair list
static int loop = 0;					// global loop var for main

// call back function prototypes for lat_cmd table callbacks
void dump_av_pairs( void );
void count_av_pairs( void );
void exit_program( void );
void print_help( void );

struct lat_cmd cmd_table[] = {
	{"AT+", "AT+<Key>=<Value> add key value pair", NULL},
	{"AT-", "AT-<Key> remove key value pair by key", NULL},
	{"AT?", "AT?<Key> get key value", NULL},
	{"DUMP", "dump av pair list", dump_av_pairs},
	{"COUNT", "get number of av pairs in list", count_av_pairs },
	{"EXIT", "exit program", exit_program },
	{"HELP", "print command help", print_help},
	{NULL, NULL, NULL}
};



//int main( int argc, char **argv )
int main( void )
{
	int ok = 0;
	char buf[ MAX_BUF_SIZE ];

	ctrl_cfg_init( &CFG );
//	printf( "ctrl_cfg_init - done\n\r" );
	memset( buf, 0, sizeof( buf ) );

	llist = init_items();
	add_kv_pair( llist, "name", "matthew" );
	add_kv_pair( llist, "age", "40" );
	add_kv_pair( llist, "street", "dalton" );
	add_kv_pair( llist, "one", "1" );
	add_kv_pair( llist, "two", "2" );

//	printf("added default config items\n\r");

	while( loop == 0 ) {
		ok = read_cmd_line( STDIN_FILENO, buf, sizeof( buf ) );
		// process_cmd_buf will handle command in cmd_table
		if ( ! process_cmd_buf( buf, ok, cmd_table ) ) {
			if ( ok ) {
				// parse more complex AT style commands
				parse_at_command( &CFG, &llist, buf );
			}
		}
		memset( buf, 0, sizeof( buf ) );
	}
	if( llist != NULL ) {
		del_all_kv_pairs( &llist );
		free( llist );
	}
	printf( "exiting...\n\r" );
	return 0;
}

void dump_av_pairs( void )
{
	print_kv_pair_lists( llist );
	return;
}

void count_av_pairs( void )
{
	printf( "%d\n\r", kv_pair_lists_count() );
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
		printf( "\t%s:\t%s\n\r", cmd_table[i].name, cmd_table[i].help );
		i++;
	}
	return;
}
