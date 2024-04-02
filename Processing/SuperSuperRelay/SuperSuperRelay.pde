
/**
 * Super Super Relay Sketch version 0.9 
 * 
 * This is a simple OSC relay from the arduino to QLab. In our case it uses
 * the local IP addresses for our system, but change as needed.
 */
 
import oscP5.*;
import netP5.*;
  
OscP5 oscP5;
NetAddress qlab;

boolean DEBUG = true;
int lastIndex = 0;
int OSC_INCOMING_ARDUINO = 8000;
float DELAY = 200f;//Delay in milliseconds for the system, 200 seemed about right for us (3-4 frames)
float LAST_POS_X = 0f;

//SENSOR PAIRED TO PROJECTION MAP (Refactor into array)
Pair<Float> QLAB_MIN_MAX  = new Pair<>(-576.0f, 585.0f);
Pair<Integer> SENSOR_MIN_MAX = new Pair<>(0, -991);

void setup() {
  size(400,400);
  frameRate(30);
 
  oscP5 = new OscP5(this, OSC_INCOMING_ARDUINO);
 
  //We assume QLab is running on the same machine so it is faster. Don't be slower, the delay is 200ms as is.
  qlab = new NetAddress("127.0.0.1", 53000);
}


void draw() {
  background(0);  
}

 //<>//
/** Read incomming OSC events here */
void oscEvent(OscMessage msg) {
  
  //We should probably check this...it is really lazy not to.
  String pattern = msg.addrPattern();
  //We use "whatever" message value comes in
  int value = msg.get(0).intValue();
  //This is the velocity from Arduino
  float velocityMicrosToTicks = msg.get(1).floatValue();
  
  float vOffset = velocityMicrosToTicks * DELAY * 1000f; //Microseconds to Milliseconds * delay
  if(vOffset != 0) {
    if(DEBUG) {
      println("\t->" + value + " offset by velocity " + vOffset);
    }
    
  }
  
  //ALGORITHM FOR POSITION--trivial
  float by = (float)value / (SENSOR_MIN_MAX.two - SENSOR_MIN_MAX.one);
  float posX = lerp(QLAB_MIN_MAX.one, QLAB_MIN_MAX.two, by) ;
   
  //Don't SPAM QLab, it can't handle it
  lastIndex = value;
  
  if(LAST_POS_X != posX) {
    //ss1 is the name of our cue (super super 1)
    OscMessage live = new OscMessage("/cue/ss1/translation/live");//QLAB 5 API, use live
    //live.add(posX + animationOffset);
    live.add(posX - vOffset);
    live.add(200.0f);
    oscP5.send(live, qlab);  
  }
  
  LAST_POS_X = posX;
}
