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
#include <Arduino.h>  // for type definitions
#include <stdarg.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include "iot_cmd.h"

enum cmd_no user_cmd;

const char *cmd_list[CMD_LIST_LEN] = {
  "config",
  "show",
  "load",
  "save",
  "connect",
  "send",
  "listen",
  "close",
  "help",
  "reinit",
  "reset"
};

uint8_t get_command( uint8_t *cmd )
{
  uint8_t i;
  uint8_t ok = 0;
  for(i=0;i<CMD_LIST_LEN;i++) {
    if( ( strlen((const char *)cmd) - 1) == strlen((const char *)cmd_list[i])) {
      ok = strncmp((const char *)cmd, cmd_list[i], (strlen((const char *)cmd) - 1));
      if( ok == 0 ){
        return (uint8_t)i + 1;
      }
    }
  }
  return 0;
}

void software_reset( void )
{
  asm volatile ("  jmp 0");  
}





