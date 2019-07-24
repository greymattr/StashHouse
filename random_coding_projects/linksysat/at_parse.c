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

#include "at_parse.h"

static unsigned int kv_count;

/* like fprintf but using a filedescriptor */
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

int ctrl_cfg_init( struct ctrl_cfg *c )
{
	memset( c, 0, sizeof( struct ctrl_cfg ) );
	// set some default values for the session
	c->quiet_mode = QUIET_OFF;
	c->verbose_mode = VERBOSE_ENGLISH;
	c->eol_code = EOL_CRLF;
	c->lf_char = 0x0a;
	c->cr_char = 0x0d;
	c->esc_char = 0x1b;
	c->backsp_char = 0x08;
	c->iofd = STDIN_FILENO;
	return 0;
}

void print_english_result( struct ctrl_cfg *c, int res )
{
	switch( res ) {
	case RES_OK:
		//printf( "OK" );
		fdprintf( c->iofd, "OK" );
		break;
	case RES_CONNECT:
		fdprintf( c->iofd, "CONNECT" );
		break;
	case RES_RING:
		fdprintf( c->iofd, "RING" );
		break;
	case RES_NO_CARRIER:
		fdprintf( c->iofd, "NO CARRIER" );
		break;
	case RES_ERROR:
		fdprintf( c->iofd, "ERROR" );
		break;
	case RES_NO_DIALTONE:
		fdprintf( c->iofd, "NO DAILTONE" );
		break;
	case RES_BUSY:
		fdprintf( c->iofd, "BUSY" );
		break;
	case RES_NO_ANSWER:
		fdprintf( c->iofd, "NO ANSWER" );
		break;
	case 5:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 22:
	case 23:
	case 24:
	case 25:
	case 28:
		fdprintf( c->iofd, "CONNECT" );
		break;
	}
	return ;
}

void print_result( struct ctrl_cfg *c, int res )
{
	if( c->verbose_mode == VERBOSE_NUMERIC ) {
		//printf( "%d", res );
		fdprintf( c->iofd, "%d", res ); 
	} else if ( c->verbose_mode == VERBOSE_ENGLISH ) {
		print_english_result( c, res );
	}
	switch( c->eol_code ) {
	case EOL_NONE:
		break;
	case EOL_CR:
		fdprintf( c->iofd, "%c", c->cr_char );
		break;
	case EOL_LF:
		fdprintf( c->iofd, "%c", c->lf_char );
		break;
	case EOL_CRLF:
		fdprintf( c->iofd, "%c%c", c->cr_char, c->lf_char );
		break;
	case EOL_CRNULL:
		fdprintf( c->iofd, "%c%c", c->cr_char, 0x00 );
		break;
	case EOL_LFNULL:
		fdprintf( c->iofd, "%c%c", c->lf_char, 0x00 );
		break;
	case EOL_CUSTOM:
		fdprintf( c->iofd, "%s", c->eol_custom );
		break;
	}
	return;
}

int read_cmd_line( int fd, char *buf, unsigned int buf_len )
{
	unsigned int count = 0;
	char ch;
	while( read( fd, &ch, 1 ) > 0 ) {
		if( ch != '\n' ) {
			buf[count] = ch;
			count++;
		} else {
			break;
		}
		if( count == ( buf_len - 1 ) ) {
			break;
		}
	}
	return count;
}

