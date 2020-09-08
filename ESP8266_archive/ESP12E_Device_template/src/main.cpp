/*
 * Basic Template project for ESP12E NodeMCU1.0 boards
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Matthew Fatheree greymattr(_at_)gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 /*  Include all headers needed for full device functionality */
 #include <Arduino.h>
 #include <ESP8266WiFi.h>          // https://github.com/esp8266/Arduino
 #include <DNSServer.h>            // Include for network functionality
 #include <ESP8266WebServer.h>     // Include for web server functionality
 #include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager
 #include <WiFiClient.h>           // for connection to AP
 #include <Ticker.h>               // for LED status indicator
 #include <NTPClient.h>
 #include <WiFiUdp.h>
 #include <ArduinoOTA.h>
 #include <ESP8266mDNS.h>          // Include the mDNS library
 #include <EEPROM.h>               // for storing configuration
 #include <PubSubClient.h>
 #include <FS.h>                   // Include the SPIFFS library
 #include <Wire.h>
 #include "SSD1306Wire.h"
 #include <Adafruit_Sensor.h>
 #include <DHT.h>
 #include <DHT_U.h>
 #include <HCSR04.h>
 #include "OneButton.h"


/* ESP12E / NodeMCU default Pin MAP     */
/* PIN No.  Input Ok    Output Ok
  n D0
  a D1         X           X
  a D2         X           X  pwm
  o D3                     X  ( pulled up )
  o D4                     X  Built in LED( pulled up )
  a D5         X           X  pwm
  a D6         X           X  pwm
  a D7         X           X
  o D8                     X  pwm ( pulled low, boot fails if high at boot )
    TX
    RX
*/

/* Build time configuration defines. comment out to disable functionality */
/* Built in board functionality */
#define DEVICE_HAS_EEPROM
#define DEVICE_HAS_HTTP
#define DEVICE_HAS_MDNS
#define DEVICE_HAS_MQTT
#define DEVICE_HAS_NTP
#define DEVICE_HAS_OTA
#define DEVICE_HAS_SPIFFS
#define DEVICE_HAS_MIN_TICKER
#define DEVICE_HAS_SEC_TICKER
/* Add on sensor and IO support section */
//#define DEVICE_HAS_SPEAKER
//#define DEVICE_HAS_SD1306_SCREEN
//#define DEVICE_HAS_DHT_SENSOR
//#define DEVICE_HAS_HC_SR04
//#define DEVICE_HAS_BUTTON

/* HTTP define configurations */
#ifdef DEVICE_HAS_HTTP
#define HTTP_SERVER_PORT  80
ESP8266WebServer server( HTTP_SERVER_PORT );              // web server
#endif

/* SD1306 Screen define configurations */
#ifdef DEVICE_HAS_SD1306_SCREEN
#define SCREEN_WIDTH        128
#define SCREEN_HEIGH        64
#define SCL_PIN             D4
#define SDA_PIN             D3
SSD1306Wire  display(0x3c, SDA_PIN, SCL_PIN); // SSD1306 Display init
#endif

/* Speaker define configurations             */
#ifdef DEVICE_HAS_SPEAKER
#define SPEAKER_PIN         D8
#endif

/* DHT temp & humidity sensor define configurations */
#ifdef DEVICE_HAS_DHT_SENSOR
#define DHTPIN              D3    // Digital pin connected to the DHT sensor
#define DHTTYPE             DHT11     // DHT 11
//#define DHTTYPE             DHT22     // DHT 22 (AM2302)
//#define DHTTYPE             DHT21     // DHT 21 (AM2301)
DHT_Unified dht(DHTPIN, DHTTYPE);
  #ifdef DEVICE_HAS_SD1306_SCREEN
    #define TEMP_DISPLAY_X_OFFSET     75
    #define TEMP_DISPLAY_Y_OFFSET     54
    #define HUMIDITY_DISPLAY_X_OFFSET     97
    #define HUMIDITY_DISPLAY_Y_OFFSET     54
  #endif
#endif

