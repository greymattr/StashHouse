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

#ifndef __AT_PARSE_H__
#define __AT_PARSE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for read();
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define QUIET_ON					0
#define QUIET_OFF					1

#define VERBOSE_NUMERIC		0
#define VERBOSE_ENGLISH		1

#define EOL_NONE					0
#define EOL_CR						1
#define EOL_LF            2
#define EOL_CRLF					3
#define EOL_CRNULL				4
#define EOL_LFNULL				5
#define EOL_CUSTOM				6

#define CFG_MODE_INIT			0
#define CFG_MODE_ADD			1

#define RES_OK						0
#define RES_CONNECT				1
#define RES_RING					2
#define RES_NO_CARRIER		3
#define RES_ERROR					4
#define RES_NO_DIALTONE		6
#define RES_BUSY					7
#define RES_NO_ANSWER			8
#define RES_EXIT					-1

#define MAX_BUF_SIZE			512

#define MAX_INFO_STR_LEN	64
#define MAC_ADDR_LEN			6
#define MIN_INFO_STR_LEN	8

struct dev_info {
	unsigned char name[ MAX_INFO_STR_LEN ];
	unsigned char serial_no[ MAX_INFO_STR_LEN ];
	unsigned char model_no[ MAX_INFO_STR_LEN / 2 ];
	unsigned char fw_version[ MIN_INFO_STR_LEN ];
	unsigned char hw_version[ MIN_INFO_STR_LEN ];
	unsigned char hw_revision[ MAX_INFO_STR_LEN ];
	unsigned char mac_addr[ MAC_ADDR_LEN ];
	unsigned char c_store[ MAX_BUF_SIZE ];
};

struct ctrl_cfg {
	unsigned int quiet_mode;		// 0 = do not display result codes, 1 = display result codes
	unsigned int verbose_mode;  // 0 = numeric result codes, 1 = english result codes
	unsigned int baud_rate;  		// serial port baud rate
	unsigned int eol_code;   		// end of line type, CR, LF, CR/LF, CR/NULL, LF/NULL
	unsigned char lf_char;			// line feed character value
	unsigned char cr_char;			// carriage return character value
	unsigned char esc_char;			// esc character value
	unsigned char backsp_char;  // back space character value
	unsigned char eol_custom[4];// custom EOL string
};

//extern unsigned int kv_count;

struct kv_pair {
	char *key;
	char *value;
};

struct kv_pair_list {
	struct kv_pair kv;
	struct kv_pair_list *next;
};

struct lat_cmd {
	const char *name;
	const char *help;
	void ( *callback )();
};


int fdprintf( int fd, const char *fmt, ... );
int ctrl_cfg_init( struct ctrl_cfg *c );
void print_english_result( int res );
void print_result( struct ctrl_cfg *c, int res );
int read_cmd_line( int fd, char *buf, unsigned int buf_len );
int parse_at_command ( struct ctrl_cfg *c, struct kv_pair_list **l , char *buf );
struct kv_pair_list *init_items( void );
void print_kv_pair_lists( struct kv_pair_list *l );
int kv_pair_lists_count( void );
int add_kv_pair( struct kv_pair_list *l, char *key, char *value );
//int del_kv_pair( struct kv_pair_list **l, char *key );
int del_kv_pair( struct kv_pair_list **l, char *key );
void del_all_kv_pairs( struct kv_pair_list **l );
char *find_key_value( struct kv_pair_list *l, char *key );
char *parse_key( char *str );
char *parse_value( char *str );
int write_config_file( struct kv_pair_list *l, char *cfg, int mode );
int parse_config_file( struct kv_pair_list *l, char *cfg, int mode );
int tokpos( char *str, int len, char tok );
//int process_cmd_buf( char *cmdbuf, int len );
int process_cmd_buf( char *cmdbuf, int cmdlen, struct lat_cmd *c );


#endif