int parse_at_command ( struct ctrl_cfg *c, struct kv_pair_list **l, char *buf )
{
	int res = RES_OK;
	char *val_buf;
	char *key = NULL;
	char *value = NULL;

	switch( buf[0] ) {
	case 'A':
		if( buf[1] != 'T' ) {
			res = RES_ERROR;
		} else if ( buf[2] == '\0' ) {
			res = RES_OK;
		} else {
			val_buf = malloc( strlen( buf )+1 );
			memset( val_buf, 0, strlen( buf )+1 );
			memcpy( val_buf, buf, strlen( buf ) );
			//  first two bytes of buf are 'AT' so start processing
			if( buf[2] == '?' ) {
				res = RES_ERROR;
				if( buf[3] ) {
					key = parse_key( &buf[3] );
					//printf("Key = %s\n\r", key);
					if( key != NULL ) {
						value = find_key_value( *l, key );
						if( value != NULL ) {
							//printf( "%s\n\r", value );
							fdprintf(c->iofd, "%s\n\r", value );
							res = RES_OK;
						}
					}
				}
			} else if( buf[2] == '+' ) {
				//printf("Set Value(s) - %s\n\r", &buf[3]);
				res = RES_ERROR;
				key = parse_key( &buf[3] );
				if( key != NULL ) {
					//printf("Key = %s\n\r", key);
					value = parse_value( &val_buf[3] );
					if( value != NULL ) {
						//printf("Value = %s\n\r", value);
						if( ( key != NULL ) && ( value != NULL ) ) {
							if( add_kv_pair( *l, key, value ) ) {
								res = RES_OK;
							}
						}
					}
				}
			} else if( buf[2] == '-' ) {
				res = RES_ERROR;
				//printf("Del Value(s) - %s\n\r", &buf[3]);
				key = parse_key( &buf[3] );
				if( key != NULL ) {
					if( del_kv_pair( l, key ) ) {
						res = RES_OK;
					}
				} else {
					res = RES_ERROR;
				}
			} else if( buf[2] == 'Q' ) {
				if( buf[3] == '1' ) {
					c->quiet_mode = QUIET_ON;
				} else if ( ( ! buf[3] ) || ( buf[3] == '0' ) ) {
					c->quiet_mode = QUIET_OFF;
				} else {
					res = RES_ERROR;
				}
			} else if( buf[2] == 'V' ) {
				if( buf[3] == '1' ) {
					c->verbose_mode = VERBOSE_ENGLISH;
				} else if ( ( ! buf[3] ) || ( buf[3] == '0' ) ) {
					c->verbose_mode = VERBOSE_NUMERIC;
				} else {
					res = RES_ERROR;
				}
			} else if( buf[2] != '\0' ) {
				res = RES_ERROR;
			}
			free( val_buf );
		}
		break;
	default:
		res = RES_ERROR;
		break;
	}
	if( c->quiet_mode == QUIET_OFF ) {
		print_result( c, res );
	}
	return res;
}

struct kv_pair_list *init_items( void )
{
	kv_count = 0;
	struct kv_pair_list *llist;
	llist = malloc( sizeof( struct kv_pair_list ) );
	llist->next = NULL;
	return llist;
}

void print_kv_pair_lists( struct ctrl_cfg *c, struct kv_pair_list *l )
{
	if ( kv_count > 0 ) {
		while( l->next != NULL ) {
			if( l != NULL ) {
				//printf( "%s = %s ( %d )\n\r", l->kv.key, l->kv.value, (int)l );
				//printf( "%s=%s\n\r", l->kv.key, l->kv.value );
				fdprintf( c->iofd, "%s=%s\n\r", l->kv.key, l->kv.value );
			}
			l = l->next;
		}
	}
	//printf("(%d)\n\r", kv_count);
}

int kv_pair_lists_count( void )
{
	return kv_count;
}

int add_kv_pair( struct kv_pair_list *l, char *key, char *value )
{
	int done = 0;
	if( key[0] && value[0] ) {
		while( l->next != NULL ) {
			if( ( strlen( l->kv.key ) == strlen( key ) ) &&
			    ( strncmp( l->kv.key, key, strlen( key ) ) == 0 ) ) {
				free( l->kv.value );
				l->kv.value = malloc( strlen( value )+1 );
				memset( l->kv.value, 0, ( strlen( value )+1 ) );
				memcpy( l->kv.value, value, strlen( value ) );
				l->kv.value[ strlen( value ) ] = 0;
				done = 1;
				break;
			} else {
				l = l->next;
			}
		}
		if( done == 0 ) {
			l->next = ( struct kv_pair_list * )malloc( sizeof( struct kv_pair_list ) );
//			printf("item %d = %d\n\r", kv_count, (int)l);
//			printf("item %d->next = %d\n\r", kv_count, (int)l->next);
			l->kv.key = malloc( strlen( key )+1 );
			l->kv.value = malloc( strlen( value )+1 );
			memset( l->kv.key, 0, ( strlen( key )+1 ) );
			memset( l->kv.value, 0, ( strlen( value )+1 ) );
			memcpy( l->kv.key, key, strlen( key ) );
			memcpy( l->kv.value, value, strlen( value ) );
			l->next->next = NULL;
			//l->next=NULL;
			kv_count++;
			return 1;
		}
	}
	return done;
}

