# ESP8266_multihop_network
This project has the goal to make it easy to deploy a multihop network of ESP8266 Microcontrollers, which gather temperature and humidity data.
Furthermore the aim for the type of the deployment is a near zero-config deployment. ESP-NOW is beeing used as the communication protocol between the individual modules.
There are nodes and gateways. The nodes have two tasks: gather and send data (temperature and humidity) and transmit other payloads if needed. The gateways has the task to show the transmitted data from the nodes to the user, but currently the work of a gateway has been abandon.

## Node
As already mentioned a single node gathers information about the temperature and humidity. Afterwards it sends these information to the Broadcast-Address.
If the node receives a packet it checks weather it should be retransmited. If this is the case, the module trys to retransmit the foreign packet.

### Hardware of the node
The node itself is a NodeMCU ESP8266. Attached to the node is a DHT22, which is beeing used to gather temperature data and humidity data.

| DHT22-Pin | NodeMCU ESP8266-Pin |
| ----------- | ----------- |
| + | 3V3 |
| Data | D1/GPIO 5 |
| - | GND |

As shown in the table above, the DHT22 is beeing supplied with a 3,3 V and the Data-Pin is connected to the fifth GPIO pin of the NodeMCU ESP8266.

### Software of the node
The software of a node can be found within the folder called [node](https://github.com/RaphWa/ESP8266_multihop_network/blob/main/node). It is written in C/C++ and the Arduino IDE was beeing used to do so. Three librarys have been used within this software: ESP8266WiFi, [espnow](https://github.com/esp8266/Arduino/blob/master/tools/sdk/include/espnow.h) and [DHT](https://github.com/adafruit/DHT-sensor-library).
In general, the node waits until a specified number has been reached. If this is the case it gathers the temperature data and humidity data and sends it to the broadcast address and resets a counter variable. While it waits it listens for packets. If a packet has been reiceived by the node it checks if it should be retransmitted. If the reiceved packet is eligible for retransmission the node will try to do so to the broadcast address. If not the packet will not be retransmited. 

## Ideas for a gateway
Currently, the inital idea of a NodeMCU ESP8266 with a LCD1602 with an I2C-Interface as a gateway, called G01, has been abandon. In the following text are a few ideas for a gateway:
- NodeMCU ESP8266 with a LCD1602 with an I^2^C-Interface: displays all received data, one 16x2 page consists out of the name of the sender in the first line and the temperature data and humidity data in the second line, loops automatically (or due to a push of a button) through all nodes
- NodeMCU ESP8266 sends data to a MQTT-Server (perhaps a server on a Raspberry Pi), which is visualized with Node-Red
- NodeMCU ESP8266 serves as a webserver, shows received data on a webpage

