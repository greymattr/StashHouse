/*
Copyright (c) 2014 Matthew Fatheree
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include <EEPROM.h>

#define	MIN(a,b)	((a) < (b) ? (a) : (b))
#define	MAX(a,b)	((a) > (b) ? (a) : (b))

#define BZERO(x, y)    memset(x, 0, y)

#if 0
#define u_int8 unsigned char
#define int8 char
#define u_int16 unsigned int
#define int16 int
#endif

#define MAX_USER_INPUT_LEN  64

#include <Arduino.h>  // for type definitions
#include <stdarg.h>
#include <avr/pgmspace.h>
#include "iot_cfg.h"
#include "iot_cmd.h"

#define MIN_CMD_LEN     2

void user_pause();
int16_t do_config();
uint16_t get_serial_ipaddr( uint8_t *ipaddr );
uint16_t get_serial_int( int16_t number );
uint16_t get_serial_hex( int16_t number );
uint16_t get_serial_input( uint8_t *input, uint8_t delim );
void p_ok( void );
void p_err( int16_t err );
void p( char *fmt, ... );


void(* resetFunc) (void) = 0;

static struct d_cfg dCfg;

extern uint8_t crc1, crc2;

extern uint8_t cfg_inited = 0;


void print_cfg( struct d_cfg *d )
{
//	uint8_t i = 0;
	p("\n\rConfiguration\n\r");
	p(" Ver      : %d\n\r", d->ver);
	p(" ID       : %d\n\r", d->id);
	p(" Type     : %d\n\r", d->type);
	p(" Name     : %s\n\r", d->name);
#define SHOW_NETCFG
#ifdef SHOW_NETCFG
	p("NetCfg\n\r");
	p(" Eth Addr : %02X:%02X:%02X:%02X:%02X:%02X\n\r", d->netcfg.eth_addr[0],
	d->netcfg.eth_addr[1], d->netcfg.eth_addr[2], d->netcfg.eth_addr[3],
	d->netcfg.eth_addr[4], d->netcfg.eth_addr[5]);
	p(" IP Addr  : %d.%d.%d.%d\n\r", d->netcfg.ip_addr[0],
	d->netcfg.ip_addr[1], d->netcfg.ip_addr[2], d->netcfg.ip_addr[3]);
	p(" Proto    : %d\n\r", d->netcfg.proto);
	p(" Port     : %d\n\r", d->netcfg.port);
	p(" Mode     : %d\n\r", d->netcfg.mode);
#endif
	p("--------------------------------\n\r");  
	p(" CRC      : %d\n\r", d->dcrc);
//	p(" Size     : %d\n\r", CFG_SIZE);
	p("\n\r");
}

/* SETUP - MAIN SETUP */
void setup() {
	Serial.begin( 115200 );
	while( !Serial ) {
		delay( 500 );
	}
	p("reset...\n\r"); 
}

void p_ok( void )
{
  p("\n\rOK\n\r");
}

void p_err( int16_t err )
{
  p("\n\rERR - %d\n\r", err);
}

void loop() {
	uint16_t ok = 0;
  uint8_t selection[ MAX_USER_INPUT_LEN ];
  
	ok = load_cfg( &dCfg );
	if( ok != CFG_ERR_OK ){
		p_err( ok );
		print_cfg( &dCfg );
		p("creating config %d bytes\n\r", CFG_SIZE);
		ok = init_cfg( &dCfg, CFG_RESET_FLAG);
		if( ok != CFG_ERR_OK ){
			p_err(ok);
		} 
		else {
			print_cfg( &dCfg );
		}
		ok = save_cfg( &dCfg );
		if ( ok != CFG_ERR_OK ) {
			p_err(ok);
		} else {
		  p_ok();
	  }  
	} 
	else {
		p_ok();
		print_cfg( &dCfg ); 
	}

	while(1){
    BZERO(selection, sizeof(selection));
    p("\n\r> ");
    ok = get_serial_input(selection, NULL);
    if( ok ){
      ok = get_command( selection );
      if( ok ) {
        switch( ok )
        {
          case config:
            do_config();
          break;
          case show:
            print_cfg( &dCfg );
          break;
          case load:
            ok = load_cfg( &dCfg );
            if( ok != CFG_ERR_OK ){
              p_err(ok);
              print_cfg( &dCfg );
            } else {
              p_ok();
            }
          break;
          case save:
            print_cfg( &dCfg );
            save_cfg( &dCfg );
            p_ok();
          break;
          case reset:
            resetFunc();
          break;
          default:
            goto err_unknown;
          break;
        }
      } else {
        err_unknown:
        p_err(CMD_ERR_UNKNOWN);
      }
    }
		delay(1000);
	}
}

