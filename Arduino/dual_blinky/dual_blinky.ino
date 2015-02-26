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

#define MODE_OFF          0
#define MODE_ON           1
#define MODE_BLINK        2
#define MODE_FADE         3
#define MODE_PULSE        4

#define MIN_CMD_LEN       2
#define MAX_STEPS         20

#define CMD_DELIM         ';'

#define BZERO(x, y)    memset(x, 0, y)

#define	MIN(a,b)	((a) < (b) ? (a) : (b))
#define	MAX(a,b)	((a) > (b) ? (a) : (b))



struct color_t {
  uint8_t   r;      // red color value 0 - 255
  uint8_t   g;      // green color value 0 - 255
  uint8_t   b;      // blue color value 0 - 255
}color_t;

struct led_t {
  uint8_t   r_pin;      // red PWM pin  
  uint8_t   g_pin;      // green PWM pin
  uint8_t   b_pin;      // blue PWM pin
  struct color_t start; // the start color for the LED
  struct color_t curr;  // the current color
  struct color_t end;   // the end color for the LED
  uint8_t   mode;       // the led mode: on, off, blink, pulse, fade
  uint8_t   r_step;     // the step value for the red pin
  uint8_t   g_step;     // the step value for the green pin
  uint8_t   b_step;     // the step value for the blue pin
  uint16_t  freq;       // the LED transition frequency
  uint8_t   repeat;     // how often to repeat
  uint8_t   counter;    // a counter for this LED
  unsigned long timer;  // keep track of time for this LED
  uint8_t  dcrc;
}led_t;

#define CFG_SIZE sizeof(struct led_t)
//#define CFG_ADDR 1000
#define CFG_ADDR 300

struct led_t L1;   // RGB Light 1
struct led_t L2;   // RGB Light 2 

static unsigned long TimeNow;
static uint16_t crc1, crc2;
static int def_config1 = 1;
static int def_config2 = 1;
static char commandbuffer[128];
static uint8_t cmd_len = 0;

void test_setup( void );
void p( char *fmt, ... );
int8_t get_step_value( uint8_t start, uint8_t stop, uint16_t freq);
void set_led(struct led_t *l, struct color_t c);
uint8_t CRC8(uint8_t *message, uint8_t length);
int16_t load_cfg( uint8_t no, struct led_t *d );
int16_t save_cfg( uint8_t no, struct led_t *d );
uint8_t get_user_input( void );
uint8_t parse_command( void );
void dump_config( struct led_t t );
int16_t clear_cfg( void );
void print_help( void );

void setup() {
  int ok;
  // load or initialize LED configuration structures

  BZERO(&L1, sizeof(L1));
  BZERO(&L2, sizeof(L2));
  
  // setup the LED pin assignments for L1, and L2
  L1.r_pin = 3;
  L1.g_pin = 5;
  L1.b_pin = 6;
  L2.r_pin = 9;
  L2.g_pin = 10;
  L2.b_pin = 11;

  // set pins to output mode
  pinMode(L1.r_pin, OUTPUT);
  pinMode(L1.g_pin, OUTPUT);
  pinMode(L1.b_pin, OUTPUT);   
  pinMode(L2.r_pin, OUTPUT);
  pinMode(L2.g_pin, OUTPUT);
  pinMode(L2.b_pin, OUTPUT);

  L1.timer = millis();
  L2.timer = L1.timer;
  TimeNow = L1.timer;

  // start the serial port
  Serial.begin( 57600 );
  while( !Serial ) {
    delay( 1000 );
  }
  p("reset...\n");
  
}



void loop() {
  int8_t ok = 0;
  int8_t status;

  ok = load_cfg(1, &L1);
  if( ok == 0 ) def_config1 = 0;
  ok = load_cfg(2, &L2);
  if( ok == 0 ) def_config2 = 0;
  
  if( def_config1 == 1)p("LED1 default config\n\r");
  else{
    p("LED1 config loaded from EEPROM\n");
  }
  if( def_config2 == 1)p("LED2 default config\n\r");
  else{
    p("LED2 config loaded from EEPROM\n");
  }
  
  test_setup();
  // put your main code here, to run repeatedly: 
  while(1){
    TimeNow = millis();
    if( get_user_input() > 0 ){
      if( cmd_len > MIN_CMD_LEN ) {
        p("\n");
        //p("user input: %s\n", commandbuffer);
        status = parse_command();
        p("\n gm> ");
      } else {
        p("cmd %s err len\n", commandbuffer);
        p("\n gm> ");
      }
      BZERO(commandbuffer, sizeof(commandbuffer));
      cmd_len = 0;
    }
    if( L1.repeat == 0 ){
      led_run(&L1);
    } else {
      if ( L1.counter < L1.repeat ) {
        led_run(&L1);
      } 
    }
    if( L2.repeat == 0 ) {
      led_run(&L2);
    } else {
      if ( L2.counter < L2.repeat ) {
        led_run(&L2);
      }
    }
    delay(20);
  }
  
}

