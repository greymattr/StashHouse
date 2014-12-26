#ifndef __IOT_CMD_H__
#define __IOT_CMD_H__
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


#define CMD_LIST_LEN  16
#define MAX_CMD_LEN   16

#define CMD_ERR_UNKNOWN       -20
#define CMD_ERR_OK            0

enum cmd_no{
    config=1,
    show,
    load,
    save,
    connect,
    send,
    listen,
    close,
    help,
    reinit,
    reset
};

uint8_t get_command( uint8_t *cmd );
void software_reset( void );

#endif
