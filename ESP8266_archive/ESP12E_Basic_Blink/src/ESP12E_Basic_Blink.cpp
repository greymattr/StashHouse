#include <Arduino.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

#include <WiFiClient.h>

#include <PubSubClient.h>

//for LED status
#include <Ticker.h>

#define PIN_WIRE_SDA (4)
#define PIN_WIRE_SCL (5)
#define  A0  17
#define  D0  16
#define  D1   5
#define  D2   4
#define  D3   0
#define  D4   2
#define  D5   14
#define  D6   12
#define  D7   13
#define  D8   15
#define  D9   3       /* UART0_RX */
#define  D10  1       /* UART0_TX */
#define  LED_BUILTIN D0
#define  BUILTIN_LED D0
#define  SDA PIN_WIRE_SDA
#define  SCL PIN_WIRE_SCL

#define USER_WIFI_RESET 20
#define USER_RESET      40

#define redPin D2
#define greenPin D6
#define bluePin D5

#define STATE_INIT      1
#define STATE_CONFIG    10
#define STATE_CONNECTED  20

#define DEFAULT_DEVICE_NAME "ESP8266-12E"

#define UUID_PREFIX         "MCF1ESP"
/* UUID_PREFIX IN HEX 4d 43 46 31 45 53 50 0a */

#define DO_ECHO

static int prog_state = 0;
static int led_state = 0;

static WiFiManager wifiManager;
ESP8266WebServer server(80);

char mqtt_server[16] = "192.168.1.1\0";
char mqtt_cmd_topic[512];
char mqtt_status_topic[512];

byte dev_uuid[16]; // UUIDs in binary form are 16 bytes long
byte dev_mac[6];

WiFiClient wifiClient;

PubSubClient client(wifiClient);

Ticker ticker;

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  *(payload + length) = '\0';
  Serial.print("Recieved: ");
  Serial.printf("%s\n\r", (char *)payload);
  Serial.printf("from topic: %s\n\r", topic);
}

void set_color(int red, int green, int blue)
{
  #ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  #endif
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
  if((red==0)&&(green==0)&&(blue==0))led_state=0;
  else led_state=1;
  return ;
}

void rgb_off( void ) {
  set_color(0,0,0);
  led_state = 0;
  return;
}

void print_user_prompt( void ) {
  Serial.printf("\n\r> ");
  return ;
}

void print_hello( void ) {
  Serial.printf("  %s mcf1-IoT dev platform\n\r", DEFAULT_DEVICE_NAME);
  Serial.printf("  Entering command mode\n\r");
  return;
}

void print_help( void ) {
  Serial.printf("\n\rAvailable user commands:\n\r");
  Serial.printf("  help - this screen\n\r");
  Serial.printf("  show - show system information\n\r");
  Serial.printf("  reset - resets the saved wifi settings\n\r");
  Serial.printf("  reboot - stop everything ( doesn't work right )\n\r");
  Serial.printf("\n\r");
  return;
}

void show_info( void ) {
  int i;
  Serial.printf("\n\rCurrent Settings\n\r");
  Serial.printf("    ChipID: %d\n\r", ESP.getChipId());
  Serial.printf("PortalSSID: ");
  Serial.println(wifiManager.getConfigPortalSSID());
  Serial.printf("      SSID: ");
  Serial.println(WiFi.SSID());
  Serial.printf("       PSK: ");
  Serial.println(WiFi.psk());
  Serial.printf("  local ip: ");
  Serial.println(WiFi.localIP());
  Serial.printf("     gw ip: ");
  Serial.println(WiFi.gatewayIP());
  Serial.printf("  hostname: ");
  Serial.println(WiFi.hostname());
  Serial.printf("  mac addr: ");
  for(i=0;i<sizeof(dev_mac);i++){
    Serial.printf("%02X", dev_mac[i]);
    if(i<(sizeof(dev_mac)-1))Serial.printf(":");
  }
  Serial.printf("\n\r");
  Serial.printf("      UUID: ");
  for(i=0;i<sizeof(dev_uuid);i++){
    Serial.printf("%02X", dev_uuid[i]);
    if((i==3)||(i==5)||(i==7)||(i==9))printf("-");
  }
  Serial.printf("\n\r");
  Serial.printf("IoT Topics: network/");
  for(i=0;i<sizeof(dev_uuid);i++){
    Serial.printf("%02X", dev_uuid[i]);
  }
  Serial.printf("/useriot/subdev/");
  for(i=0;i<sizeof(dev_mac);i++){
    Serial.printf("%02X", dev_mac[i]);
  }
  Serial.printf("/<status/cmd>\n\r");
  Serial.printf(" MQTT Stat:");
  if(client.connected())Serial.printf(" Connected\n\r");
  else Serial.printf("Disconnected\n\r");

}