/*************************************************************************************************************
 *               UTILITY FUNCTIONS BELOW THIS COMMENT BLOCK
 ************************************************************************************************************/

void test_setup( void )
{
  if( def_config1 == 1 ){
    L1.mode = MODE_FADE;
  //  L1.mode = MODE_BLINK;
  //  L1.mode = MODE_ON;
  //  L1.mode = MODE_OFF;
    L1.start.r = 255;
    L1.start.g = 255;
    L1.start.b = 255;
    L1.freq = 2000;
    L1.repeat = 5;
    L1.r_step = get_step_value(L1.start.r, L1.end.r, L1.freq);
    L1.g_step = get_step_value(L1.start.g, L1.end.g, L1.freq);
    L1.b_step = get_step_value(L1.start.b, L1.end.b, L1.freq);
    L1.counter = 0;
  }
  if( def_config2 == 1 ){
  //  L2.mode = MODE_FADE;
  //  L2.mode = MODE_BLINK;
  // L2.mode = MODE_ON;
    L2.mode = MODE_OFF;
    L2.start.r = 255;
    L2.start.g = 255;
    L2.start.b = 255;
    L2.freq = 5000;
    L2.repeat = 5;
    L2.r_step = get_step_value(L2.start.r, L2.end.r, L2.freq);
    L2.g_step = get_step_value(L2.start.g, L2.end.g, L2.freq);
    L2.b_step = get_step_value(L2.start.b, L2.end.b, L2.freq);
    L2.counter = 0;
  }

  L1.timer = millis();
  L2.timer = L1.timer;
  TimeNow = L1.timer;
  set_led(&L1, L1.start);
  set_led(&L2, L2.start);

  return;
}

void p( char *fmt, ... )
{
  char buf[255]; // resulting string limited to 255 chars
  va_list args;
  va_start ( args, fmt );
  memset( buf, 0, sizeof( buf ) );
  vsnprintf( buf, 255, fmt, args );
  va_end ( args );
  Serial.print( buf );
}

int8_t get_step_value( uint8_t start, uint8_t stop, uint16_t freq)
{
//  if( stop > start ){
//    return (int8_t)(freq / (stop - start));
//  } else {
//    return (int8_t)(freq / (start - stop));
//  }
//return (int8_t)(freq / (stop - start));
//  return (int8_t)((stop - start) / MAX_STEPS );
  if( stop > start ){
    return (int8_t)((stop - start) / MAX_STEPS );
  } else {
    return (int8_t)((start - stop) / MAX_STEPS );
  }
}

void set_led(struct led_t *l, struct color_t c)
{
  l->curr.r = c.r;
  l->curr.g = c.g;
  l->curr.b = c.b;
  analogWrite(l->r_pin, l->curr.r );
  analogWrite(l->g_pin, l->curr.g );
  analogWrite(l->b_pin, l->curr.b );
  l->timer = millis();
  return;
}

