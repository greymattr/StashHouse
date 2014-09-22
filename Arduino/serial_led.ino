 /***********************************************************************
 *  Copyright (c) 2014 - Matthew Fatheree <greymattr@gmail.com>
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES.  THE USER(S) ARE HEREBY NOTIFIED THAT USE OF THIS SOFTWARE MAY 
 * MAY CAUSE LOSS OF DATA, DATA DESTRUCTION, HEALTH AND MENTAL PROBLEMS. IT 
 * MAY CAUSE YOU TO START DRINKING, INCREASE YOUR STUPIDITY LEVEL, CLUB 
 * BABY SEALS, AND OVERALL BECOME A BAD PERSON.
 *
 * PLEASE USE AT YOUR OWN RISK
 * ____    ________
 * \   \  /    ___/
 * |    \/     __|
 * |  |\__/|  |
 * /  \atthew \atheree  
 ************************************************************************/

#include <stdarg.h>

#define	min(a,b)	  ((a) < (b) ? (a) : (b))
#define	max(a,b)	  ((a) > (b) ? (a) : (b))

#define SET_BIT(p,n)      ((p) |= (1 << (n)))
#define CLR_BIT(p,n)      ((p) &= (~(1) << (n)))
#define IS_BIT_SET(val, bit_no) (((val) >> (bit_no)) & 1)

#define INTENSE_MIN        0
#define INTENSE_MAX        255


//#define FADE_STEPS         8
#define FADE_STEPS         16
//#define FADE_STEPS         32

#define SL_VERSION         "v0.1.1"



#define BORDER             "\n--------------------\n"
#define LED_CHOICE         "Which LED would you like to"
#define FREQ_CHOICE        "How quickly would you like it to"
#define REPEAT_CHOICE      "How many times should it"
#define INTENSE_CHOICE     "How bright"
#define MODE_SEL_ERR       "mode select error\n"
#define PARAM_ERR          "parameter error\n"

#define HIGH_VAL_STR       "High Value"
#define MID_VAL_STR        "Mid Value"
#define LOW_VAL_STR        "Low Value"

#define CONTROL_SETTINGS   "Control Settings"
#define START_INTS         "Start Intensity"
#define END_INTS           "End Intensity"
#define TXT_MACRO          "\n\n macro = "


#define MAX_MACRO_LEN      128
#define SMALL_BUFF         8

#define LED1_FLAG          1
#define LED2_FLAG          2
#define LED3_FLAG          4
#define LED4_FLAG          8
#define LED5_FLAG          16
#define LED6_FLAG          32
#define LED7_FLAG          64
#define LED8_FLAG          128
#define LED9_FLAG          256
#define LED10_FLAG         512
#define LED11_FLAG         1024
#define LED12_FLAG         2048
#define LED13_FLAG         4096

//macro modes
#define MODE_BLINK         1
#define MODE_PULSE         2
#define MODE_FADE          3
#define MODE_WAIT          4
#define MODE_SET           5

#define MAX_LEDS           13
#define ALL_LEDS           8191
#define NONE_LEDS          0

#define POWER_SAVE

int HighVal = INTENSE_MAX;
int MidVal = INTENSE_MAX / 2;
int LowVal = INTENSE_MIN;
int UserLed = 10;
int UserVal = INTENSE_MAX;
int UserFreq = 1000;
int UserRepeat = 1;
int UserState1 = LowVal;
int UserState2 = HighVal;
char Uinput[ MAX_MACRO_LEN ];

void setup()
{
  int x = 0;

  for( x=0; x<=MAX_LEDS; x++ ) {
    pinMode( UserLed,OUTPUT );    // Configure the onboard LED for output
    analogWrite( UserLed,0 );  // default to LED off
  }
  Serial.begin( 57600 );
  while( !Serial ) {
    delay( 500 );
  }
  delay( 1000 ); // because Arduino,... am I right?...

}

void p( char *fmt, ... )
{
  char buf[MAX_MACRO_LEN]; // resulting string limited to 128 chars
  va_list args;
  va_start ( args, fmt );
  memset( buf, 0, sizeof( buf ) );
  vsnprintf( buf, MAX_MACRO_LEN, fmt, args );
  va_end ( args );
  Serial.print( buf );
}

