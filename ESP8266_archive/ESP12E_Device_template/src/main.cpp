/*
 * Basic Template project for ESP12E NodeMCU1.0 boards
 *
 */
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

#define STATUS_LED          LED_BUILTIN  // use the built in LED for status
#define EEPROM_DATA_SIZE    512

#define NTP_SERVER          "time.nist.gov"
#define MQTT_SERVER         "greysic.com"


void run_server_tasks( void );
void configModeCallback ( WiFiManager *myWiFiManager );
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);
void blinker_task( void );
void handle_root( void );
void handle_404( void );
int init_eeprom( void );
int eeprom_read_buf( char *buf, unsigned int offset, unsigned int size );
int eeprom_write_buf( char *buf, unsigned int offset, unsigned int size );
int init_spiffs( void );

static unsigned char dev_mac[6];                  // MAC Address is 6 bytes
static WiFiManager wifiManager;
ESP8266WebServer server( 80 );
void configModeCallback ( WiFiManager *myWiFiManager );
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER, -25200, 60000);
Ticker blink_ticker;

const char* mqtt_server = MQTT_SERVER;
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting...");
  pinMode( STATUS_LED, OUTPUT );                // set pin for led STATUS
  WiFi.macAddress( ( byte * )dev_mac );         // fill in the devices mac address

  ArduinoOTA.onStart([]() {
      Serial.println("Upgrade Start");
  });
  ArduinoOTA.onEnd([]() {
      Serial.println("\nUpgrade End...Restarting");
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

  delay(1500);
  Serial.printf( "WiFi upgrade enabled\n\r");
  ArduinoOTA.begin();
  blink_ticker.attach(0.5, blinker_task);

  init_eeprom();                // start the eeprom NV storage
  init_spiffs();                // start the spiffs file system



}

void loop() {
  // put your main code here, to run repeatedly:
  char dev_name[ 64 ];
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
  delay( 100 );
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
  client.setCallback(callback);

  while( 1 ){
     run_server_tasks();
  }

}

void run_server_tasks( void )
{
  server.handleClient();
  ArduinoOTA.handle();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  return ;
}

// configModeCallback - gets called when WiFiManager enters configuration mode
void configModeCallback ( WiFiManager *myWiFiManager )
{
  static int fast_ticker = 0;
  Serial.println( "Entered config mode" );
  if( fast_ticker == 0 ) {
    blink_ticker.detach();
    blink_ticker.attach(0.2, blinker_task);
    fast_ticker = 1;
  }
  Serial.println( WiFi.softAPIP() );
  // print the SSID of the config portal
  Serial.println( myWiFiManager->getConfigPortalSSID() );
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "bradsrelay";
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Serial.printf("subscribing to topics\n\r");
      //client.subscribe("topic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length)
{
  unsigned int i;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  return ;
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
  return;
}

void handle_root( void )
{
  char buf[ 512 ];
  char mbuf[ 18 ];
  memset( buf, 0, sizeof( buf ) );
  sprintf( buf, "<HTML><HEAD></HEAD><BODY><pre>" );
  sprintf( buf+strlen( buf ), "<h3>%s</h3>", WiFi.hostname().c_str() );
  sprintf( buf+strlen( buf ), "<table cellpadding=3 cellspacing=0 border=0>" );
  sprintf( buf+strlen( buf ), "<tr><td>Hostname</td><td>%s</td></tr>", WiFi.hostname().c_str() );
  sprintf( buf+strlen( buf ), "<tr><td>IP</td><td>%s</td></tr>", WiFi.localIP().toString().c_str() );
  sprintf( buf+strlen( buf ), "<tr><td>MAC</td><td>");
  memset(mbuf, 0, sizeof(mbuf));
  sprintf(mbuf, "%02X:%02X:%02X:%02X:%02X:%02X", dev_mac[0],dev_mac[1],dev_mac[2],dev_mac[3],dev_mac[4],dev_mac[5]);
  sprintf(buf+strlen( buf ), "%s</td></tr>", mbuf);
  sprintf( buf+strlen( buf ), "<tr><td>ssid</td><td>%s</td></tr>",  WiFi.SSID().c_str() );
  sprintf( buf+strlen( buf ), "</table>");
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
