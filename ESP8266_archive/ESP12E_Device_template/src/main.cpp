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

/* Build time configuration defines. comment out to disable functionality */
/* THESE DEFINES ARE NOT CURRENTLY USED                                   */
#define DEVICE_HAS_EEPROM
#define DEVICE_HAS_HTTP
#define DEVICE_HAS_MDNS
#define DEVICE_HAS_MQTT
#define DEVICE_HAS_NTP
#define DEVICE_HAS_OTA
#define DEVICE_HAS_SPIFFS
#define DEVICE_HAS_MIN_TICKER
#define DEVICE_HAS_SEC_TICKER

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

/* some useful defines for all projects */
#define STATUS_LED          LED_BUILTIN     // on board LED control
#define BLINKER_FAST        0.2
#define BLINKER_SLOW        2
#define BLINKER_NORMAL      0.5

#define EEPROM_DATA_SIZE    512             // EEPROM data partition size

#define MAC_ADDR_LEN        6

#define NTP_SERVER          "time.nist.gov" // time server

#define MQTT_SERVER         "greysic.com"   // MQTT server

/* function prototypes for IOT built-in IOT functionality */
void run_server_tasks( void );
void configModeCallback ( WiFiManager *myWiFiManager );
void blinker_task( void );
void once_per_sec_task( void );
void once_per_min_task( void );
void handle_root( void );
void handle_404( void );
int init_eeprom( void );
int eeprom_read_buf( char *buf, unsigned int offset, unsigned int size );
int eeprom_write_buf( char *buf, unsigned int offset, unsigned int size );
int init_spiffs( void );
void init_OTA_upgrade( void );
void reboot_esp( void );
void reset_wifi_config( void );
void reconnect();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void shutdown_services( void );

/* Global variables for IOT functionality  */
static unsigned char dev_mac[ MAC_ADDR_LEN ];            // MAC Address of this device
static char dev_name[ 64 ];                        // device name
Ticker blink_ticker;                        // defualt LED blinker functionality
Ticker once_per_sec;                        // second ticker task ( DONT BLOCK );
Ticker once_per_min;                        // minute ticker task ( DONT BLOCK );
/* wifi Manager variables/objects */
static WiFiManager wifiManager;             // wifiManager for wifi configuration
void configModeCallback ( WiFiManager *myWiFiManager );
/* NTP variables/objects */
WiFiUDP ntpUDP;                             // NTP UDP object
NTPClient timeClient(ntpUDP, NTP_SERVER, (-7 * 3600), 3600);  // NTP Client
/* web server variables/objects */
ESP8266WebServer server( 80 );              // web server
/* MQTT variables/objects */
const char* mqtt_server = MQTT_SERVER;
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting...");
  pinMode( STATUS_LED, OUTPUT );                // set pin for led STATUS
  WiFi.macAddress( ( byte * )dev_mac );         // fill in the devices mac address
  blink_ticker.attach(BLINKER_NORMAL, blinker_task);       // start the default blinker task
  once_per_sec.attach(1, once_per_sec_task);
  once_per_min.attach(60, once_per_min_task);
  init_OTA_upgrade();                 // start the over the air upgrade task
  init_eeprom();                      // start the eeprom NV storage
  if( init_spiffs() == 1 ) {          // start the SPIFFS file system
    Serial.printf("spiffs file system formatted\n\r");
  }
}
/*     END OF setup()      */

