#ifndef __IOT_CFG_H__
#define __IOT_CFG_H__

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
#include <Arduino.h>  // for type definitions
#include <avr/pgmspace.h>

#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#define MAX(a,b)  ((a) > (b) ? (a) : (b))

#define BZERO(x, y)    memset(x, 0, y)

#define MAX_NAME_LEN  16
#define MAX_STR_LEN   (MAX_NAME_LEN*2)
#define MAC_ADDR_LEN  6
#define IP_ADDR_LEN   4


#define IOT_VERSION       1

#define IOT_TYPE_MASTER   0
#define IOT_TYPE_SWITCH   30
#define IOT_TYPE_SENSOR   40
#define IOT_TYPE_SLAVE    50
#define IOT_TYPE_LISTENER 60
#define IOT_TYPE_DOGGY    70

#define CFG_ERR_OK        0
#define CFG_ERR_INIT      -100
#define CFG_ERR_CRC       -101
#define CFG_ERR_LOAD      -102
#define CFG_ERR_SAVE      -103
#define CFG_ERR_VERSION   -104
#define CFG_ERR_UNKNOWN   -113

#define CFG_VERSION       IOT_VERSION

#define CFG_RESET_FLAG    0

#define CFG_ADDR          ( CFG_SIZE * 0 )

#define CLASS_C_NETMASK   24
#define CLASS_B_NETMASK   16
#define CLASS_A_NETMASK   8

#define NET_PROTO_TCP     4
#define NET_PROTO_UDP     6

#define SHORT_DELAY       10

/* default config */
#define CFG_DEFAULT_NAME  "dumbDev"


/********************************************************************************
 * [0-1]      Version
 * 
 ********************************************************************************/

struct gpio_cfg{
	uint16_t   IO_flags;
	uint16_t   HL_flags;
	uint16_t   timer1;
	uint16_t   timer2;
	uint16_t   IO_mask;
	uint16_t   HL_mask;
	uint16_t   delay1;
	uint16_t   delay2;
};

struct net_auth{
	uint8_t ssid[ MAX_STR_LEN ];
	uint8_t passkey[  MAX_STR_LEN ];
	uint8_t user[  MAX_STR_LEN ];
	uint8_t password[  MAX_STR_LEN ];
};

struct net_cfg{
	uint8_t eth_addr[ MAC_ADDR_LEN ];
	uint8_t ip_addr[ IP_ADDR_LEN ];
	uint8_t netmask;
	uint8_t mode;
	uint16_t port;
	uint8_t proto;
	uint8_t none;
	struct net_auth auth;
};

struct d_cfg{
	uint16_t  ver;
	uint16_t  id;
	uint8_t   type;
	uint8_t   reserved_1[ IP_ADDR_LEN ];  // place holder
	//  struct gpio_cfg   gpio; // gpio configuration
	uint8_t   name[ MAX_NAME_LEN ]; // device name
	struct net_cfg netcfg; // network configuration
	uint8_t    reserved_2[ MAX_STR_LEN ];
	uint8_t   save_page;    // partion in EEPROM where it's saved
	uint8_t   dcrc;       // configuration crc
};

struct d_state{
	struct d_cfg *cfg;
	uint16_t cfg_flags;
	uint8_t status;
	uint8_t state;
	uint16_t reserved;
};

#define CFG_SIZE          ( sizeof( struct d_cfg ))

/*  function prototypes */

int16_t init_cfg( struct d_cfg *d, uint8_t flag  );
int16_t load_cfg( struct d_cfg *d );
int16_t save_cfg( struct d_cfg *d );

int16_t set_device_id( uint16_t id );
uint16_t get_device_id( void );

int8_t set_device_type( uint8_t type );
uint8_t get_device_type( void );
int8_t set_device_name( uint8_t *name, uint8_t name_len );
uint8_t *get_device_name( void );


uint8_t CRC8_2(uint8_t *data, uint8_t len);
uint8_t CRC8(uint8_t *message, uint8_t length);

#endif
