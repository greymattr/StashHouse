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


void init_cfg( struct d_cfg *d )
{
  BZERO(d, sizeof(d));
}

uint16_t crc_ck( uint8_t *data, uint16_t len )
{
  uint16_t crc = 0;
  uint16_t i = 0;
  for(i=0;i<len;i++){
    crc += data[i];
  }
  return crc;
}

int16_t load_cfg( void )
{
  uint16_t crc;
  uint16_t i;
  uint8_t c[ CFG_SIZE ];

  for(i=0;i<CFG_SIZE;i++){
    c[i] = EEPROM.read(i);
  }
  memcpy(&dCfg, c, CFG_SIZE);
  crc = crc_ck(&dCfg, ( CFG_SIZE - 2 ));
  if( crc != dCfg.dcrc ){
    return -1;
  }
  return 0;
}

void save_cfg( void )
{
  uint16_t i;
  uint8_t c[ CFG_SIZE ];

  memcpy(c, &dCfg, CFG_SIZE);
  dCfg.dcrc = crc_ck(&dCfg, ( CFG_SIZE - 2 ));
  for(i=0;i<CFG_SIZE;i++){
    c[i] = EEPROM.write(i);
  }
}