void display_menu()
{
  delay( 500 );
  p( "\n\n    LED %s %s\n", CONTROL_SETTINGS, SL_VERSION );
  p( " I. Intensity %s\n", CONTROL_SETTINGS );
  p( " F. Fade  %s\n", CONTROL_SETTINGS );
  p( " P. Pulse %s\n", CONTROL_SETTINGS );
  p( " B. Blink %s\n", CONTROL_SETTINGS );
  p( " C. Clear all LEDS off\n" );
#if 0
  p( " M. Macro %s\n", CONTROL_SETTINGS );
#endif
  p( "%s", BORDER);
  p( " > " );
}

void loop()
{
  char *selection;
  int a = 0;
  int ok;

  display_menu();
  selection = get_user_input();
  memset( Uinput, 0, sizeof( Uinput ) );
  for( a=0; a<( MAX_MACRO_LEN-1 ); a++ ) {
    Uinput[a] = ( char ) *( selection + a );
    if( Uinput[a] == '\0' ) {
      break;
    }
  }
  switch ( Uinput[0] ) {
  case 'I':
  case 'i':
    IntensityControls();
    break;
  case 'F':
  case 'f':
    FadeControls();
    break;
  case 'P':
  case 'p':
    PulseControls();
    break;
  case 'B':
  case 'b':
    BlinkControls();
    break;
  case 'C':
  case 'c':
    p( "\n All LEDS OFF\n\n" );
    a = 8191;
    p("%s c\n", TXT_MACRO);
    set_leds( a, 0 );
    break;
  case '*':
    SayHello();
    break;
#if 1
  case 't':
    // put test functions here
    unrecognized();
    break;
  case 'M':
  case 'm':
    //unrecognized();
    parse_macro( ( char * )( Uinput + 1 ) );
    break;
#endif
  default:
    unrecognized();
    break;
  }

}


void set_leds( int leds, int brightness )
{
  int x;
  //p("\nset led(s) %d at %d\n", leds, brightness);
  for( x=1; x<=MAX_LEDS; x++ ) {
    if( IS_BIT_SET( leds, x ) ) {
      analogWrite( x,brightness );
    }
  }
}

void leds_off( int leds )
{
  set_leds( leds,0 );
}

void leds_on( int leds )
{
  set_leds( leds,255 );
}

void blink_leds( int leds, int brightness, int freq, int repeat )
{
  int i = 0, x = 0;
  int led;

  set_leds( leds, 0 );
  //p( "\n blink led(s) %d at %d/%dms %d times\n", leds, brightness, freq, repeat );
  for( i=0; i<repeat; i++ ) {
    set_leds( leds, brightness );
    delay( freq / 2 );
    set_leds( leds, 0 );
    delay( freq/2 );
  }
}

void blink_led( int led, int b, int f, int r )
{
  int ls = 0;
  SET_BIT( ls, led );
  blink_leds( ls, b, f, r );
}

void fade_leds( int leds, int from, int to, int freq, int repeat )
{
  int total_time = freq;
  int steps = FADE_STEPS;
  int total_range = ( ( max( from,to ) ) - min( from,to ) );
  int step_value =  ( total_range / steps );
  int sleep_value = ( total_time / steps );
  int i, y;
  int curval;
  int led = 0;

  //p( "\n fade led(s) %d from %d to %d/%dms %d times\n", leds, from, to, freq, repeat );
  for( i=0; i<repeat; i++ ) {
    curval = from;
    for ( y=0; y<=steps; y++ ) {
      if( curval < 0 ) {
        curval = 0;
      }
      set_leds( leds, curval );
      if( curval > to ) {
        curval = curval - step_value;
      } else {
        curval = curval + step_value;
      }
      delay( sleep_value );
    }
  }
  set_leds( leds, to );
}

void fade_led( int leds, int from, int to, int freq, int repeat )
{
  int ls = 0;
  SET_BIT( ls, leds );
  fade_leds( ls, from, to, freq, repeat );
}


void pulse_leds( int leds, int from, int to, int freq, int repeat )
{
  int i;
  int new_freq = freq / 2;
  for( i=0; i<repeat; i++ ) {
    fade_leds( leds, from, to, new_freq, 1 );
    fade_leds( leds, to, from, new_freq, 1 );
  }

}