void led_run( struct led_t *led )
{
  struct color_t tmp_c;
  unsigned long time_passed = 0;
  BZERO(&tmp_c, sizeof(tmp_c));
  
  switch(led->mode)
  {
    case MODE_OFF:
      // turn the LED off
      set_led(led, tmp_c);
      //p("off\n");
    break;
    case MODE_ON:
      // set the led to the start color
      // memcpy(&tmp_c, &led->start, sizeof(struct color_t));
      set_led(led, led->start);
      //p("on %d,%d,%d\n", led->start.r, led->start.g, led->start.b);
    break;
    case MODE_BLINK:
      time_passed = TimeNow - led->timer;
      if ( time_passed >= ( led->freq / 2 ) ) {
        // figure out if we need to turn the LED on or off
        if( led->curr.r == led->start.r ) {
          set_led(led, led->end);
          //p("blink off\n");
          led->counter++;
          //p("blink count %d\n", led->counter);
        } else {
          set_led(led, led->start);
          //p("blink on\n");
        }
      }
    break;
    case MODE_FADE:
      time_passed = TimeNow - led->timer;
      if( time_passed >= ( led->freq / MAX_STEPS)){
      //if ( TimeNow >= ( led->timer + ( led->freq / MAX_STEPS ) ) ) {
#if 0
        p("\nTimeNow:%d\n", TimeNow);
        p("freq:%d\n", led->freq);
        p("timer:%d\n", led->timer);
        p("rstep:%d\n", led->r_step);
        p("gstep:%d\n", led->g_step);
        p("bstep:%d\n", led->b_step);
        p("max steps:%d\n", MAX_STEPS);
#endif
        
        //if( led->curr.r == led->end.r )set_led(led, led->start);
        //else {
          // step the LED to the next fade value
          if( led->start.r < led->end.r ){
            tmp_c.r = led->curr.r + led->r_step;
          } else {
            tmp_c.r = led->curr.r - led->r_step;
          }
          if( led->start.g < led->end.g ){
            tmp_c.g = led->curr.g + led->g_step;
          } else {
            tmp_c.g = led->curr.g - led->g_step;
          }
          if( led->start.b < led->end.b ){
            tmp_c.b = led->curr.b + led->b_step;
          } else {
            tmp_c.b = led->curr.b - led->b_step;
          }
          //p("tmp_c.r=%d, led->end.r=%d, step=%d\n", tmp_c.r, led->end.r, led->r_step);

          if( led->start.r < led->end.r ){
            //if(tmp_c.r > led->r_step){
            if( tmp_c.r > ( led->end.r - led->r_step )){
              led->counter++;
              //p("fade count %d\n", led->counter);
              set_led(led, led->end);
            } else {
              set_led(led, tmp_c);
            }
          } else {
            if(tmp_c.r < led->r_step){
              led->counter++;
              //p("fade count %d\n", led->counter);
              set_led(led, led->end);
            } else {
              set_led(led, tmp_c);
            }
          }
        //}
      }
    break;
//    case MODE_PULSE:
//
//    break;
    default:
    break;
  }
  return;
}

int16_t load_cfg( uint8_t no, struct led_t *d )
{
  uint8_t my_crc;
  uint16_t i;
  uint8_t c[ CFG_SIZE + 1];
  uint16_t loadaddr = CFG_ADDR * no;

  //p("load cfg from addr %d\n", loadaddr);
  BZERO(c, sizeof(c));
  for(i=0;i<=CFG_SIZE;i++){
    c[i] = EEPROM.read(loadaddr + i);
    //p("%02X ", c[i]);
    //delay( SHORT_DELAY );
  }
  my_crc = CRC8((uint8_t *)&c, ( CFG_SIZE - (sizeof(uint8_t))));
  //memcpy(d, c, CFG_SIZE);
  if( my_crc != c[CFG_SIZE - 1] ){
    crc1 = my_crc;
    crc2 = c[CFG_SIZE - 1];
    p("crc err: %d != %d\n", crc1, crc2);
    return -1;
  }
  memcpy(d, c, CFG_SIZE);
  return 0;
}

int16_t save_cfg( uint8_t no, struct led_t *d )
{
  uint16_t i;
  uint8_t c[ CFG_SIZE ];
  uint16_t loadaddr = CFG_ADDR * no;

  //p("save cfg to addr %d\n", loadaddr);
  d->counter = 0;
  d->timer = 0;
  d->dcrc = CRC8((uint8_t *)d, ( CFG_SIZE - (sizeof(uint8_t))));
  memcpy(c, d, CFG_SIZE);
  for(i=0;i<=CFG_SIZE;i++){
    //p("%02X ", c[i]);
    EEPROM.write((CFG_ADDR * no ) + i ,c[i]);
    //delay( 10 );
  }
  //p("save_cfg crc = %d\n", d->dcrc);
  //d->timer=millis();
  return 0;
}

int16_t clear_cfg( void )
{
  uint16_t i;
  uint8_t c[ CFG_SIZE ];

  p("clear programs\n");
  memset(c, 0xFF, CFG_SIZE);
  for(i=0;i<=CFG_SIZE;i++){
    EEPROM.write((CFG_ADDR * 1 ) + i ,c[i]);
    //delay( SHORT_DELAY );
  }
  for(i=0;i<=CFG_SIZE;i++){
    EEPROM.write((CFG_ADDR * 2 ) + i ,c[i]);
    //delay( SHORT_DELAY );
  }
  
  return 0;
}

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

