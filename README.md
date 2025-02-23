# SuperSuper
Automated scenery tracking using arduino's and encoders for single axis positioning. It helps our supers moving scenery be more super...

SuperSuper provides simple encoder mappings over UDP, OSC, and Serial interfaces for tracking scenery. The intial project was greated on the UnoRev4 Wifi which is capable of generating streams of data somewhere between 30 and 60Hz depending on the number of sensors and the network and wireless environment. Currently each Arduino needs to be flashed with code specifying the IP address to send data to. This requires a network with reserved addresses or a static configuration. Additionally, the Arduinos connect using DHCP.

# Planned Features
An OSC API for programming Arduinos on the network so that the application does not need to be recompiled and uploaded when environments change
 - OSC Command for setting broadcast address
 - OSC Command for setting the broadcast port
 - OSC Command for setting the broadcast delay (used with velocity for delay compensation)
 - OSC Command for setting the broadcast mode
 - UDP Broadcast Information Packets sent once every 5-10 seconds

# Arduino Uno Rev4 Wifi 
 - Documentation of the installation process
 - Configuration tutorials and a separate branch

## Outstanding
 - Reconfigure to use a lower level encoder library, currently this is a simple home-brew solution, and while functional, can be optimized significantly
 - Refactor to use timers for network communication instead of `delay`
 - Documentation of Serial output for sensors that do not need to be wireless (improved latency)

# Arduino Giga R1 Wifi
(in development)
 - Implementation of a dual-core senesor setup using the m4 for encoder tracking and the m7 for Wifi functinoality
 - Implementation using a single-core setup
 - Testing of 120 to 240Hz for packets over UDP
 - Testing of delay velocity in the packet, rather than at the client 

# Node.js Client
 - Implementatino of a Node.js client for controlling and mapping the sensors before relaying to applications like QLab, Watchout, and Isadora. 