int del_kv_pair( struct kv_pair_list **l, char *key )
{
	struct kv_pair_list *temp, *prev;
	int done = 1;
	prev = *l;
	temp = prev->next;

	if ( prev != NULL &&
	     ( strlen( key ) == strlen( prev->kv.key ) ) &&
	     ( strncmp( prev->kv.key, key, strlen( key ) ) == 0 ) ) {
		*l = temp;
		free( prev->kv.key );
		free( prev->kv.value );
		free( prev );
		kv_count--;
		return 1;
	}
	while( temp != NULL ) {
		if ( strncmp( temp->kv.key, key, strlen( key ) ) == 0 &&
		     ( strlen( key ) == strlen( temp->kv.key ) ) ) {
			break;
		}
		prev = temp;
		temp = temp->next;
		done++;
	}
	if ( temp == NULL ) {
		return 0;
	}
	prev->next = temp->next;
	free( temp->kv.key );
	free( temp->kv.value );
	free( temp );
	kv_count--;
	return done;
}

void del_all_kv_pairs( struct kv_pair_list **l )
{
	while( *l != NULL ) {
		struct kv_pair_list *temp = *l;
		free( temp->kv.key );
		free( temp->kv.value );
		*l = temp->next;
		free( temp );
	}
	*l = init_items();
	kv_count = 0;
	return;
}


char *find_key_value( struct kv_pair_list *l, char *key )
{
	if( kv_count > 0 ) {
		while( l->next != NULL ) {
			if( ( strlen( l->kv.key ) == strlen( key ) ) && ( strncmp( l->kv.key, key, strlen( key ) ) == 0 ) ) {
				return l->kv.value;
			} else {
				l = l->next;
			}
		}
	}
	return NULL;
}

char *parse_key( char *str )
{
	char *key;
	key = strtok( str, "=" );
	return key;
}

char *parse_value( char *str )
{
	char *value = NULL;
	char *key = NULL ;
	char *rest;

	//printf("parse_value: %s\n\r", str );
	key = strtok_r( str, "=", &rest ); // get past the key
	if ( key != NULL ) {
		value = strtok_r( NULL, " ,", &rest );
	}
	if ( value != NULL ) {
		//printf("rest = %s\n\r", rest );
		str = rest;
	}
	return value;
}

int write_kv_file( struct kv_pair_list *l, char *cfg, int mode )
{
	int ok = 0;
	int fd;
	if( mode == CFG_MODE_INIT ) {
		//printf( "erasing current config %s\n\r", cfg );
		//fdprintf(CFG.iofd, "erasing current config %s\n\r", cfg );
		fd = open( cfg, O_RDWR | O_CREAT | O_TRUNC, 0667 );
	}
	if ( mode == CFG_MODE_ADD ) {
		//printf( "appending current config %s\n\r", cfg );
		//fdprintf(CFG.iofd, "appending current config %s\n\r", cfg );
		fd = open( cfg, O_RDWR | O_APPEND, 0667 );
	}
	if( fd > 0 ) {
		if ( kv_count > 0 ) {
			while( l->next != NULL ) {
				if( l != NULL ) {
					//printf( "%s = %s ( %d )\n\r", l->kv.key, l->kv.value, (int)l );
					ok = fdprintf( fd, "%s=%s\n\r", l->kv.key, l->kv.value );
					if( ok < 0 ) {
						//printf( "error writing config file %s (%d)\n\r", cfg, ok );
						//fdprintf(CFG.iofd, "error writing config file %s (%d)\n\r", cfg, ok );
						close( fd );
						return ok;
					}
				}
				l = l->next;
			}
		}
		close( fd );
	} else {
		//printf( "error opening config file %s (%d)\n\r", cfg, fd );
		//fdprintf(CFG.iofd, "error opening config file %s (%d)\n\r", cfg, fd );
		return fd;
	}
	return ok;
}

