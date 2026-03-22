const dgram = require('dgram')
const server = dgram.createSocket('udp4')
const chalk = require('chalk').default
const osc = require('osc')

const PORT = 53007;
let lastSequence = -1;

//let ROTATION = 10000;
let ROTATION = 37536;
let INVERT = false

server.on('error', (err) => {
  console.log(`Server error:\n${err.stack}`);
  server.close();
});

server.on('message', (msg, rinfo) => {
  // Expected structure from ESP32:
  // 4 bytes: ID (char[4])
  // 4 bytes: Sequence (uint32_t, Little Endian)
  // 4 bytes: Pulses (int32_t, Little Endian)
  
  if (msg.length >= 14) {
    const id = msg.toString('utf8', 0, 4);
    const seq = msg.readUInt32LE(4);
    const pulses = msg.readInt32LE(8);
    const rssi = msg.readInt8(12);
    const txPower = msg.readInt8(13);

    if(seq > lastSequence) { //ok
        //check for gap
        const gap = seq - lastSequence - 1;
        if (lastSequence !== -1 && gap > 0) {
            console.warn( chalk.magenta.bold(`\n[${id}] Detected ${gap} dropped packet(s) before Seq: ${seq}`));
        }

        lastSequence = seq

        console.log(chalk.green(`[${id}] Seq: ${seq} | Pulses: ${pulses} | From: ${rinfo.address} | RSSI: ${rssi}dBm | TX: ${txPower}`));
        sendNum(pulses)
    } else {
        console.warn(`[${id}] Seq: ${seq} | Pulses: ${pulses} | From: ${rinfo.address} | RSSI: ${rssi}dBm | TX: ${txPower}`)
    }
    
  } else {
    console.log(`Received malformed packet of length ${msg.length}`);
  }
});

server.on('listening', () => {
  const address = server.address();
  console.log(`UDP Server listening on ${address.address}:${address.port}`);
  console.log(`Targeting 120Hz stream...`);
});

server.bind(PORT);

//////////////////////
////DEMO OSC
//////////////////////

var udpPort = new osc.UDPPort({
    localAddress: "0.0.0.0",
    localPort: 57121,
    metadata: true
});

udpPort.open();

let sendNum = (num) => {
  console.log(`osc not ready-> ${num}`);
}

const lerp = (a, b, t) => a + t * (b - a);

let sendNumActive = (num) => {
    let percent = (num / ROTATION) % ROTATION;
    let rotation;
    if(INVERT) {
      const positive = lerp(360, 0, percent) % 360
      //rotation = 360 - positive
    } else {
      rotation = lerp(0, 360, percent) % 360
    } 

    //If negative rotation, just report the positive version
    if(rotation < 0) {
      const pos = rotation;
      rotation = 360 + rotation
      console.log(chalk.magenta(`Adjust for Negative ${pos} from  ${rotation}`))      
    }
    console.log(chalk.blue(`OSC->${num} | ${num / ROTATION} | ${percent} | ${rotation}`))
    udpPort.send({
      address: "/ss",
      args:[ {
        type: "i",
        value: rotation
      }]
    }, "127.0.0.1", 8000)
    udpPort.send({
      address: "/ss",
      args:[ {
        type: "i",
        value: rotation
      }]
    }, "192.168.0.177", 8000)
}

udpPort.on("ready", function () {
  sendNum = sendNumActive;
});