/* EEPROM storage define configurations */
#ifdef DEVICE_HAS_EEPROM
#define EEPROM_DATA_SIZE    512             // EEPROM data partition size
#endif

/* NTP define configurations */
#ifdef DEVICE_HAS_NTP
//#define NTP_SERVER          "time.nist.gov" // time server
#define NTP_SERVER          "pool.ntp.org" // time server
WiFiUDP ntpUDP;                             // NTP UDP object
NTPClient timeClient(ntpUDP, NTP_SERVER, (-7 * 3600), 3600);  // NTP Client
#endif

#ifdef DEVICE_HAS_HC_SR04
#define HCSR04_ECHO_PIN     D6
#define HCSR04_TRIG_PIN     D7
UltraSonicDistanceSensor distanceSensor(HCSR04_ECHO_PIN, HCSR04_TRIG_PIN);
#endif

/* MQTT define configurations */
#ifdef DEVICE_HAS_MQTT
#define MQTT_SERVER         "greysic.com"   // MQTT server
const char* mqtt_server = MQTT_SERVER;
WiFiClient espClient;
PubSubClient client(espClient);
#endif

#ifdef DEVICE_HAS_BUTTON
#define BUTTON_PIN          D5
//OneButton button_1(BUTTON_PIN, true);     // active low
OneButton button_1(BUTTON_PIN, false, false);    // active high ( need 4.7K resistor from pin to GND )
#endif

/* some useful defines for all projects */
#define STATUS_LED          LED_BUILTIN     // ESP12E on board LED ( active HIGH )
#define DEV_BOARD_LED       D0              // NodeMCU dev on board LED ( active LOW )
#define BLINKER_FAST        0.2
#define BLINKER_SLOW        2
#define BLINKER_NORMAL      0.5
#define MAC_ADDR_LEN        6

/* function prototypes for IOT built-in IOT functionality */
void run_server_tasks( void );
void configModeCallback ( WiFiManager *myWiFiManager );
void blinker_task( void );
void once_per_sec_task( void );
void once_per_min_task( void );
void send_self_redirect( char *msg, int delay );
void handle_root( void );
void handle_404( void );
void handle_config( void );
int init_eeprom( void );
int eeprom_read_buf( char *buf, unsigned int offset, unsigned int size );
int eeprom_write_buf( char *buf, unsigned int offset, unsigned int size );
int init_spiffs( void );
int init_display( void );
void init_dht_sensor( void );
void init_OTA_upgrade( void );
void reboot_esp( void );
void reset_wifi_config( void );
void reconnect();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void shutdown_services( void );
void get_last_reset_reason( void );
void print_esp8266_data( void );
void display_temp_humidity( void );
void display_clock ( void );
int hcsr04_get_distance_cm( void );
void mqtt_status_message(char *msg);
void pressed();
void released();
void changed();
void click();
void longClick();
void doubleClick();
void tripleClick();
void tap();

/* Global variables for IOT functionality  */
static unsigned char dev_mac[ MAC_ADDR_LEN ];      // MAC Address of this device
static char dev_name[ 64 ];                        // device name
Ticker blink_ticker;                        // defualt LED blinker functionality
Ticker once_per_sec;                        // second ticker task ( DONT BLOCK );
Ticker once_per_min;                        // minute ticker task ( DONT BLOCK );
static unsigned int DeviceConnected = 0;
static unsigned int update_sec_flag = 1;
static unsigned int update_min_flag = 1;
/* wifi Manager variables/objects */
static WiFiManager wifiManager;             // wifiManager for wifi configuration
void configModeCallback ( WiFiManager *myWiFiManager );


int motor_ina = D6;
int motor_inb = D7;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting...");
  init_display();
  delay(500);
  get_last_reset_reason();
  print_esp8266_data();
  pinMode( STATUS_LED, OUTPUT );                   // set pin for STATUS
  pinMode( DEV_BOARD_LED, OUTPUT );                // set pin for board STATUS
  WiFi.macAddress( ( byte * )dev_mac );         // fill in the devices mac address
  blink_ticker.attach(BLINKER_NORMAL, blinker_task);       // start the default blinker task