int16_t do_config()
{
  uint8_t ok = 0;
  uint8_t cur_crc = dCfg.dcrc;

  p("\n\rConfigure\n\r");
  if ( get_serial_ipaddr((uint8_t *) &dCfg.netcfg.ip_addr[0] ) == 0){
    //p("\n\rUser ipaddr: %d.%d.%d.%d\n\r", dCfg.netcfg.ip_addr[0], dCfg.netcfg.ip_addr[1], dCfg.netcfg.ip_addr[2], dCfg.netcfg.ip_addr[3]); 
  }
  if ( get_serial_ethaddr((uint8_t *) &dCfg.netcfg.eth_addr[0] ) == 0){
    //p("\n\rUser Ethaddr: %02X:%02X:%02X:%02X:%02X:%02X\n\r", dCfg.netcfg.eth_addr[0], dCfg.netcfg.eth_addr[1], dCfg.netcfg.eth_addr[2], dCfg.netcfg.eth_addr[3],dCfg.netcfg.eth_addr[4], dCfg.netcfg.eth_addr[5]); 
  }
}

void user_pause()
{
  while( !Serial.available() ) {
    ;
  }
}

uint16_t get_serial_ipaddr( uint8_t *ipaddr )
{
  uint8_t buf[IP_ADDR_LEN];
  uint8_t i = 0;
  uint8_t ok = 0;
  BZERO(buf, sizeof(buf));
  p("\n\r%d.%d.%d.%d ", ipaddr[0],ipaddr[1],ipaddr[2],ipaddr[3]);
  for(i=0;i<sizeof(buf);i++){
    buf[i] = ( uint8_t ) get_serial_int(ipaddr[i]);
  }
  for(i=0;i<sizeof(buf);i++){
    if( ((int16_t)(buf[i]) < 0 ) || ((int16_t)(buf[i]) > 254)) {
      ok = 0;
    }
  }
  if( ok == 0 ){
    memcpy(ipaddr, buf, sizeof(buf));
  }
  return ok;
}

uint16_t get_serial_ethaddr( uint8_t *ethaddr )
{
  uint8_t buf[ MAC_ADDR_LEN ];
  uint8_t i = 0;
  uint8_t ok = 0;
  BZERO(buf, sizeof(buf));
  p("\n\r%02X:%02X:%02X:%02X:%02X:%02X ", ethaddr[0],ethaddr[1],ethaddr[2],ethaddr[3],ethaddr[4],ethaddr[5]);
  for(i=0;i<sizeof(buf);i++){
    buf[i] = ( uint8_t ) get_serial_hex(ethaddr[i]);
    //p("buf[i] = 0x%02X\n\r", buf[i]);
  }
  for(i=0;i<sizeof(buf);i++){
    if( ((int16_t)(buf[i]) < 0 ) || ((int16_t)(buf[i]) > 255)) {
      ok = 0;
    }
  }
  if( ok == 0 ){
    memcpy(ethaddr, buf, sizeof(buf));
  }
  return ok;
}

uint16_t get_serial_int( int16_t number )
{
  uint16_t ret =  0;
  uint8_t buf[ MAC_ADDR_LEN ];

  BZERO(buf, sizeof(buf));
  p("( %d ) ", number);
  if ( get_serial_input( buf, '.') ) {
    ret = ( uint16_t ) atoi(( const char *) buf );
    if( ( ret >= 0  ) && ( ret <= 0xffff ))return ret;
  }
  return number;
}

uint16_t get_serial_hex( int16_t number )
{
  uint16_t ret =  0;
  uint8_t buf[ MAC_ADDR_LEN ];

  BZERO(buf, sizeof(buf));
  p("( %02X ) ", number);
  if ( get_serial_input( buf, ':') ){
    if((buf[1] == 'x' ) || ( buf[1] == 'X')){
      ret = ( uint16_t ) (int)strtol((const char *)buf, NULL, 0);
    } else {
      ret = ( uint16_t ) (int)strtol((const char *)buf, NULL, 16);
    }
    if(( ret >= 0  ) && ( ret <= 256 ))return ret;
  }
  return number;
}

uint16_t get_serial_input( uint8_t *input, uint8_t delim )
{
  uint16_t i=0;
  uint8_t b = 0;
  uint8_t commandbuffer[ MAX_USER_INPUT_LEN ];
  
  user_pause();
  memset( commandbuffer, 0, sizeof( commandbuffer ) );
  while( i < ( sizeof( commandbuffer )-1 ) ) {
    if( Serial.available() ) {
      b = Serial.read();
      if( b != 0x08 ){
        commandbuffer[i++] = b;
      } else {
        commandbuffer[i] = '\0';
        i--;
      }
      if( delim != NULL ){
        if( ( b == 0x0a ) || ( b== 0x0d ) || ( b==delim)) {
          break;
        } else {
          Serial.write( b );
        }
      } else {
        if( ( b == 0x0a ) || ( b== 0x0d ) ) {
          break;
        } else {
          Serial.write( b );
        }
      }
    }
  }
  if( i < MIN_CMD_LEN ) {
    memset( commandbuffer, 0, sizeof( commandbuffer ) );
    i = 0;
  }
  commandbuffer[i]='\0';
  //p("\ncmd buffer = %s\n", commandbuffer);
  memcpy(input, commandbuffer, i);
  return i;
}

#define MAX_PRINTF_LEN  128

void p( char *fmt, ... )
{
  char buf[ MAX_PRINTF_LEN ]; // resulting string limited to 128 chars
  va_list args;
  va_start ( args, fmt );
  memset( buf, 0, sizeof( buf ) );
  vsnprintf( buf, MAX_PRINTF_LEN, fmt, args );
  va_end ( args );
  Serial.print( buf );
}

