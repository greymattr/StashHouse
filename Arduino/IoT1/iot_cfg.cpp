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

#include "iot_cfg.h"

struct d_cfg dCfg;
struct net_cfg nCfg;
struct net_auth authCfg; 
uint8_t crc1, crc2;
static uint8_t old_crc = 0;
static uint8_t cfg_inited = 0;

/*  default config */
uint8_t default_mac_addr[ MAC_ADDR_LEN ] = { 
  0x00, 0x80, 0xA3, 0X13, 0x13, 0x26 };
uint8_t default_ip_addr[ IP_ADDR_LEN ] = { 
  192, 168, 0, 113 };
uint16_t default_port = 1313;
uint16_t default_id = 1;
uint8_t default_netmask = CLASS_C_NETMASK;
uint8_t default_type = IOT_TYPE_MASTER;

int16_t init_cfg( struct d_cfg *d, uint8_t flag )
{
  BZERO(d, sizeof(struct d_cfg));

  d->ver = CFG_VERSION;
  d->id = default_id;
  d->type = default_type;
  memcpy(d->name, CFG_DEFAULT_NAME, 7);
  memcpy(d->netcfg.ip_addr, default_ip_addr, sizeof(default_ip_addr));
  memcpy(d->netcfg.eth_addr, default_mac_addr, sizeof(default_mac_addr)); 
  d->netcfg.netmask = default_netmask;
  d->netcfg.port = default_port;

  d->dcrc = CRC8((uint8_t *) d, ( CFG_SIZE - sizeof(uint8_t)));
  cfg_inited = 1;
  return CFG_ERR_OK;
}

int16_t load_cfg( struct d_cfg *d )
{
  uint8_t my_crc;
  uint16_t i = 0;
  uint8_t c[ CFG_SIZE + 1];

  BZERO(c, sizeof(c));
  for(i=0;i<=CFG_SIZE;i++){
    c[i] = EEPROM.read( CFG_ADDR + i);
  }
  my_crc = CRC8((uint8_t *)&c, ( CFG_SIZE - (sizeof(uint8_t))));
  if( my_crc != c[CFG_SIZE - 1] ){
    crc1 = my_crc;
    crc2 = c[CFG_SIZE - 1];
    return CFG_ERR_CRC;
  }
  memcpy(d, c, CFG_SIZE);
  old_crc = d->dcrc;
  return CFG_ERR_OK;
}

int16_t save_cfg( struct d_cfg *d )
{
  uint16_t i;
  uint8_t c[ CFG_SIZE ];

  d->dcrc = CRC8((uint8_t *)d, ( CFG_SIZE - (sizeof(uint8_t))));
  memcpy(c, d, CFG_SIZE);
  for(i=0;i<=CFG_SIZE;i++){
    EEPROM.write(CFG_ADDR + i ,c[i]);
    //delay( SHORT_DELAY );
  }
  old_crc = d->dcrc;
  return CFG_ERR_OK;
}

uint8_t bits_to_netmask( uint8_t bits, uint8_t *netmask)
{
  uint8_t i = 0;
  uint8_t x = 0;
  
  memset( netmask, 255, IP_ADDR_LEN);
  for(i=1;i<=bits;i++){
    if((i % 8 ) == 0 )x=0;
    x++;
    if(i < 9) CLR_BIT(netmask[3], x);
    else if(i < 17) CLR_BIT(netmask[2], x);
    else if(i < 25) CLR_BIT(netmask[1], x);
    else if(i < 33) CLR_BIT(netmask[0], x);
  }
  return bits;
}

uint8_t netmask_to_bits( uint8_t *netmask )
{
  uint8_t bits = 0;
  uint8_t i;
  uint8_t x = 0;

  for(i=1;i<=32;i++){
    if((i % 8 ) == 0 )x=0;
    x++;
    if( i < 9 ) {
      if(IS_BIT_SET(netmask[3], x))break;
      else bits++;
    }
    else if ( i < 17) {
      if(IS_BIT_SET(netmask[2], x))break;
      else bits++;
    }
    else if ( i < 25) {
      if(IS_BIT_SET(netmask[1], x))break;
      else bits++;
    }
    else {
      if(IS_BIT_SET(netmask[0], x))break;
      else bits++;
    }
  }
  return bits;
}


/*
CRC-8 - based on the CRC8 formulas by Dallas/Maxim
code released under the therms of the GNU GPL 3.0 license
*/
#if 0
uint8_t CRC8_2(uint8_t *data, uint8_t len) {
  uint8_t crc = 0x00;
  while (len--) {
    uint8_t extract = *data++;
    for (uint8_t tempI = 8; tempI; tempI--) {
      uint8_t sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum) {
        crc ^= 0x8C;
      }
      extract >>= 1;
    }
  }
  return crc;
}
#endif

uint8_t CRC8(uint8_t *message, uint8_t length)
{
  uint8_t i, j, crck = 0;

  for (i = 0; i < length; i++)
  {
    crck ^= message[i];
    for (j = 0; j < 8; j++)
    {
      if (crck & 1)
        crck ^= 0x91;
      crck >>= 1;
    }
  }
  return crck;
}

