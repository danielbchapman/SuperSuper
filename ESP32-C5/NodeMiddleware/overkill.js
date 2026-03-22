const axios = require('axios');

// Configuration
const WATCHOUT_IP = '192.168.0.177'; // Replace with your computer's IP
const WATCHOUT_PORT = 3019;           // Default port for WATCHOUT 7 API
const VARIABLE_NAME = 'Opacity';     // Name of the Generic Input in WATCHOUT

/**
 * Updates a variable via the WATCHOUT 7 REST API
 */
async function updateVariable(name, value) {
    const url = `http://${WATCHOUT_IP}:${WATCHOUT_PORT}/v0/inputs/${name}`;
    console.log("attempting post...")
    try {
        const response = await axios.post(url, {
            value: value
        });
        console.log(`Success: Set ${name} to ${value}. Status: ${response.status}`);
    } catch (error) {
        console.error('Error updating WATCHOUT:', error.response ? error.response.data : error.message);
    }
}

// Set 'Opacity' to 0.75
updateVariable(VARIABLE_NAME, 0.75);