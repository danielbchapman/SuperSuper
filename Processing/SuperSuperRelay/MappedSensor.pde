//Only handling X movement, we would need to change for two axis or y
public class MappedSensor {
  //Required at construction
  public String ipAddress;
  public float delay = 320;
  //Set after the fact
  public int sensorMin;
  public int sensorMax;
  public float qlabMin;
  public float qlabMax;
  public float qlabY;
  public String oscCommand;
  
  //Tracking Variables
  public int lastIndex;

  
  public MappedSensor(String _ipAddress) {
    this.ipAddress = _ipAddress;
  }
  
  /** Returns the OSC message to send to the media server */
  public OscMessage handleOscFromArduino(OscMessage msg) {
      
    //We should probably check this...it is really lazy not to.
    String pattern = msg.addrPattern();
   
    //We use "whatever" message value comes in
    int value = msg.get(0).intValue();
    //This is the velocity from Arduino
    float velocityMicrosToTicks = msg.get(1).floatValue();
    
    float vOffset = velocityMicrosToTicks * delay * 1000f; //Microseconds to Milliseconds * delay
    if(vOffset != 0) {
      if(SuperSuperRelay.DEBUG && SuperSuperRelay.DEBUG_SHOW_DETAILED_SENSOR_DATA) {
        println("\t->" + value + " offset by velocity " + vOffset);
      }
    }
    
    //ALGORITHM FOR POSITION--trivial
    float by = (float)value / (sensorMax - sensorMin);
    float posX = lerp(qlabMin, qlabMax, by) ;
     
    //Don't SPAM QLab, it can't handle it  
    if(lastIndex == value) {
      return null;
    }
    
    lastIndex = value;
    
    if(SuperSuperRelay.DEBUG) {
      println("[" + ipAddress + "] " + vOffset);
    }
    
    OscMessage live = new OscMessage( oscCommand );//QLAB 5 API, use live
    live.add(posX - vOffset);
    live.add(200.0f);
    
    return live;
  }
}
