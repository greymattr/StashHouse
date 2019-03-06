#include "ssd1306Menu.hpp"

static int menu_select;
static int menu_display_offset = 0;
static int skip_advance = 0;
static unsigned int button_state = HIGH;
static unsigned int long_button_press_counter = 0;
//extern SSD1306Wire display;

int check_button( void )
{
  unsigned int i;
  i = digitalRead( BUTTON_PIN );   // read the input pin
  if ( i ==  LOW ) {
    if( button_state != LOW ) {
        button_state = LOW;
        Serial.printf("button pressed %d\n\r", menu_select );
    }
    if( long_button_press_counter < LONG_BUTTON_PRESS_TIMEOUT )long_button_press_counter++;
  } else {
    if( button_state == LOW ) {
      Serial.printf("end of button pressed %d\n\r", menu_select);
      if( long_button_press_counter == LONG_BUTTON_PRESS_TIMEOUT ) {
          long_button_press_counter = 0;
          skip_advance = 1;
          return menu_select;
      }
      if( long_button_press_counter != LONG_BUTTON_PRESS_TIMEOUT ) {
        if(skip_advance == 0 ){
            menu_select++;
            if( menu_select >= MAX_MENU_ENTRIES_ON_SCREEN ){
                menu_display_offset = (( menu_select - MAX_MENU_ENTRIES_ON_SCREEN ) + SCROLL_BY_LINES );
            }
        } else {
            skip_advance = 0;
        }
      }
      button_state = HIGH;
    }
    long_button_press_counter = 0;
  }
  if( long_button_press_counter >= LONG_BUTTON_PRESS_TIMEOUT ){
      Serial.println("long button press detected");
      long_button_press_counter = LONG_BUTTON_PRESS_TIMEOUT;
  }
  return -1;
}

int show_menu( struct Menu *m, SSD1306Wire  *d)
{
    int ok = 0;
    int i = 0;
    int x = 0, y = 0;
    char sel_buf[ MAX_MENU_SIZE + 4 ];
    char title[ 32 ];

    //Serial.printf("show_menu \n\r");
    d->clear();
    d->setFont(ArialMT_Plain_10);
    memset(title, 0, sizeof(title));
    if( m->title[0] != '\0' ){
        sprintf(title, "%s", m->title);
    }

    if( m->title[0] != '\0' ){
      d->drawRect(x, y, SCREEN_WIDTH, CHARACTER_HEIGHT + 2);
      d->setTextAlignment(TEXT_ALIGN_CENTER);
      d->drawString(SCREEN_WIDTH/2, 0, m->title);
      x = CHARACTER_HEIGHT + 2;
    }
    d->setColor(WHITE);
    d->setTextAlignment(TEXT_ALIGN_LEFT);
    for(i=menu_display_offset; i <= (menu_display_offset + MAX_MENU_ENTRIES_ON_SCREEN); i++ ){
        if( m->menu[i][0] == '\0' )break;
        if( menu_select >= (int)(m->length)){
            menu_select = 0;
            menu_display_offset = 0;
        }
        if( i == menu_select ){
           d->fillRect(y, x+1, SCREEN_WIDTH, CHARACTER_HEIGHT);
           d->setColor(BLACK);
           if ( long_button_press_counter >= LONG_BUTTON_PRESS_TIMEOUT ) {
              memset(sel_buf, 0, sizeof(sel_buf));
              if(strlen(m->menu[i]) < ( MAX_MENU_SIZE - SELECT_INDICATOR_LEN )){
                  sprintf(sel_buf, "%s %s",  m->menu[i], SELECT_INDICATOR);
              } else {
                  memcpy(sel_buf, m->menu[i], (strlen(m->menu[i]) - SELECT_INDICATOR_LEN));
                  sprintf(sel_buf+(strlen(m->menu[i])- SELECT_INDICATOR_LEN ), (char *)SELECT_INDICATOR);
              }
              d->drawString(y, x, sel_buf);
           } else {
               d->drawString(y, x, m->menu[i]);
           }
           d->setColor(WHITE);
       } else {
           d->drawString(y, x, m->menu[i]);
       }
       x+=CHARACTER_HEIGHT;
    }
    d->display();
    ok = check_button( );
    if( ok != -1){
      Serial.printf("user made selection #%d\n\r", ok);
      if(m->callback[ok] != NULL)m->callback[ok]();
      menu_select = 0;
      menu_display_offset = 0;
    }
    if(m->type == MENU_INFO){
      menu_select = 0;
      menu_display_offset = 0;
    }
    return ok;
}
