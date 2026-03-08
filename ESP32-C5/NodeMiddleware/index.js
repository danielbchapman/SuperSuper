const dgram = require('dgram')
const server = dgram.createSocket('udp4')
const chalk = require('chalk').default

const PORT = 53007;
let lastSequence = -1;
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
            console.warn( chalk.orange.bold(`\n[${id}] Detected ${gap} dropped packet(s) before Seq: ${seq}`));
        }

        lastSequence = seq

        console.log(chalk.green(`[${id}] Seq: ${seq} | Pulses: ${pulses} | From: ${rinfo.address} | RSSI: ${rssi}dBm | TX: ${txPower}`));
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