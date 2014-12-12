#ifndef __IOT_CFG_H__
#define __IOT_CFG_H__


/********************************************************************************
 * [0-1]      Version
 * [2-3]      deviceId
 * [4]        deviceType
 * [5-8]      (reserved)
 * [9-25]    gpio cfg
 * [26-31]    (reserved)
 * [32-33]    crc
 * 
 ********************************************************************************/

#typedef struct d_cfg{
  uint_16t  ver;
  uint_16t  id;
  uint8_t   type;
  uint8_t   res1[3];  // place holder
  uint8_t   gpio[15]; // place holder
  uint16_t  crc;
}d_cfg;

#define CFG_START         0
#define CFG_VER           2













#endif