void tick()
{
  switch( prog_state ) {
    case STATE_INIT:
      if(led_state != 1) {
        set_color(255, 0, 0);  // red
      } else {
        rgb_off();
      }
    break;
    case STATE_CONFIG:
      if(led_state != 1) {
        set_color(255, 90, 0);  // yellow
      } else {
        rgb_off();
      }
    break;
    case STATE_CONNECTED:
      if(led_state != 1) {
        set_color(0, 255, 0);  // green
      } else {
        rgb_off();
      }
    break;
  }
  return;
}

void stop_blink( void ) {
  ticker.detach();
  rgb_off();
}

void start_blink( double rate ) {
  ticker.attach(rate, tick);
}

void clear( void ) {
  int i;
  for(i=0;i<20;i++){
    Serial.printf("\n\r");
  }
  return ;
}

int parse_input() {
  static char buf[128] = "\0";
  static int input_counter = 0;
  int ret = 0;

  while (Serial.available() > 0) {
    // read the incoming byte:
    buf[input_counter] = Serial.read();

    if( ( buf[input_counter]=='\n' ) || (input_counter==sizeof(buf)) ) {
      buf[input_counter]='\0';

      if (strcmp(buf, "reboot")==0){
        Serial.printf("rebooting system now\n\r");
        delay(500);
        //ESP.reset();
        ret = USER_RESET;
      } else if (strcmp(buf, "reset")==0) {
        Serial.printf("resetting wifi settings to factory defaults\n\r");
        delay(500);
        wifiManager.resetSettings();
        ret = USER_WIFI_RESET ;
        //return 10;
      } else if (strcmp(buf, "show")==0) {
        show_info();
      } else if (strcmp(buf, "help")==0) {
        print_help();
      } else if (strcmp(buf, "clear")==0) {
        clear();
      } else if (strcmp(buf, "stop led")==0) {
        Serial.println("Stopping LED blink");
        stop_blink();
      } else if (strcmp(buf, "start led")==0) {
        Serial.println("Starting LED blink");
        start_blink(2);
      } else {
        if( input_counter > 0) {
          Serial.printf("\n\r[ERR] cmd: \'%s\'\n\r", buf);
        }
      }
      memset(buf, 0, sizeof(buf));
      print_user_prompt();
      input_counter = 0;
    } else {
      #ifdef DO_ECHO
      Serial.print(buf[input_counter]);
      #endif
      if(buf[input_counter]!='\r')input_counter++;
    }
  }
  //delay(20);
  return ret;
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  prog_state = STATE_CONFIG;
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.printf("Please connect to WiFi network: ");
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  //ticker.attach(0.2, tick);
  start_blink(0.2);
  //ticker2.attach(1, parse_input);
}

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.macAddress(dev_mac);
  memset(dev_uuid, 0, sizeof(dev_uuid));
  memcpy(dev_uuid, UUID_PREFIX, 7);
  memcpy(dev_uuid+10, dev_mac, 6);
}

