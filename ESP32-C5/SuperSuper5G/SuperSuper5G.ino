/*
  Super Super Arduino v3

  This sends position and velocity and has a very simple calibration
  interface where you can send OSC messages on port 8001 to reset or nudge it.

  The Arduino Uno Wifi R4 version have a bug where WiFi will drop into low power 
  for no reason in the latest firmware, and you can't disable it. I've filed a bug report (Late 2025, but I have low hopes of seeing that bridge fixed).

  Track the status of this potential problem here:
  https://github.com/arduino/ArduinoCore-renesas/issues/512
 */

#include <string.h>
#include <WiFi.h>
#include <OSCMessage.h>
#include "secrets.h"
//ESP32 direct PCNT Hardware (we're leveraging the fact this chip can just do this)
#include "driver/pcnt.h"
const int ESP32_PIN_A = 22;
const int ESP32_PIN_B = 23;
const pcnt_unit_t PCNT_UNIT = PCNT_UNIT_0;

//ENCODE VARIABLES
//Position
bool DEBUG = false;
long ss_index  = -999L;

//Velocity
float ss_indexLast = -999;
float vTimeLast = -1;
float vLast = 0.0;

//Encoder
int a = -1;
int a_now = -1;
int b = -1;


long ss_time = -1L;
int STEP = (int) (1000.0/30.0);
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASS;    
int keyss_ss_index = 0;            

//LOCAL COMMAND PORT
unsigned int RECEIVE_PORT = 8001;      // local port to listen on

char packetBuffer[256]; //buffer to hold incoming packet
char ReplyBuffer[] = "acknowledged\n";       // a string to send back

IPAddress server = IPAddress(192, 168, 1, 124);
//IPAddress backup = IPAddress(192, 168, 0, 138);
//IPAddress dev = IPAddress(192, 168, 0, 43);
WiFiUDP Udp;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  ss_index = 0;

  Serial.println("Super Super, the automatic super starting:");  
  Serial.println("Attempting to connect to Wifi");
  // check for the WiFi module:
  // if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
  //   Serial.println("Please upgrade the firmware");
  // }

  // attempt to connect to WiFi network:
  WiFi.mode(WIFI_STA);
  int retry = 0;
  WiFi.begin(ssid, pass);
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    // if(retry++ > 10) {
    //   delay(60000); //wait a minute
    //   retry = 0;
    //   //WiFi.
    //   delay(5000);
    //   //WiFi.begin(ssid, pass);
    //   //This should failover to a webUI or something to setup the SuperSuper
    // };

    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected to WiFi");
  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  Udp.begin(RECEIVE_PORT);
}


void interruptPin2() {
  a_now = digitalRead(2);
  if(a_now != a) {
    if(a_now == b) {
      ss_index++;
    } else {
      ss_index--;
    }
  }

  a = a_now;
}

void interruptPin3() {
  b = digitalRead(3);
}

int wait = 22; //44HZ, 1000/44
void loop() {
  //METER LOOP to 44Hz
  int16_t count = 0;
  
  // 5. Read the hardware register directly
  pcnt_get_counter_value(PCNT_UNIT, &count);
  long time_now = millis();
  if(time_now - ss_time > STEP) {
    Serial.print("Position: ");
    Serial.println(count);

    if(DEBUG) {
      Serial.print("ss_index:");
      Serial.print(ss_index);
      Serial.print(",time:");
      Serial.print(time_now);
      Serial.println();
    } else {
      if(ss_indexLast != ss_index) {
        Serial.println(ss_index);
      }
    }

    //Update Velocity
    float mTime = (float) micros();
    float steps = (float) (ss_index - ss_indexLast);
    float velocity = steps / (mTime - vTimeLast); 

    ss_indexLast = ss_index;
    vTimeLast = mTime;
    vLast = velocity;
     //We are just going to send
    // send a reply, to the IP address and port that sent us the packet we received
    // sendData(server);
    //sendData(backup);
    //sendData(dev);

    sendDataOSC(server);
    readUdp();
    ss_time =  time_now;
  }
  return; 
}

uint8_t OSC_BUFFER[1024];

void readUdp() {
  int len = Udp.parsePacket();
  if(len > 0) {
    OSCMessage msg;
    Udp.readBytes(OSC_BUFFER, len);
    msg.fill(OSC_BUFFER, len);
    if(msg.hasError()) {
      Serial.println("Misformated message");
    }
    String s = String(msg.getAddress());
    long value = (long) msg.getInt(0);
    
    //Decision Tree
    if( s.indexOf("calibrate") > -1) {
      Serial.print("Calibrate called ");
      Serial.println(value);
      ss_index = value;
      ss_indexLast = value;
      vLast = 0.0;  
    } else if (s.indexOf("nudge") > -1) {
      Serial.print("Nudge called ");
      Serial.println(value);
      ss_index = ss_index + value;
      ss_indexLast = ss_index;
      vLast = 0.0;  
    } else {
      Serial.print("unknown command");
      Serial.println(s);
    }
   
    if(DEBUG) {
      Serial.println();
      Serial.print("\t");
      Serial.print(len);
      Serial.print(msg.getAddress());
      Serial.println( value );
    }

    msg.empty();

    // for(int i = 0; i < len; i++) {
    //   Serial.print(OSC_BUFFER[i]);
    //   Serial.print("->");
    //   Serial.println(i);
    // }
    // Serial.println("A PACKET");
  }
}

OSCMessage msg("/arduino/live");
void sendDataOSC(IPAddress to) {
  msg.add((int32_t) ss_index);
  msg.add(vLast);
  Udp.beginPacket(to, 8000);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}

char BUFFER[] = {
  0x00,
  0x00,
  0x00,
  0x00,

  0x00,
  0x00,
  0x00,
  0x00,

  's',
  's',
  '0',
  '1'
};

/* 
This is the old method, we don't need it anymore, but you
could choose to send over a simple UDP packet if that's easier for you.

This does not send the velocity as it was version 1 and we swapped to OSC for simplicity
*/
inline void sendData(IPAddress a) {
  
  // BUFFER
  BUFFER[0] = ss_index >> 24 & 0xFF;
  BUFFER[1] = ss_index >> 16 & 0xFF;
  BUFFER[2] = ss_index >> 8 & 0xFF;
  BUFFER[3] = ss_index >> 0 & 0xFF;
  int startPacket = Udp.beginPacket(a, 53008); 
  //Serial.println(startPacket);
  //Serial.println("xx");
  Udp.write( (const uint8_t*)BUFFER, 12);
  // Udp.write(ss_index >> 24 & 0xFF);
  // Udp.write(ss_index >> 16 & 0xFF);
  // Udp.write(ss_index >> 8 & 0xFF);
  // Udp.write(ss_index >> 0 & 0xFF);

  // Udp.write(time >> 24 & 0xFF);
  // Udp.write(time >> 16 & 0xFF);
  // Udp.write(time >> 8 & 0xFF);
  // Udp.write(time >> 0 & 0xFF);
  // Udp.write("ss01");
  int end = Udp.endPacket();

  //Serial.println(ss_index);
  //Serial.print("-");
  //Serial.println(end);
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}