void pulse_led( int led, int from, int to, int freq, int repeat )
{
  int ls = 0;
  SET_BIT( ls, led );
  pulse_leds( ls, from, to, freq, repeat );
}

void SayHello()
{
  p( "%s", BORDER );
  p( "\n Hey Matt... greymattr want a beer?... You're awesome!...\n" );
  p( "%s", BORDER );
}

char *get_user_input()
{
  int i=0;
  char b = 0;
  char commandbuffer[MAX_MACRO_LEN];

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
  if( i<2 ) {
    return "\0";
  }
  commandbuffer[i]='\0';
  //p("\ncmd buffer = %s\n", commandbuffer);
  return commandbuffer;
}

int get_user_int( int orig )
{
  char *arg = '\0';
  int val = 0;
  arg = get_user_input();
  if( *arg != '\0' ) {
    val=atoi( arg );
    if( ( val >= INTENSE_MIN ) && ( val <= INTENSE_MAX ) ) {
      return val;
    } else {
      return orig;
    }
  }
  return orig;
}

int get_user_intlarge( int orig )
{
  char *arg = '\0';
  int val = 0;
  arg = get_user_input();
  if( *arg != '\0' ) {
    val=atoi( arg );
    if( ( val >= 0 ) && ( val <= 0xFFFF ) ) {
      return val;
    } else {
      return orig;
    }
  }
  return orig;
}

void BlinkControls()
{
  int aNumber;
  char resp;
  char *arg = '\0';
  int ok;
  int val;

  Serial.flush();
  p( "\n\nBlink %s\n", CONTROL_SETTINGS );
  p( "%s blink [ %d ]?: ", LED_CHOICE, UserLed );
  while( !Serial.available() ) {
    ;
  }
  UserLed = get_user_int( UserLed );
  p( "\n%s blink [ %dms ]?: ", FREQ_CHOICE, UserFreq );
  while( !Serial.available() ) {
    ;
  }
  UserFreq = get_user_intlarge( UserFreq );
  p( "\n%s (1-255) [ %d ]?: ", INTENSE_CHOICE, UserVal );
  while( !Serial.available() ) {
    ;
  }
  UserVal = get_user_int( UserVal );
  p( "\n%s should it blink [ %d ]?: ", REPEAT_CHOICE, UserRepeat );
  while( !Serial.available() ) {
    ;
  }
  UserRepeat = get_user_int( UserRepeat );
  p("%s ml(%d)b(%d,%d,%d)\n", TXT_MACRO,UserLed, UserVal, UserFreq, UserRepeat);
  blink_led( UserLed, UserVal, UserFreq, UserRepeat );
}

void IntensityControls()
{
  char *arg = NULL;
  int val = 0;
  int ok;

  Serial.flush();
  p( "\n\nIntensity %s ( 1 - 255 )\n", CONTROL_SETTINGS );
  p( "%s [ %d ]?: ",HIGH_VAL_STR, HighVal );
  while( !Serial.available() ) {
    ;
  }
  HighVal = get_user_int( HighVal );
  p( "\n %s [ %d ]?: ",MID_VAL_STR, MidVal );
  while( !Serial.available() ) {
    ;
  }
  MidVal = get_user_int( MidVal );
  p( "\n %s [ %d ]?: ",LOW_VAL_STR, LowVal );
  while( !Serial.available() ) {
    ;
  }
  LowVal = get_user_int( LowVal );
  p( "%s", BORDER );
  p( "%s=%d, %s=%d, %s=%d\n",HIGH_VAL_STR,HighVal,MID_VAL_STR, MidVal,LOW_VAL_STR, LowVal );
  p( "%s", BORDER );
  p( "\n%s set [ %d ]?: ", LED_CHOICE, UserLed );
  while( !Serial.available() ) {
    ;
  }
  UserLed = get_user_int( UserLed );
  p( "\n%s [ %d ]?: ", INTENSE_CHOICE, UserVal );
  while( !Serial.available() ) {
    ;
  }
  UserVal = get_user_intlarge( UserVal );
  p( "%s", BORDER );
  p( "\nLED %d set to %d\n", UserLed, UserVal );
  p("%s ml(%d)i(%d)\n", TXT_MACRO,UserLed, UserVal);
  analogWrite( UserLed, UserVal );
  p( "%s", BORDER );
}