#ifdef DEVICE_HAS_SEC_TICKER
  once_per_sec.attach(1, once_per_sec_task);
#endif
#ifdef DEVICE_HAS_MIN_TICKER
  once_per_min.attach(60, once_per_min_task);
#endif

#ifdef DEVICE_HAS_BUTTON
  button_1.attachClick(click);
  button_1.attachDoubleClick(doubleClick);
#endif

  init_OTA_upgrade();                 // start the over the air upgrade task
  init_eeprom();                      // start the eeprom NV storage
  if( init_spiffs() == 1 ) {          // start the SPIFFS file system
    Serial.printf("spiffs file system formatted\n\r");
  }
  init_dht_sensor();
}
/*     END OF setup()      */

void loop() {
  // put your main code here, to run repeatedly:
#ifdef DEVICE_HAS_SD1306_SCREEN
  delay(250);
  display.clear();
  display.display();
#endif

  memset( dev_name, 0, sizeof( dev_name ) );
  sprintf( dev_name, "ESP_%02X%02X%02X", dev_mac[3], dev_mac[4], dev_mac[5] );
  Serial.printf( "%s - START, attempting to connect to wifi\n\r", dev_name );
  wifiManager.setAPCallback( configModeCallback );
  if ( !wifiManager.autoConnect( dev_name ) ) {
    Serial.printf( "Failed to connect and hit timeout\n\r" );
  }
  delay(100);
  /* past this line the unit is connected to wifi */
  DeviceConnected = 1;
  WiFi.hostname( dev_name );
  blink_ticker.detach();
  digitalWrite(STATUS_LED, LOW);
  delay(100);

#ifdef DEVICE_HAS_HTTP
  // start the web server
  server.on( "/", handle_root );
  server.on("/config", handle_config );
  server.onNotFound( handle_404 );
  server.begin();
#endif

  Serial.printf( "SSID: %s\n\r", WiFi.SSID().c_str() );
  Serial.printf( "  IP: %s\n\r", WiFi.localIP().toString().c_str() );
  Serial.printf( "NAME: %s\n\r", WiFi.hostname().c_str() );

#ifdef DEVICE_HAS_SD1306_SCREEN
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(5, 0, WiFi.hostname().c_str());
  display.drawString(5, 18, WiFi.localIP().toString().c_str());
  display.display();
#endif

#ifdef DEVICE_HAS_MDNS
  if (!MDNS.begin(dev_name)) {             // Start the mDNS responder for esp8266.local
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.println("mDNS responder started");
  }
#endif

#ifdef DEVICE_HAS_MQTT
  Serial.printf("connecting to mqtt server @ %s\n\r", mqtt_server);
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqtt_callback);
#endif

#ifdef DEVICE_HAS_NTP
  timeClient.begin();
  delay(100);
  timeClient.update();
  Serial.printf("Current time: %s\n\r", timeClient.getFormattedTime().c_str());
#endif

  // loop forever or until reboot
  while( true ){
    // run run_server_tasks is a function that should contain all custom functionality
    run_server_tasks();
  }
  shutdown_services();
}
/*      END OF loop()          */

/* run_server_tasks is meant to run in the main loop, as a sort of very basic task manager */
void run_server_tasks( void )
{
  if( DeviceConnected == 1 ){
#ifdef DEVICE_HAS_OTA
    ArduinoOTA.handle();          // handle OTA upgrade traffic
#endif
#ifdef DEVICE_HAS_HTTP
    server.handleClient();        // handle web server traffic
#endif
#ifdef DEVICE_HAS_MQTT
    if (!client.connected()) {    // handle MQTT connections
      reconnect();
    }
    client.loop();
#endif
#ifdef DEVICE_HAS_BUTTON
    button_1.tick();
#endif
  }
  // offline server tasks below
  if( update_sec_flag == 1 ) {
    display_clock();
    update_sec_flag = 0;
  }
  if( update_min_flag == 1 ) {
    display_temp_humidity();
    update_min_flag = 0;
  }
  //yield();
  return ;
}