void loop() {
  // put your main code here, to run repeatedly:
  memset( dev_name, 0, sizeof( dev_name ) );
  sprintf( dev_name, "ESP_%02X%02X%02X", dev_mac[3], dev_mac[4], dev_mac[5] );
  Serial.printf( "%s - START, attempting to connect to wifi\n\r", dev_name );
  wifiManager.setAPCallback( configModeCallback );
  if ( !wifiManager.autoConnect( dev_name ) ) {
    Serial.printf( "Failed to connect and hit timeout\n\r" );
  }
  delay(300);
  /* past this line the unit is connected to wifi */
  WiFi.hostname( dev_name );

  blink_ticker.detach();
  digitalWrite(STATUS_LED, LOW);
  delay(100);
  timeClient.begin();
  delay(100);
  timeClient.update();

  // start the web server
  server.on( "/", handle_root );
  server.onNotFound( handle_404 );

  server.begin();
  Serial.printf( "SSID: %s\n\r", WiFi.SSID().c_str() );
  Serial.printf( "  IP: %s\n\r", WiFi.localIP().toString().c_str() );
  Serial.printf( "NAME: %s\n\r", WiFi.hostname().c_str() );

  if (!MDNS.begin(dev_name)) {             // Start the mDNS responder for esp8266.local
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.println("mDNS responder started");
  }

  Serial.printf("connecting to mqtt server @ %s\n\r", mqtt_server);
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqtt_callback);

  Serial.printf("Current time: %s\n\r", timeClient.getFormattedTime().c_str());

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
  ArduinoOTA.handle();          // handle OTA upgrade traffic
  server.handleClient();        // handle web server traffic
  if (!client.connected()) {    // handle MQTT connections
    reconnect();
  }
  client.loop();
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
}

void blinker_task( void )
{
  static int led_state = LOW;
  if( led_state == HIGH ){
    led_state = LOW;
  } else {
    led_state = HIGH;
  }
  digitalWrite(STATUS_LED, led_state);
}

void once_per_sec_task( void )
{
  static int second_counter = 0;
  //Serial.printf("sec task %s\n\r", timeClient.getFormattedTime().c_str());
  second_counter++;
}

void once_per_min_task ( void )
{
  static int minute_counter = 0;
  Serial.printf("min task %s\n\r", timeClient.getFormattedTime().c_str());
  minute_counter++;
  if(( minute_counter % 60 ) == 0 ){
    //timeClient.update();
    Serial.printf("[ %3d ] [ %s ] - once_per_minute_task\n\r", minute_counter, timeClient.getFormattedTime().c_str());
  }
}

void handle_root( void )
{
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
  return;
}

void handle_404( void )
{
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
  return;
}

int init_eeprom( void )
{
  EEPROM.begin(EEPROM_DATA_SIZE);
  return 0;
}

int eeprom_read_buf( char *buf, unsigned int offset, unsigned int size )
{
  unsigned int i;
  for(i=0;i<size;i++){
    buf[i] = EEPROM.read(offset + i);
  }
  return i;
}

int eeprom_write_buf( char *buf, unsigned int offset, unsigned int size )
{
  unsigned int i;
  for(i=0;i<size;i++){
    EEPROM.write(offset + i, buf[i]);
  }
  return i;
}

int init_spiffs( void )
{
  int fs_is_new = 0;
  if(!SPIFFS.begin()){
    Serial.printf("formatting file system\n\r");
    SPIFFS.format();
    fs_is_new = 1;
  }
  return fs_is_new;
}

void init_OTA_upgrade( void )
{
  ArduinoOTA.onStart([]() {
      Serial.println("Upgrade Start");
      blink_ticker.detach();
      blink_ticker.attach(BLINKER_FAST, blinker_task);
  });
  ArduinoOTA.onEnd([]() {
      Serial.println("\nUpgrade End...Restarting");
      blink_ticker.detach();
      delay(1000);
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Upgrade Progress: %u%%\r", (progress / (total / 100)));
      if( ( progress / (total / 100)) == 100 ){
          Serial.printf("[ Upgrade Complete ]");
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
  String clientId = dev_name;  // clientID = dev_name
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.printf("connected\n\r");
      // by default subscribe to our own device name topic
      Serial.printf("subscribing to topic: %s\n\r", dev_name);
      client.subscribe(dev_name);
    } else {
      Serial.printf("failed, rc= %d\n\r", client.state());
      Serial.printf(" try again in 5 seconds...\n\r");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
  unsigned int i;
  Serial.printf("MQTT MSG\n\r");
  Serial.printf("Topic: %s \n\r", topic);
  Serial.printf("  Msg: ");
  for (i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  return ;
}

void shutdown_services( void )
{
  Serial.printf("running shutdown\n\r");
  once_per_min.detach();
  once_per_sec.detach();
  EEPROM.end();
  server.stop();
}
