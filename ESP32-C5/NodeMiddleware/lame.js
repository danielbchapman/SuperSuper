const dgram = require('dgram');
const client = dgram.createSocket('udp4');

// Configuration
const WATCHOUT_IP = '127.0.0.1'; // Change to your WATCHOUT computer's IP
const WATCHOUT_PORT = 3040;         // Default WATCHOUT control port

/**
 * Updates a Generic Input variable in WATCHOUT
 * @param {string} name - The name of the input as defined in WATCHOUT
 * @param {number} value - The numerical value to assign
 */
function updateWatchoutVariable(name, value) {
    // Command format: setInput "VariableName" Value followed by a Carriage Return (\r)
    const command = `setInput "${name}" ${value}\r`;
    const message = Buffer.from(command);

    client.send(message, WATCHOUT_PORT, WATCHOUT_IP, (err) => {
        if (err) {
            console.error('Failed to send UDP packet:', err);
        } else {
            console.log(`Command sent: ${command.trim()}`);
        }
    });
}

let i = 0.0;
setInterval(()=>{
    updateWatchoutVariable('authenticate', 1);
    updateWatchoutVariable('Opacity', (i % 10)/10);
    i++
}, 1000)
// Example usage: Set variable "Opacity" to 0.5


// Note: In a real app, keep the client open for frequent updates. 
// Use client.close() only when the application is shutting down.