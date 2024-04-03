
/**
 * Super Super Relay Sketch version 0.9 
 * 
 * This is a simple OSC relay from the arduino to QLab. In our case it uses
 * the local IP addresses for our system, but change as needed.
 */
import java.util.*;
import oscP5.*;
import netP5.*;
  
OscP5 oscP5;
NetAddress qlab;

public static boolean DEBUG = true;
public static boolean DEBUG_SHOW_DETAILED_SENSOR_DATA = true;

public static int OSC_INCOMING_ARDUINO = 8000;
//float DELAY = 200f;//Delay in milliseconds for the system, 200 seemed about right for us (3-4 frames)
//int LAST_POS_X = 0;

//SENSOR PAIRED TO PROJECTION MAP (Refactor into array)
//Pair<Float> QLAB_MIN_MAX  = new Pair<>(-576.0f, 585.0f);
//Pair<Integer> SENSOR_MIN_MAX = new Pair<>(0, -991);

HashMap<String, MappedSensor> routes = new HashMap<>();

void setup() {
  //We have a set of known IP addresses and commands
  addRoute("192.168.0.174", "/cue/ss1/translation/live", 0, -991, -576f, 585f);
  addRoute("192.168.0.220", "/cue/ss2/translation/live", 0, -991, -576f, 585f);
  addRoute("192.168.0.97", "/cue/ss3/translation/live", 0, -991, -576f, 585f);
  //Processing stuff
  size(400,400);
  frameRate(30);//Meter to 30 frames a second
 
  oscP5 = new OscP5(this, OSC_INCOMING_ARDUINO);
 
  //We assume QLab is running on the same machine so it is faster. Don't be slower, the delay is 200ms as is.
  qlab = new NetAddress("127.0.0.1", 52000);
}

/**
 * For some reason this returns /ip_address so we just add that here
*/
void addRoute(String oscIp, String oscCommand, int sensorMin, int sensorMax, float qlabMin, float qlabMax) {
  MappedSensor sensor = new MappedSensor(oscIp);
  sensor.oscCommand = oscCommand;
  
  sensor.sensorMin = sensorMin;
  sensor.sensorMax = sensorMax;
  
  sensor.qlabMin = qlabMin;
  sensor.qlabMax = qlabMax;
  
  routes.put("/"+oscIp, sensor);
}
void draw() {
  background(0);  
}

 //<>//
/** Read incomming OSC events here */
void oscEvent(OscMessage msg) {
  
  //We should probably check this...it is really lazy not to.
  String ipAddress = msg.address();
  MappedSensor sensor = routes.get(ipAddress);
  if(sensor == null) {
    println("[warning] sensor: " + ipAddress + " is not mapped");
    return;
  }
  
  //Process via sensor class,
  OscMessage toSend = sensor.handleOscFromArduino(msg);
  if(toSend != null) {
    oscP5.send(toSend, qlab);
  } //<>//
}