/* configModeCallback - gets called when WiFiManager enters configuration mode */
void configModeCallback ( WiFiManager *myWiFiManager )
{
  static int offline_ticker = 0;
  Serial.println( "Entered config mode" );
  if( offline_ticker == 0 ) {
    blink_ticker.detach();
    blink_ticker.attach(BLINKER_SLOW, blinker_task);
    offline_ticker = 1;
  }
  Serial.println( WiFi.softAPIP() );
  // print the SSID of the config portal
  Serial.println( myWiFiManager->getConfigPortalSSID() );

#ifdef DEVICE_HAS_SD1306_SCREEN
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setColor(WHITE);
  display.drawString(0, 0, "DISCONNECTED");

  display.drawString(5, 18, myWiFiManager->getConfigPortalSSID());
  display.setFont(ArialMT_Plain_10);
  display.drawString(5, 54, WiFi.softAPIP().toString());
#endif
}

void blinker_task( void )
{
  static int led_state = LOW;
  if( led_state == HIGH ){
    led_state = LOW;
  } else {
    led_state = HIGH;
  }
  //digitalWrite(STATUS_LED, led_state);
  digitalWrite( DEV_BOARD_LED, led_state );
}

void once_per_sec_task( void )
{
  /* This task will run once per second, and should not block!
     If read data is needed a flag should be set, and read in the loop, or
     run_server_tasks functions.
  */
  static int second_counter = 0;
  //Serial.printf("sec task %d\n\r", second_counter);
#ifdef DEVICE_HAS_NTP
  //Serial.printf("sec task %s\n\r", timeClient.getFormattedTime().c_str());
#endif
  update_sec_flag = 1;
  second_counter++;

#if 0
  if( second_counter < 50 ){
    Serial.printf("motor speed %d\n\r", second_counter * 10);
    analogWrite(motor_ina, ( second_counter * 10 ));
    digitalWrite(motor_inb, LOW);
  } else {
    analogWrite(motor_ina, 160);
    digitalWrite(motor_inb, LOW);
  }
#endif
}

void once_per_min_task ( void )
{
  /* This task will run once per minute, and should not block!
     If read data is needed a flag should be set, and read in the loop, or
     run_server_tasks functions.
  */
  static int minute_counter = 0;
  Serial.printf("min task %d\n\r", minute_counter);
#ifdef DEVICE_HAS_NTP
  //Serial.printf("min task %s\n\r", timeClient.getFormattedTime().c_str());
#endif
  update_min_flag = 1;
  minute_counter++;
}

void send_self_redirect( char *msg, int delay )
{
#ifdef DEVICE_HAS_HTTP
  char buf[ 256 ];
  memset( buf, 0, sizeof( buf ) );
  sprintf( buf, "<HTML><HEAD><meta http-equiv=\"refresh\" content=\"%i; URL='/'\"/></HEAD><BODY>%s</BODY></HTML>", delay, msg );
  server.send( 200, "text/html", buf);
#endif
}

void handle_root( void )
{
#ifdef DEVICE_HAS_HTTP
  char buf[ 512 ];
  char mbuf[ 32 ];
  memset( buf, 0, sizeof( buf ) );
  sprintf( buf, "<HTML><HEAD></HEAD><BODY><pre>" );
  sprintf( buf+strlen( buf ), "<h3>%s</h3>", WiFi.hostname().c_str() );
  sprintf( buf+strlen( buf ), "<table cellpadding=3 cellspacing=0 border=0>" );
  sprintf( buf+strlen( buf ), "<tr><td>Hostname</td><td>%s</td></tr>", WiFi.hostname().c_str() );
  sprintf( buf+strlen( buf ), "<tr><td>IP</td><td>%s</td></tr>", WiFi.localIP().toString().c_str() );
  sprintf( buf+strlen( buf ), "<tr><td>MAC</td><td>");
  memset( mbuf, 0, sizeof(mbuf));
  sprintf( mbuf, "%02X:%02X:%02X:%02X:%02X:%02X", dev_mac[0],dev_mac[1],dev_mac[2],dev_mac[3],dev_mac[4],dev_mac[5]);
  sprintf( buf+strlen( buf ), "%s</td></tr>", mbuf);
  sprintf( buf+strlen( buf ), "<tr><td>ssid</td><td>%s</td></tr>",  WiFi.SSID().c_str() );
  sprintf( buf+strlen( buf ), "</table>");
  sprintf( buf+strlen( buf ), "%s", timeClient.getFormattedTime().c_str());
  sprintf( buf+strlen( buf ), "</pre></BODY></HTML>\n\r" );
  server.send( 200, "text/html", buf );
#endif
}