uint8_t get_user_input( void )
{
  //static int i=0;
  char b = 0;

  if( Serial.available() ) {
    b = Serial.read();
    if( b != 0x08 ){
      commandbuffer[cmd_len++] = b;
    } else {
      commandbuffer[cmd_len] = '\0';
      cmd_len--;
    }
    if( ( b == 0x0a ) || ( b== 0x0d ) || ( b == CMD_DELIM) ) {
      if( ( b == 0x0a ) || ( b== 0x0d ) ){
        commandbuffer[cmd_len]=CMD_DELIM;
      }
      return cmd_len;
    } else {
      Serial.write( b );
    }
  }
  return 0;
}

uint8_t parse_command( void )
{
  int8_t ok = 0;
  uint8_t i;
  uint8_t x;
  uint8_t buf[32];
  uint8_t var;
  uint16_t var2;
  uint8_t ledno = 0;
  struct led_t *pled = NULL;
  uint8_t program_flag = 0;

  for(i=0;i<cmd_len;i++) {
    switch( commandbuffer[i] ) {
      case 'p':
        program_flag = 1;
//        p("program flag set\n\r", var);
      break;
      case 'l':
        BZERO(buf, sizeof(buf));
        // move to next char
        i++;
        var=commandbuffer[i] - 48;
        if( var == 1 ){
          pled = &L1;
          ledno = 1;
        } else if( var == 2 ){
          pled = &L2;
          ledno = 2;
        } else {
          p("parse err led no\n");
          break;
        }
//        if( pled != NULL ){
//          p("led %d selected\n\r", var);
//        }
        i++;
      break;
      case 'x':
        if( pled != NULL ){
            pled->mode = MODE_OFF;
        } else {
          p("parse error blink - no led selected\n");
          break;
        }
      break;
      case '+':
        if( pled != NULL ){
            pled->mode = MODE_ON;
        } else {
          p("parse error blink - no led selected\n");
          break;
        }
        i++;
        BZERO(buf, sizeof(buf));
        x = 0;
        while( commandbuffer[i] != ',' ){
          buf[x++] = commandbuffer[i++];
          if( x == sizeof(buf)){
            p("parse error blink color values\n");
            break;
          }
        }
        var = (uint8_t)atoi((const char *)(buf));
        pled->start.r = var;
        //p("pled->start.r = %d\n", var);
        
        i++;
        BZERO(buf, sizeof(buf));
        x = 0;
        while( commandbuffer[i] != ',' ){
          buf[x++] = commandbuffer[i++];
          if( x == sizeof(buf)){
            p("parse error blink color values\n");
            break;
          }
        }
        var = (uint8_t)atoi((const char *)(buf));
        pled->start.g = var;
        //p("pled->start.g = %d\n", var);

        i++;
        BZERO(buf, sizeof(buf));
        x = 0;
        while( commandbuffer[i] != ';' ){
          buf[x++] = commandbuffer[i++];
          if( x == sizeof(buf)){
            p("parse error blink color values\n");
            break;
          }
        }
        var = (uint8_t)atoi((const char *)(buf));
        pled->start.b = var;
        //p("pled->start.b = %d\n", var);
      break;
      case 'b':
      case 't':
        if( pled != NULL ){
          if( commandbuffer[i] == 'b' ){
            p("blink led %d ", ledno);
            pled->mode = MODE_BLINK;
          } else {
            p("transition led %d ", ledno);
            pled->mode = MODE_FADE;
          }
        } else {
          p("parse error blink - no led selected\n");
          break;
        }
        i++;
        BZERO(buf, sizeof(buf));
        x = 0;
        while( commandbuffer[i] != ',' ){
          buf[x++] = commandbuffer[i++];
          if( x == sizeof(buf)){
            p("parse error blink color values\n");
            break;
          }
        }
        var = (uint8_t)atoi((const char *)(buf));
        pled->start.r = var;
        //p("pled->start.r = %d\n", var);
        
        i++;
        BZERO(buf, sizeof(buf));
        x = 0;
        while( commandbuffer[i] != ',' ){
          buf[x++] = commandbuffer[i++];
          if( x == sizeof(buf)){
            p("parse error blink color values\n");
            break;
          }
        }
        var = (uint8_t)atoi((const char *)(buf));
        pled->start.g = var;
        //p("pled->start.g = %d\n", var);

        i++;
        BZERO(buf, sizeof(buf));
        x = 0;
        while( commandbuffer[i] != ',' ){
          buf[x++] = commandbuffer[i++];
          if( x == sizeof(buf)){
            p("parse error blink color values\n");
            break;
          }
        }
        var = (uint8_t)atoi((const char *)(buf));
        pled->start.b = var;
        //p("pled->start.b = %d\n", var);

        i++;
        BZERO(buf, sizeof(buf));
        x = 0;
        while( commandbuffer[i] != ',' ){
          buf[x++] = commandbuffer[i++];
          if( x == sizeof(buf)){
            p("parse error blink color values\n");
            break;
          }
        }
        var = (uint8_t)atoi((const char *)(buf));
        pled->end.r = var;
        //p("pled->end.r = %d\n", var);

        i++;
        BZERO(buf, sizeof(buf));
        x = 0;
        while( commandbuffer[i] != ',' ){
          buf[x++] = commandbuffer[i++];
          if( x == sizeof(buf)){
            p("parse error blink color values\n");
            break;
          }
        }
        var = (uint8_t)atoi((const char *)(buf));
        pled->end.g = var;
        //p("pled->end.g = %d\n", var);

        i++;
        BZERO(buf, sizeof(buf));
        x = 0;
        while( commandbuffer[i] != ',' ){
          buf[x++] = commandbuffer[i++];
          if( x == sizeof(buf)){
            p("parse error blink color values\n");
            break;
          }
        }
        var = (uint8_t)atoi((const char *)(buf));
        pled->end.b = var;
        //p("pled->end.b = %d\n", var);

        i++;
        BZERO(buf, sizeof(buf));
        x = 0;
        while( commandbuffer[i] != ',' ){
          buf[x++] = commandbuffer[i++];
          if( x == sizeof(buf)){
            p("parse error blink color values\n");
            break;
          }
        }
        //var2 = atoi((const char *)(buf));
        var2 = strtol((const char *)buf, NULL, 10);
        pled->freq = var2;
        //p("pled->freq = %d\n", var2);

        i++;
        BZERO(buf, sizeof(buf));
        x = 0;
        while( commandbuffer[i] != ';' ){
          buf[x++] = commandbuffer[i++];
          if( x == sizeof(buf)){
            p("parse error blink color values\n");
            break;
          }
        }
        var2 = atoi((const char *)(buf));
        pled->repeat = var2;

        pled->r_step = get_step_value(pled->start.r, pled->end.r, pled->freq);
        pled->g_step = get_step_value(pled->start.g, pled->end.g, pled->freq);
        pled->b_step = get_step_value(pled->start.b, pled->end.b, pled->freq);
        
        //p("pled->repeat = %d\n", var2);
        p("from (%d,%d,%d) to (%d,%d,%d) / %dms ", pled->start.r,pled->start.g, pled->start.b,
                                                      pled->end.r, pled->end.g,pled->end.b,
                                                      pled->freq);
        if( pled->repeat == 0 ){
          p("forever\n");
        } else {
          p("%d times\n", pled->repeat);
        }
        set_led(pled, pled->start);
      break;
      case 's':
        if( ( commandbuffer[i+1] == 'y' ) &&
            ( commandbuffer[i+2] == 'n' ) &&
            ( commandbuffer[i+3] == 'c' ) ) {
              set_led(&L1, L1.start);
              set_led(&L2, L2.start);
              L1.timer = millis();
              L2.timer = L1.timer;
              p("sync led timers\n");
              i+=4;
            }
        else if( ( commandbuffer[i+1] == 'a' ) &&
            ( commandbuffer[i+2] == 'v' ) &&
            ( commandbuffer[i+3] == 'e' ) ) {
              save_cfg(1,&L1);
              save_cfg(2,&L2);
              p("save led programs\n");
              i+=4;
            }     
        else if( ( commandbuffer[i+1] == 'h' ) &&
             ( commandbuffer[i+2] == 'o' ) &&
             ( commandbuffer[i+3] == 'w' ) ) {
              BZERO(buf, sizeof(buf));
              sprintf((char *)buf, "l1,");
              switch(L1.mode){
                case MODE_OFF:
                  sprintf((char *)buf+3,"x;\n");
                break;
                case MODE_ON:
                  sprintf((char *)buf+3,"+%d,%d,%d;\n", L1.start.r, L1.start.g, L1.start.b);
                break;
                case MODE_BLINK:
                case MODE_FADE:
                  sprintf((char *)buf+3,"b%d,%d,%d,%d,%d,%d,%d,%d;\n", L1.start.r, L1.start.g, L1.start.b,
                                                      L1.end.r, L1.end.g, L1.end.b,
                                                      L1.freq, L1.repeat);
                break;
              }
              p("%s", buf);
              BZERO(buf, sizeof(buf));
              sprintf((char *)buf, "l2,");
              switch(L2.mode){
                case MODE_OFF:
                  sprintf((char *)buf+3,"x;\n");
                break;
                case MODE_ON:
                  sprintf((char *)buf+3,"+%d,%d,%d;\n", L2.start.r, L2.start.g, L2.start.b);
                break;
                case MODE_BLINK:
                case MODE_FADE:
                  sprintf((char *)buf+3,"b%d,%d,%d,%d,%d,%d,%d,%d;\n", L2.start.r, L2.start.g, L2.start.b,
                                                      L2.end.r, L2.end.g, L2.end.b,
                                                      L2.freq, L2.repeat);
                break;
              }
              p("%s", buf);
              i+=4;
            } 
      break;
      case 'h':
        if( ( commandbuffer[i+1] == 'e' ) &&
            ( commandbuffer[i+2] == 'l' ) &&
            ( commandbuffer[i+3] == 'p' ) ) {
              print_help();
              i+=4;
            }
      break;
      case 'r':
        if( ( commandbuffer[i+1] == 'e' ) &&
            ( commandbuffer[i+2] == 's' ) &&
            ( commandbuffer[i+3] == 'e' ) &&
            ( commandbuffer[i+4] == 't' ) ) {
              L1.timer = millis();
              L2.timer = L1.timer;
              L1.counter = 0;
              L2.counter = 0;
              p("reset program counters\n");
              i+=5;
            }
      break;
      case 'D':
        if( commandbuffer[i+1] == 'D' ) {
          p("dump config L1\n");
          dump_config(L1);
          p("\n");
          i+=2;
        }
      break;
      case 'X':
        if( ( commandbuffer[i+1] == 'X' ) &&
        ( commandbuffer[i+2] == 'X' )) {
          p("clear config");
          clear_cfg();
          p("\n");
          i+=3;
        }
      break;
    }
  }

  if( program_flag == 1 ){
    if( pled != NULL ){
      save_cfg(ledno, pled);
      p("\n");
      dump_config(*pled);
      p("\nprogram saved to eeprom\n");
    } else {
      p("save config error ledno\n");
    }
  }
  pled->counter = 0;
  pled->timer = millis();
  return ok;
}

