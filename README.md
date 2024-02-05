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
The following diagram shows the software of the node represented as a UML class diagram.

![node_uml_class_diagram](https://github.com/RaphWa/ESP8266_multihop_network/blob/main/images/node_uml_class_diagram.svg)

On the right side of the class node are three structs displayed. The head_struct consists out of three variables and contains general information about the packet. The payload_struct consists out of two variables, which store a temperature value and a humidity value. The struct data_packet consists out of two variables, one variable is of the type head_struct and the other variable is the type of payload_struct. This struct represents the packet, which is beeing send between the modules. As seen in the diagram above, the class node uses three librarys. Furthermore, it contains 14 variables and 10 functions (and the definition of the three structs).

#### Variables in the class node 
The first three declared variables are used pins, two pins for controlling two LEDs on the NodeMCU ESP8266 and one pin to get the data from the DHT22. The next two variables are for communicating to the DHT22. The three variables called MAX_LENGTH_DATA_PACKET_ID_AR, data_packet_id_arr, data_packet_id_arr_index_counter are beeing used to keep track of ids of already received packets. The array data_packet_id_arr, the storage of the ids, has a length of MAX_LENGTH_DATA_PACKET_ID_AR. The function store_data_packet_id(id) handles this storage: The first id is beeing stored at index 0, the second id is beeing stored at index 1 and so on... . If the array is full, the first element of the array will be overritten, then the next and so on... . The variables MINIMUM_DISTANCE_BETWEEN_TRANSMITTING_DATA_PACKETS and old_millis are beeing used to emulate parallel working in the function loop() in order to enable the module to listen for new packets and to send new packets from time to time. The variable DELAY_LED_BLINK stores the delay of the two LEDs between on and off in milliseconds. Naming the module happens later, and if it happens, the name (which is the MAC address of the module) will be stored in the variable modul_name. The broadcast address is stored in the array broadcast_address and the last declared variable with the name packet is beeing used to temporarily store the received packets. The function transmit_data_packet(pkt) stores the id of pkt, sets the esp now role of the module to ESP_NOW_ROLE_CONTROLLER, transmitts the data_packet pkt and sets the esp now role of the module to ESP_NOW_ROLE_SLAVE. 

#### Functions in the class node
The function store_data_packet_id(id) handles the array data_packet_id_arr and stores the given id (for more information you may look into the text above). The function is_data_packet_allowed_to_be_transmitted(pkt) uses the two functions is_data_packet_id_known(id) and is_this_module_addressee_of_data_packet(addressee_of_pkt) to determin if the data_packet pkt is allowed to be transmitted. It is allowed to be transmitted, if its id is not stored in the array data_packet_id_arr and if the modul, which trys to transmitt this packet, is not the addresse of this packet. Only if both conditions are met the return value of the function is_data_packet_allowed_to_be_transmitted(pkt) will be true, otherwise it will be false. In addtion to transmitting a packet the function transmit_new_data_packet(addr_name, pyld) generates a id and constructs a data_packet with the given parameters. Furthermore it gives some feedback with the serial interface and a LED. The function if_data_packet_transmitted(addr, status) gives some feedback with the serial interface if a packet has been transmitted by the module and the function if_data_packet_received(addr, data, received_bytes) gives some feedback with the serial interface and one/both LED/LEDs if a packet has been received and has been retransmitted by the module.
The last two functions are setup() and loop(). The function setup() initializes the module (initialization of the DHT22, the WiFi, the name of the modul, the two pins for the LEDs, the serial interface and ESP-NOW) and gives feedback with the serial interface about the fact that the module is now initialized. The function loop() uses the variables old_millis, MINIMUM_DISTANCE_BETWEEN_TRANSMITTING_DATA_PACKETS and the function millis() to determine if it is time to measure the temerature and humidity and send the maesured data to the broadcast address. If this is the case, it will do so.

## Ideas for a gateway
Currently, the inital idea of a NodeMCU ESP8266 with a LCD1602 with an I2C-Interface as a gateway, called G01, has been abandon. In the following text are a few ideas for a gateway:
- NodeMCU ESP8266 with a LCD1602 with an I^2^C-Interface: displays all received data, one 16x2 page consists out of the name of the sender in the first line and the temperature data and humidity data in the second line, loops automatically (or due to a push of a button) through all nodes
- NodeMCU ESP8266 sends data to a MQTT-Server (perhaps a server on a Raspberry Pi), which is visualized with Node-Red
- NodeMCU ESP8266 serves as a webserver, shows received data on a webpage