void handle_404( void )
{
#ifdef DEVICE_HAS_HTTP
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET )?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for ( uint8_t i=0; i<server.args(); i++ ) {
    message += " " + server.argName( i ) + ": " + server.arg( i ) + "\n";
  }
  server.send( 404, "text/plain", message );
#endif
}

void handle_config( void )
{
#ifdef DEVICE_HAS_HTTP
  char buf[1024];
  int i;

  memset(buf, 0, sizeof(buf));
  sprintf( buf, "<HTML><HEAD></HEAD><BODY>" );
  for ( i = 0; i < server.args(); i++ ) {
    if ( strncmp(server.argName(i).c_str(), "wificfg", 7) == 0 ) {
      if( strncmp(server.arg(i).c_str(), "reset", 5) == 0 ) {
        Serial.printf("reset wifi configuration\n\r");
        send_self_redirect((char *)"resetting wifi in 3 seconds...\n\r", 30);
        delay(2000);
        wifiManager.resetSettings();
        delay(1000);
        ESP.restart();
        return;
      }
    }
  }
  sprintf( buf+strlen( buf ),"<button onclick=\"document.location='config?wificfg=reset'\">WiFi Config Reset</button><br>\n\r");
  sprintf( buf+strlen( buf ),"</BODY></HTML>\n\r");
  server.send( 200, "text/html", buf );
#endif
}

int init_eeprom( void )
{
#ifdef DEVICE_HAS_EEPROM
  EEPROM.begin(EEPROM_DATA_SIZE);
#endif
  return 0;
}

int eeprom_read_buf( char *buf, unsigned int offset, unsigned int size )
{
  unsigned int i = 0;
#ifdef DEVICE_HAS_EEPROM
  for(i=0;i<size;i++){
    buf[i] = EEPROM.read(offset + i);
  }
#endif
  return i;
}

int eeprom_write_buf( char *buf, unsigned int offset, unsigned int size )
{
  unsigned int i = 0;
#ifdef DEVICE_HAS_EEPROM
  for(i=0;i<size;i++){
    EEPROM.write(offset + i, buf[i]);
  }
#endif
  return i;
}

int init_spiffs( void )
{
  int fs_is_new = 0;
#ifdef DEVICE_HAS_SPIFFS
  if(!SPIFFS.begin()){
    Serial.printf("formatting file system\n\r");
    SPIFFS.format();
    fs_is_new = 1;
  }
#endif
  return fs_is_new;
}

int init_display( void )
{
#ifdef DEVICE_HAS_SD1306_SCREEN
  delay(100);
  display.init();
  delay(100);
  display.flipScreenVertically();
  display.clear();
  display.display();
  display.setFont(ArialMT_Plain_16);
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  //display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(10, 20, "screen init");
  display.display();
#endif
  return 0;
}

void init_dht_sensor( void )
{
#ifdef DEVICE_HAS_DHT_SENSOR
  dht.begin();
#endif
}