void dump_config( struct led_t t ){

#if 0
  int i;
  uint8_t buf[ CFG_SIZE ];
  memcpy(buf, &t, sizeof(struct led_t));
  for(i=0;i<sizeof(struct led_t);i++){
    p("%02X ", (uint8_t )buf[i]);
  }
#endif
  p("mode: %d\n", t.mode);
  p(" s.r: %d\n", t.start.r);
  p(" s.g: %d\n", t.start.g);
  p(" s.b: %d\n", t.start.b);
  p(" e.r: %d\n", t.end.r);
  p(" e.g: %d\n", t.end.g);
  p(" e.b: %d\n", t.end.b);
  p("rstp: %d\n", t.r_step);
  p("gstp: %d\n", t.g_step);
  p("bstp: %d\n", t.b_step);
  p("freq: %d\n", t.freq);
  p("rept: %d\n", t.repeat);
  return;
}

void print_help( void )
{
  p("Use:\n");
  p("(p)l#,<mode>(args)\n");
  p("p  - program flag\n");
  p("l# - led no 1 or 2\n");
  p("modes:\n");
  p("  bsr,sb,sg,er,eg,eb,freq,repeat = blink\n");
  p("  tsr,sb,sg,er,eg,eb,freq,repeat = transition\n");
  p("  +r,g,b = on color\n");
  p("  x      = led off\n");
  p("reset    = reset program\n");
  p("XXX      = clear config\n");
  return ;
}
