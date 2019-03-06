#ifndef __SSD1306MENU_H__
#define __SSD1306MENU_H__

#include <Wire.h>  // needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"

#define MAX_MENU_SIZE               22  // Max number of characters for menu item
#define MAX_MENU_ENTRIES_ON_SCREEN  5   // how many menu entries fit on a 128x64 screen at 10pt font
#define MAX_SCROLL_PAGES            3   // don't scroll down more than this many pages
#define MAX_MENU_ENTRIES            ( MAX_MENU_ENTRIES_ON_SCREEN*MAX_SCROLL_PAGES )   // Maximum number of entries in a menu

#define BUTTON_PIN                  D6
#define LONG_BUTTON_PRESS_TIMEOUT   10  // increase to make long hold select time longer
#define SELECT_INDICATOR            " <<<"
//#define SELECT_INDICATOR            " ***"
//#define SELECT_INDICATOR            " SELECT"
#define SELECT_INDICATOR_LEN        strlen(SELECT_INDICATOR)
#define NICE                        delay(100)

#define SCROLL_BY_LINES             2  // when scrolling, scroll by this number of lines

/* different menu types */
#define MENU_MAIN                   100
#define MENU_SUB                    150
#define MENU_INFO                   200
#define MENU_CFG                    250

/* screen and font information */
#define SCREEN_WIDTH                128
#define SCREEN_HEIGHT               64
#define CHARACTER_HEIGHT            10

/* Menu type definition */
typedef struct Menu {
    char title[MAX_MENU_SIZE-2];
    unsigned int length;
    char menu[MAX_MENU_ENTRIES][ MAX_MENU_SIZE ];
    unsigned int type;
    void (*callback[MAX_MENU_ENTRIES])( );
}Menu;


int check_button( void );
//int show_menu( struct Menu *m );
int show_menu( struct Menu *m, SSD1306Wire  *d );

#endif