void init_OTA_upgrade( void )
{
#ifdef DEVICE_HAS_OTA
  ArduinoOTA.onStart([]() {
      Serial.println("Upgrade Start");
      blink_ticker.detach();
      blink_ticker.attach(BLINKER_FAST, blinker_task);
#ifdef DEVICE_HAS_SD1306_SCREEN
      display.setFont(ArialMT_Plain_16);
      display.setColor(WHITE);
      display.clear();
      display.drawString(5, 32, "Upgrade: 0%");
      display.display();
#endif
  });
  ArduinoOTA.onEnd([]() {
      Serial.println("\nUpgrade End...Restarting");
      blink_ticker.detach();
#ifdef DEVICE_HAS_SD1306_SCREEN
      display.end();
#endif
      delay(1000);
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Upgrade Progress: %u%%\r", (progress / (total / 100)));
#ifdef DEVICE_HAS_SD1306_SCREEN
      display.clear();
      display.drawString(5, 32, "Upgrade: " + String((int)round((progress / (total / 100)))) + "%");
      display.display();
#endif
      if( ( progress / (total / 100)) == 100 ){
          Serial.printf("[ Upgrade Complete ]");
#ifdef DEVICE_HAS_SD1306_SCREEN
          display.clear();
          display.drawString(5, 5, "Complete 100%");
          display.display();
#endif
          delay(1000);
      }
  });
  ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Upgrade Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
      delay(2000);
  });
  delay(1000);
  Serial.printf( "WiFi upgrade enabled\n\r");
  ArduinoOTA.begin();
#endif
}

void reboot_esp( void )
{
  shutdown_services();
  ESP.restart();
}

void reset_wifi_config( void )
{
  Serial.printf("reset wifi configuration\n\r");
  wifiManager.resetSettings();
  delay(1000);
  ESP.restart();
}

void reconnect()
{
#ifdef DEVICE_HAS_MQTT
  char cmd_topic[255];
  char stat_topic[255];

  memset(cmd_topic, 0, sizeof(cmd_topic));
  memset(stat_topic, 0, sizeof(stat_topic));

  sprintf(cmd_topic, "%s/cmd", dev_name);
  sprintf(stat_topic, "%s/status", dev_name);
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(dev_name)) {
      Serial.printf("connected\n\r");
      // by default subscribe to our own device name topic, device_name/cmd, device_name/status
      Serial.printf("subscribing to topic: %s\n\r", dev_name);
      client.subscribe(dev_name);
      Serial.printf("subscribing to topic: %s\n\r", cmd_topic);
      client.subscribe(cmd_topic);
      Serial.printf("subscribing to topic: %s\n\r", stat_topic);
      client.subscribe(stat_topic);
      mqtt_status_message((char *)"Online");
    } else {
      Serial.printf("failed, rc= %d\n\r", client.state());
      Serial.printf(" try again in 5 seconds...\n\r");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
#endif
}

void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
#ifdef DEVICE_HAS_MQTT
  unsigned int i;
  Serial.printf("MQTT MSG\n\r");
  Serial.printf("Topic: %s \n\r", topic);
  Serial.printf("  Msg: ");
  for (i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
#endif
  return ;
}

void shutdown_services( void )
{
  Serial.printf("running shutdown\n\r");
#ifdef DEVICE_HAS_MIN_TICKER
  once_per_min.detach();
#endif
#ifdef DEVICE_HAS_SEC_TICKER
  once_per_sec.detach();
#endif
#ifdef DEVICE_HAS_EEPROM
  EEPROM.end();
#endif
#ifdef DEVICE_HAS_HTTP
  server.stop();
#endif
}

void get_last_reset_reason( void )
{
  Serial.printf("Reset reason %s\n\r", ESP.getResetReason().c_str());
}