void intensity_menu()
{
  p( "\n a. %s ( %d )\n", HIGH_VAL_STR, HighVal );
  p( " b.  %s ( %d )\n", MID_VAL_STR, MidVal );
  p( " c.  %s ( %d )\n", LOW_VAL_STR, LowVal );
}

void FadeControls()
{
  char *arg = '\0';
  int state1 = UserState1;
  int state2 = UserState2;

  Serial.flush();
  p( "\n\nFade %s\n", CONTROL_SETTINGS );
  p( "%s fade [ %d ]?: ", LED_CHOICE, UserLed );
  while( !Serial.available() ) {
    ;
  }
  UserLed = get_user_int( UserLed );
  intensity_menu();
  p( "%s [ %d ]?: ", START_INTS, UserState1 );
  while( !Serial.available() ) {
    ;
  }
  arg = get_user_input();
  switch( *arg ) {
  case 'A':
  case 'a':
    UserState1 = HighVal;
    break;
  case 'B':
  case 'b':
    UserState1 = MidVal;
    break;
  case 'C':
  case 'c':
    UserState1 = LowVal;
    break;
  default:
    state1 = atoi( arg );
    if( ( state1 > 255 ) || ( state1 < 0 ) ) {
      state1=UserState1;
    } else {
      state1 = UserState1;
    }
    break;
  }
  intensity_menu();
  p( "%s [ %d ]?: ",END_INTS, UserState2 );
  while( !Serial.available() ) {
    ;
  }
  arg = get_user_input();
  switch( *arg ) {
  case 'A':
  case 'a':
    state2 = HighVal;
    break;
  case 'B':
  case 'b':
    state2 = MidVal;
    break;
  case 'c':
  case 'C':
    state2 = LowVal;
    break;
  default:
    state2 = atoi( arg );
    if( ( state2 > 255 ) || ( state2 < 0 ) ) {
      state2=UserState2;
    } else {
      state2 = UserState2;
    }
    break;
  }
  UserState2 = state2;
  p( "\n%s fade [ %d ]?: ", FREQ_CHOICE, UserFreq );
  while( !Serial.available() ) {
    ;
  }
  UserFreq = get_user_intlarge( UserFreq );
  p( "\n%s fade [ %d ]?: ", REPEAT_CHOICE, UserRepeat );
  while( !Serial.available() ) {
    ;
  }
  UserRepeat = get_user_int( UserRepeat );
  p("%s ml(%d)f(%d,%d,%d,%d)\n", TXT_MACRO,UserLed, UserState1, UserState2, UserFreq, UserRepeat);
  fade_led( UserLed, UserState1, UserState2, UserFreq, UserRepeat );
}

void PulseControls()
{
  char *arg = '\0';
  int sel = 0;
  int state1, state2;
  int ok;

  Serial.flush();
  p( "\n\nPulse %s\n", CONTROL_SETTINGS );
  p( "%s Pulse [ %d ]?: ",LED_CHOICE, UserLed );
  while( !Serial.available() ) {
    ;
  }
  UserLed = get_user_int( UserLed );
  intensity_menu();
  p( "%s [ %d ]?: ", START_INTS, UserState1 );
  while( !Serial.available() ) {
    ;
  }
  arg = get_user_input();
  switch( arg[0] ) {
  case 'a':
  case 'A':
    state1 = HighVal;
    break;
  case 'b':
  case 'B':
    state1 = MidVal;
    break;
  case 'c':
  case 'C':
    state1 = LowVal;
    break;
  default:
    state1 = atoi( arg );
    if( ( state1 > 255 ) || ( state1 < 0 ) ) {
      state1=UserState1;
    } else {
      state1 = UserState1;
    }
    break;
  }
  UserState1 = state1;
  p( "\n%s %d\n", START_INTS, UserState1 );
  intensity_menu();
  p( "%s [ %d ]?: ",END_INTS, UserState2 );
  while( !Serial.available() ) {
    ;
  }
  arg = get_user_input();
  switch( arg[0] ) {
  case 'a':
  case 'A':
    state2 = HighVal;
    break;
  case 'b':
  case 'B':
    state2 = MidVal;
    break;
  case 'c':
  case 'C':
    state2 = LowVal;
    break;
  default:
    state2 = atoi( arg );
    if( ( state2 > 255 ) || ( state2 < 0 ) ) {
      state2=UserState2;
    } else {
      state2 = UserState2;
    }
    break;
  }
  UserState2 = state2;
  p( "\n%s pulse [ %d ]?: ", FREQ_CHOICE, UserFreq );
  while( !Serial.available() ) {
    ;
  }
  UserFreq=get_user_intlarge( UserFreq );
  p( "\n%s pulse [ %d ]?: ", REPEAT_CHOICE, UserRepeat );
  while( !Serial.available() ) {
    ;
  }
  UserRepeat=get_user_int( UserRepeat );
  p("%s ml(%d)p(%d,%d,%d,%d)\n", TXT_MACRO,UserLed, UserState1, UserState2, UserFreq, UserRepeat);
  pulse_led( UserLed, UserState1, UserState2, UserFreq, UserRepeat );
}