void handle_root( ) {
  server.send(200, "text/plain", "hello from esp8266!");




  return;
}

void handle_404(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  return;
}

void loop()
{
  int ok = 0;
  char buf[512];
  char str_uuid[64];
  char str_mac[12];

  memset(str_uuid, 0, sizeof(str_uuid));
  sprintf(str_uuid, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", dev_uuid[0], dev_uuid[1], dev_uuid[2], dev_uuid[3], dev_uuid[4], dev_uuid[5], dev_uuid[6], dev_uuid[7], dev_uuid[8], dev_uuid[9], dev_uuid[10], dev_uuid[11], dev_uuid[12], dev_uuid[13], dev_uuid[14], dev_uuid[15]);
  memset(str_mac, 0, sizeof(str_mac));
  sprintf(str_mac, "%02X%02X%02X%02X%02X%02X", dev_mac[0],dev_mac[1],dev_mac[2],dev_mac[3],dev_mac[4],dev_mac[5]);

  start_everything:
  Serial.printf("\n %s v.0.0.mcf1 starting...\n", WiFi.hostname().c_str());
  //reset settings - for testing
  //wifiManager.resetSettings();
  prog_state = STATE_INIT;
  // start ticker with 0.5 because we start in AP mode and try to connect
  //ticker.attach(0.6, tick);
  start_blink(0.6);
  delay(500);

  if( ok != USER_RESET ) {
    //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
    wifiManager.setAPCallback(configModeCallback);
  }
  ok = 0;
  //  this below will block if it can not connect to a wifi AP,
  //  and has to open the config AP
  //  fetches ssid and pass and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  //
  Serial.printf("starting Auto connect wifi (%s)\n\r", WiFi.hostname().c_str());
  if (!wifiManager.autoConnect(WiFi.hostname().c_str())) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    goto start_everything;
  }
  //if you get here you have connected to the WiFi
  prog_state = STATE_CONNECTED;
  print_hello();
  Serial.printf("connected to SSID: ");
  Serial.println(WiFi.SSID());
  Serial.println("starting web server");

  server.on("/", handle_root);
  server.onNotFound(handle_404);
  server.begin();
  //slow down led
  //ticker.attach(2, tick);
  start_blink(2);
  Serial.printf("local ip: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  memset(mqtt_cmd_topic, 0, sizeof(buf));
  memset(mqtt_status_topic, 0, sizeof(buf));
  sprintf(mqtt_cmd_topic, "network/%s/useriot/subdev/%s/cmd", str_uuid, str_mac);
  sprintf(mqtt_status_topic, "network/%s/useriot/subdev/%s/status", str_uuid, str_mac);

  if (client.connect((char*) str_mac)) {
    Serial.println("Connected to MQTT broker");
    Serial.printf(" Status topic is: %s\n\r", mqtt_status_topic);
    Serial.printf("Command topic is: %s\n\r", mqtt_cmd_topic);


    if (client.publish(mqtt_status_topic, "ESP8266 device online")) {
      Serial.println("Publish ok");
      client.subscribe(mqtt_cmd_topic);
    }
    else {
      Serial.println("Publish failed");
    }
  }
  else {
    Serial.println("MQTT connect failed");
    Serial.println("Skipping...");
  }


  print_user_prompt();
  while(1) {
  //delay(1000);
    ok = parse_input();
    if( ok != 0 )break;
    server.handleClient();
    if (client.connected()){
      client.loop();
    } else {
      if (client.connect((char*) str_mac)) {
        Serial.println("Connected to MQTT broker");
        client.subscribe(mqtt_cmd_topic);
      } else {
        Serial.printf("MQTT connect to %s failed\n\r", mqtt_server);
      }
    }
  }
  stop_blink();
  server.close();
  if( ok == USER_WIFI_RESET )goto start_everything;
  if( ok == USER_RESET )goto start_everything;


  return;
}