int parse_kv_file( struct kv_pair_list *l, char *cfg, int mode )
{
	int ok = 0;
	char buf[ 512 ], buf2[512];
	char *key;
	char *value;
	int fd = open( cfg, O_RDONLY );

	if( mode == CFG_MODE_INIT ) {
		//printf( "erasing current config %s\n\r", cfg );
		//fdprintf(CFG.iofd, "erasing current config %s\n\r", cfg );
		close( fd );
		fd = open( cfg, O_RDONLY | O_TRUNC );
	}
	if( fd < 0 ) {
		//printf( "error reading config file %s\n\r", cfg );
		//fdprintf(CFG.iofd, "error reading config file %s\n\r", cfg );
		ok = -1;
	} else {
		//printf( "reading config file %s\n\r", cfg );
		//fdprintf(CFG.iofd, "reading config file %s\n\r", cfg );
		memset( buf, 0, sizeof( buf ) );
		memset( buf2, 0, sizeof( buf2 ) );
		while( ( ok = read_cmd_line( fd, buf, sizeof( buf ) ) ) > 0 ) {
			if( ( buf[0] != '\n' ) &&
			    ( buf[0] != '\r' ) &&
			    ( buf[0] != '#' ) &&
			    ( strlen( buf ) > 2 ) ) {
				memcpy( buf2, buf, ok );
				//printf("read %d bytes [ %s ]\n\r", ok, buf);
				key = parse_key( buf );
				//printf("key [ %s ]\n\r", key);
				value = parse_value( buf2 );
				//printf("value [ %s ]\n\r", value);
				if( ( key != NULL ) && ( value != NULL ) ) {
					//printf("key[ %s ] = value[ %s ]\n\r", key, value);
					if( ! add_kv_pair( l, key, value ) ) {
						//printf( "error adding key value pair [ %s = %s ]\n\r", key, value );
						//fdprintf(CFG.iofd, "error adding key value pair [ %s = %s ]\n\r", key, value );
						ok = -1;
					}
				} else {
					//printf("error getting key value pair [ %s ]\n\r", buf);
				}
				memset( buf, 0, sizeof( buf ) );
				memset( buf2, 0, sizeof( buf2 ) );
			}
		}
		close( fd );
	}
	return ok;
}

int tokpos( char *str, int len, char tok )
{
	int i;
	int pos = -1;
	for( i=0; i<len; i++ ) {
		if( str[i] == tok ) {
			pos = i;
			break;
		}
	}
	return pos;
}

int process_cmd_buf( char *cmdbuf, int cmd_len, struct lat_cmd *c )
{
	int i = 0;

	while ( c[i].name != NULL ) {
		if( !strncmp( cmdbuf,c[i].name, cmd_len ) &&
		    ( cmd_len == ( int )strlen( c[i].name ) ) ) {
			if( c[i].callback != NULL ) {
				c[i].callback();
			} else {
				// if the command doesn't have a call back function return 0, to do more processing
				return 0;
			}
			return 1;
		} else {
			i++;
		}
	}
	return 0;
}
#if 0
{
	int pos = 0;
	int pos2 = 0;
	int ok = 0;

//	int i;
	char buf1[32], buf2[32];
	char *bufptr = cmdbuf;

	while( ( ok = tokpos( bufptr, len - pos, '=' ) ) > 0 ) {
		memset( buf1, 0, sizeof( buf1 ) );
		memset( buf2, 0, sizeof( buf2 ) );
		printf( "= @ pos %d\n\r", ok );
		pos += ok;
		memcpy( buf1, bufptr, ok );
		if ( ( pos2 = tokpos( &bufptr[ok+1], len - pos, ';' ) ) > 0 ) {
			printf( "; @ pos %d\n\r", pos2 );
			memcpy( buf2, &bufptr[ok+1], pos2 );
		} else {
			pos2 = 0;
		}
		if( buf1[0] && buf2[0] ) {
			printf( "Key=%s, Val=%s\n\r", buf1, buf2 );
			pos += 2;
		}
		pos += pos2;
		bufptr = &cmdbuf[ pos ];
	}

	return ok;
}
#endif