int parse_macro( char *macro )
{
  int ok = 0;
  long int leds = 0;
  char *ch;
  char buf[ SMALL_BUFF ];
  int val1 = -1;
  int val2 = -1;
  int freq = -1;
  int repeat = -1;
  int wait = -1;
  int state = 0;
  int i, x;
  int scratch = 0;
  int mode = 0;

  set_leds( ALL_LEDS, 0 );
  p( "\nRun Macro [ %s ]\n", macro-1 );
  for( i=0; i<( MAX_MACRO_LEN - 1 ); i++ ) {
    ch = ( char * )( macro + i );
    switch( *ch ) {
    case 'l':
    case 'L':
      leds = 0;
      if( *( ch + 1 ) == '(' ) {
        i++;  // skip char
        memset( buf, 0, sizeof( buf ) );
        for( x=0; x<sizeof( buf ); x++ ) {
          i++;
          ch = ( char * )( macro + i );
          switch( *ch ) {
          case ',':
          case ')':
            scratch = atoi( buf );
            //p("got led #%d\n", scratch);
            SET_BIT( leds, scratch );
            if( *ch == ')' ) {
              for( x=0; x<MAX_LEDS; x++ ) {
                if( IS_BIT_SET( leds, x ) ) {
                  p( "+ LED #%d\n", x );
                }
              }
              x = sizeof( buf ); // stop here
            } else {
              memset( buf, 0, sizeof( buf ) );
              x = -1;
            }
            break;
          default:
            buf[x] = *ch;
            //p("buf[%d] = %c\n", x, *ch);
            break;
          }
        }
      } else {
        p( "LED err: %s", PARAM_ERR );
        ok = -1;
      }
      //leds = 0;
      val1 = -1;
      val2 = -1;
      freq = -1;
      wait = - 1;
      repeat = -1;
      mode = 0;
      break;
    case 'b':
    case 'B':
      i++;
      if( mode == 0 ) {
        p("\nBlink\n");
        mode = MODE_BLINK;
      } else {
        p( "%s",MODE_SEL_ERR );
      }
      for( x=0; x<sizeof( buf ); x++ ) {
        i++;
        ch = ( char * )( macro + i );
        switch( *ch ) {
        case ',':
        case ')':
          scratch = atoi( buf );
          //p("got led #%d\n", scratch);
          if( val1 == -1 ) {
            p( "got %s %d\n", END_INTS, scratch );
            val1 = scratch;
            x = -1;
          } else {
            if( freq == -1 ) {
              p( "got frequency %d\n", scratch );
              freq = scratch;
              x = -1;
            } else {
              if( repeat == -1 ) {
                p( "got repeat %d\n", scratch );
                repeat = scratch;
                blink_leds( leds, val1, freq, repeat );
                x = sizeof( buf );
              } else {
                p( "Blink set %s", PARAM_ERR );
              }
            }
          }
          memset( buf, 0, sizeof( buf ) );
          break;
        default:
          buf[x] = *ch;
          //p("buf[%d] = %c\n", x, *ch);
          break;
        }
      }
      //leds = 0;
      val1 = -1;
      val2 = -1;
      freq = -1;
      wait = - 1;
      repeat = -1;
      mode = 0;
      break;
    case 'f':
    case 'F':
    case 'p':
    case 'P':
      i++;
      if( mode == 0 ) {
        if( ( *ch == 'f' ) || ( *ch == 'F' ) ) {
          mode = MODE_FADE;
          p( "\nFade\n" );
        } else {
          mode = MODE_PULSE;
          p( "\nPulse\n" );
        }
      } else {
        p( "%s",MODE_SEL_ERR );
      }
      for( x=0; x<sizeof( buf ); x++ ) {
        i++;
        ch = ( char * )( macro + i );
        switch( *ch ) {
        case ',':
        case ')':
          scratch = atoi( buf );
          //p("got led #%d\n", scratch);
          if( val1 == -1 ) {
            p( "got %s %d\n",START_INTS, scratch );
            val1 = scratch;
            x = -1;
          } else {
            if ( val2 == -1 ) {
              p( "got %s %d\n",END_INTS, scratch );
              val2 = scratch;
              x = -1;
            } else {
              if( freq == -1 ) {
                p( "got frequency %d\n", scratch );
                freq = scratch;
                x = -1;
              } else {
                if( repeat == -1 ) {
                  p( "got repeat %d\n", scratch );
                  repeat = scratch;
                  if( mode == MODE_FADE ) {
                    fade_leds( leds, val1, val2, freq, repeat );
                  } else {
                    pulse_leds( leds, val1, val2, freq, repeat );
                  }
                  x = sizeof( buf );
                } else {
                  p( "set %s", PARAM_ERR );
                }
              }
            }
          }
          memset( buf, 0, sizeof( buf ) );
          break;
        default:
          buf[x] = *ch;
          //p("buf[%d] = %c\n", x, *ch);
          break;
        }
      }
      //leds = 0;
      val1 = -1;
      val2 = -1;
      freq = -1;
      wait = - 1;
      repeat = -1;
      mode = 0;
      break;
    case 'i':
    case 'I':
      if( mode == 0 ) {
        mode = MODE_SET;
        p( "\nIntensity\n" );
      } else {
        p( "%s",MODE_SEL_ERR );
      }
      i++;
      mode = MODE_SET;
      memset( buf,0, sizeof( buf ) );
      for( x=0; x<sizeof( buf ); x++ ) {
        i++;
        ch = ( char * )( macro + i );
        switch( *ch ) {
        case ')':
          if( freq == -1 ) {
            scratch = atoi( buf );
            p( "got %s %d\n", END_INTS, scratch );
            val1 = scratch;
            set_leds( leds, val1 );
            x = sizeof( buf );
          } else {
            p( "intensity %s", PARAM_ERR );
          }
          break;
        default:
          buf[x] = *ch;
          break;
        }
      }
      //leds = 0;
      val1 = -1;
      val2 = -1;
      freq = -1;
      wait = - 1;
      repeat = -1;
      mode = 0;
      break;
    case 'w':
    case 'W':
      if( ( mode == 0 ) &&  ( *( ch+1 ) == '(' ) ) {
        i++;
        p( "\nWait\n" );
        mode = MODE_WAIT;
        memset( buf,0, sizeof( buf ) );
        for( x=0; x<sizeof( buf ); x++ ) {
          i++;
          ch = ( char * )( macro + i );
          switch( *ch ) {
          case ')':
            if( freq == -1 ) {
              scratch = atoi( buf );
              p( "got freq %d\n", scratch );
              freq = scratch;
              delay( freq );
              x = sizeof( buf );
            } else {
              p( "Wait set %s", PARAM_ERR );
            }
            break;
          default:
            buf[x] = *ch;
            break;
          }
        }
      } else {
        p( "%s",MODE_SEL_ERR );
      }
      //leds = 0;
      val1 = -1;
      val2 = -1;
      freq = -1;
      wait = - 1;
      repeat = -1;
      mode = 0;
      break;
    case 'c':
    case 'C':
      i++;
      p( "Clear\n" );
      set_leds( ALL_LEDS, 0 );
      //leds = 0;
      val1 = -1;
      val2 = -1;
      freq = -1;
      wait = - 1;
      repeat = -1;
      mode = 0;
      break;
    }
  }// end of loop through macro
  //p("led = %d\n", leds );
  return ok;
}

void unrecognized()
{
  p( "\nError!\n" );
}

