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

void user_pause();
uint16_t get_serial_input( uint8_t *input );
void p( char *fmt, ... );


extern struct d_cfg dCfg;

extern uint8_t crc1, crc2;

extern uint8_t cfg_inited = 0;


//#define CFG_HEX_DUMP

void print_cfg( struct d_cfg *d )
{
	uint8_t i = 0;
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
	p(" Size     : %d\n\r", CFG_SIZE);
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

void loop() {
	uint16_t ok = 0;
  uint8_t selection[ MAX_USER_INPUT_LEN ];
  
	ok = load_cfg( &dCfg );
	if( ok != CFG_ERR_OK ){
		p("load config error %d ( %d ) ( %d )\n\r", ok, crc1, crc2);
		print_cfg( &dCfg );
		p("creating config %d bytes\n\r", CFG_SIZE);
		ok = init_cfg( &dCfg, CFG_RESET_FLAG);
		if( ok != CFG_ERR_OK ){
			p("error creating config %d\n\r", ok);
		} 
		else {
			print_cfg( &dCfg );
		}
		ok = save_cfg( &dCfg );
		if ( ok != CFG_ERR_OK ) {
			p("error saving config\n\r", ok);
		}
		p("config saved to EEPROM %d bytes\n\r", CFG_SIZE);  
	} 
	else {
		p("config load from EEPROM - ok ( %d bytes )\n\r", CFG_SIZE);
		print_cfg( &dCfg ); 
	}

	while(1){
    p("> ");
    get_serial_input(selection);
    p("\n\rInput = %s\n\r", selection);
		delay(1000);
	}
}

void user_pause()
{
  while( !Serial.available() ) {
    ;
  }
}

#define MIN_CMD_LEN     2

uint16_t get_serial_input( uint8_t *input )
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
      if( ( b == 0x0a ) || ( b== 0x0d ) ) {
        break;
      } else {
        Serial.write( b );
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