void print_esp8266_data( void )
{
  Serial.printf("     ESP Data\n\r");
  Serial.printf("   Chip ID: %u\n\r", ESP.getChipId());
  Serial.printf(" Free Heap: %u\n\r", ESP.getFreeHeap());
  Serial.printf("  Core Ver: %s\n\r",  ESP.getCoreVersion().c_str());
  Serial.printf("   SDK Ver: %u\n\r", (int)ESP.getSdkVersion());
  Serial.printf("  CPU Freq: %u\n\r", ESP.getCpuFreqMHz());
  Serial.printf("  Prog Use: %u used, %u free\n\r", ESP.getSketchSize(), ESP.getFreeSketchSpace());
  Serial.printf("  Flash ID: %u\n\r", ESP.getFlashChipId());
  Serial.printf(" Board LED: %d\n\r", STATUS_LED);
  Serial.printf("   Pin Map:\n\r");
  Serial.printf("    %d = D0\n\r", D0);
  Serial.printf("    %d = D1\n\r", D1);
  Serial.printf("    %d = D2\n\r", D2);
  Serial.printf("    %d = D3\n\r", D3);
  Serial.printf("    %d = D4\n\r", D4);
  Serial.printf("    %d = D5\n\r", D5);
  Serial.printf("    %d = D6\n\r", D6);
  Serial.printf("    %d = D7\n\r", D7);
  Serial.printf("    %d = D8\n\r", D8);
  Serial.printf("    %d = A0\n\r", A0);
  Serial.printf("\n\r");
}

void display_temp_humidity( void )
{
#ifdef DEVICE_HAS_DHT_SENSOR
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Serial.print(F("Temperature: "));
    //Serial.print(event.temperature);
    //Serial.println(F("°C"));
    Serial.print((int)round((1.8*event.temperature)+32));
    Serial.println(F("°F"));
#ifdef DEVICE_HAS_SD1306_SCREEN
    display.setFont(ArialMT_Plain_10);
    display.setColor(BLACK);
    display.fillRect(TEMP_DISPLAY_X_OFFSET, TEMP_DISPLAY_Y_OFFSET, 20, 10);
    display.setColor(WHITE);
    display.drawString(TEMP_DISPLAY_X_OFFSET, TEMP_DISPLAY_Y_OFFSET, String((int)round((1.8*event.temperature)+32)) + "°F");
#endif
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
#ifdef DEVICE_HAS_SD1306_SCREEN
    display.setColor(BLACK);
    display.fillRect(HUMIDITY_DISPLAY_X_OFFSET, HUMIDITY_DISPLAY_Y_OFFSET, 20, 10);
    display.setColor(WHITE);
    display.drawString(HUMIDITY_DISPLAY_X_OFFSET, HUMIDITY_DISPLAY_Y_OFFSET, "/ " + String((int)round(event.relative_humidity)) + "%");
#endif
  }
#ifdef DEVICE_HAS_SD1306_SCREEN
  display.display();
#endif
#endif
}

void display_clock ( void )
{
#ifdef DEVICE_HAS_SD1306_SCREEN
#ifdef DEVICE_HAS_NTP
  display.setFont(ArialMT_Plain_16);
  display.setColor(BLACK);
  display.fillRect(4, 36, 64, 18);
  display.setColor(WHITE);
  display.drawString(5, 36, timeClient.getFormattedTime());
  display.display();
  update_sec_flag = 0;
#endif
#endif
}

int hcsr04_get_distance_cm( void )
{
  int ok = -1;
#ifdef DEVICE_HAS_HC_SR04
  Serial.print("Distance: ");
  ok = (int)distanceSensor.measureDistanceCm();
  //Serial.println(distanceSensor.measureDistanceCm());
#endif
  return ok;
}

void mqtt_status_message(char *msg)
{
#ifdef DEVICE_HAS_MQTT
  char stat_topic[255];
  memset(stat_topic, 0, sizeof(stat_topic));
  sprintf(stat_topic, "%s/status", dev_name);
  if(client.connected()) {
    client.publish(stat_topic, msg);
  }
#endif
return;
}

void pressed() {
    Serial.println("pressed");
}
void released() {
    Serial.print("released: ");
}
void changed() {
    Serial.println("changed");
}
void click() {
    Serial.println("click\n");
#ifdef DEVICE_HAS_SPEAKER
    tone(SPEAKER_PIN, 1400, 500);
#endif
}
void longClick() {
    Serial.println("long click\n");
}
void doubleClick() {
    Serial.println("double click\n");
}
void tripleClick() {
    Serial.println("triple click\n");
}
void tap() {
    Serial.println("tap");
}
