/*
  Super Super Arduino v3

  This sends position and velocity and has a very simple calibration
  interface where you can send OSC messages on port 8001 to reset or nudge it.

 */

#include <string.h>
#include <WiFiS3.h>
#include <OSCMessage.h>
#include "secrets.h"
//ENCODE VARIABLES
//Position
bool DEBUG = false;
long index  = -999;

//Velocity
float indexLast = -999;
float vTimeLast = -1;
float vLast = 0.0;

//Encoder
int a = -1;
int a_now = -1;
int b = -1;


long time = -1L;
int STEP = (int) (1000.0/30.0);

//WIFI VARIABLES
int status = WL_IDLE_STATUS;
 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = WIFI_SSID
char pass[] = WIFI_PASS;    
int keyIndex = 0;            

//LOCAL COMMAND PORT
unsigned int RECEIVE_PORT = 8001;      // local port to listen on

char packetBuffer[256]; //buffer to hold incoming packet
char ReplyBuffer[] = "acknowledged\n";       // a string to send back

IPAddress server = IPAddress(192, 168, 0, 33);
IPAddress backup = IPAddress(192, 168, 0, 138);
IPAddress dev = IPAddress(192, 168, 0, 43);
WiFiUDP Udp;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  index = 0;

  //ENCODER LOGIC
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), interruptPin2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(3), interruptPin3, CHANGE);
  Serial.println("Super Super, the automatic super starting:");  

  Serial.println("Attempting to connect to Wifi");
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  Serial.println(fv);
  // if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
  //   Serial.println("Please upgrade the firmware");
  // }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
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
      index++;
    } else {
      index--;
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
  long time_now = millis();
  if(time_now - time > STEP) {
    if(DEBUG) {
      Serial.print("index:");
      Serial.print(index);
      Serial.print(",time:");
      Serial.print(time_now);
      Serial.println();
    } else {
      if(indexLast != index) {
        Serial.println(index);
      }
    }

    //Update Velocity
    float mTime = (float) micros();
    float steps = (float) (index - indexLast);
    float velocity = steps / (mTime - vTimeLast); 

    indexLast = index;
    vTimeLast = mTime;
    vLast = velocity;
     //We are just going to send
    // send a reply, to the IP address and port that sent us the packet we received
    // sendData(server);
    //sendData(backup);
    //sendData(dev);

    sendDataOSC(server);
    readUdp();
    time =  time_now;
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
      index = value;
      indexLast = value;
      vLast = 0.0;  
    } else if (s.indexOf("nudge") > -1) {
      Serial.print("Nudge called ");
      Serial.println(value);
      index = index + value;
      indexLast = index;
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
  msg.add((int32_t) index);
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
  BUFFER[0] = index >> 24 & 0xFF;
  BUFFER[1] = index >> 16 & 0xFF;
  BUFFER[2] = index >> 8 & 0xFF;
  BUFFER[3] = index >> 0 & 0xFF;
  int startPacket = Udp.beginPacket(a, 53008); 
  //Serial.println(startPacket);
  //Serial.println("xx");
  Udp.write(BUFFER, 12);
  // Udp.write(index >> 24 & 0xFF);
  // Udp.write(index >> 16 & 0xFF);
  // Udp.write(index >> 8 & 0xFF);
  // Udp.write(index >> 0 & 0xFF);

  // Udp.write(time >> 24 & 0xFF);
  // Udp.write(time >> 16 & 0xFF);
  // Udp.write(time >> 8 & 0xFF);
  // Udp.write(time >> 0 & 0xFF);
  // Udp.write("ss01");
  int end = Udp.endPacket();

  //Serial.println(index);
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