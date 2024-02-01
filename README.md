# ESP8266_multihop_network
This project has the goal to make it easy to deploy a multihop network of ESP8266 Microcontrollers, which gather temperature and humidity data.
Furthermore the aim for the type of the deployment is a near zero-config deployment. ESP-NOW is beeing used as the communication protocol between the individual modules.
There are nodes and gateways. The nodes have two tasks: gather and send data (temperature and humidity) and transmit other payloads if needed. The gateways has the task to show the transmitted data from the nodes to the user. 

## Node
As already mentioned a single node gathers information about the temperature and humidity. Afterwards it sends these information to the Broadcast-Address.
If the node receives a packet it checks weather it should be retransmited. If this is the case, the module trys to retransmit the foreign packet.

### Hardware of the node
The node itself is a NodeMCU ESP8266. Attached to the node is a DHT22.

| DHT22-Pin | NodeMCU ESP8266-Pin |
| ----------- | ----------- |
| + | 3V3 |
| Data | D1/GPIO 5 |
| - | GND |

As shown in the table above, the DHT22 is beeing supplied with a 3,3 V and the Data-Pin is connected to the fifth GPIO pin of the NodeMCU ESP8266.


## Gateway
The gateway is a module which shows the user the received readings of the nodes.

