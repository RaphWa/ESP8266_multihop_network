#include <ESP8266WiFi.h>
#include <espnow.h>

// pins
const int ESP8266_LED = 2;   // LED des ESP8266
const int NODEMCU_LED = 16;  // LED des NodeMCU

// to keep track of known message IDs
const int MAX_LENGTH_MESSAGE_ID_ARR = 100;
long message_id_arr[MAX_LENGTH_MESSAGE_ID_ARR];
int message_id_arr_index_counter = 0;

// to emulate parallel working
const unsigned long DISTANCE_BETWEEN_TRANSMITTING_DATA_PACKETS = 2000; // in milliseconds
unsigned long old_millis = 0;

// other constants and variables
const int DELAY_LED_BLINKEN = 50;  // in milliseconds
const String MODUL_NAME = "N01";  // the name of this modul, it is a equivalent to an ip address
uint8_t broadcast_address[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

// the sructure of the received and transmitted data packet
struct data_packet {
  unsigned long message_id;
  String addressee;
  float message;
};
data_packet packet;  // the received data_packet, which may be transmitted


/**
 * Stores the given message id.
 * 
 * @param id message id, which needs to be stored
 */
void store_message_id(int id) {
  if (message_id_arr_index_counter == MAX_LENGTH_MESSAGE_ID_ARR) {
    message_id_arr_index_counter = 0;
  }

  message_id_arr[message_id_arr_index_counter] = id;
  message_id_arr_index_counter++;
}

/**
 * Checks, if the given message id is known.
 * 
 * @param id message id, which may be known
 * @return true if given message id is known, otherwise false
 */
bool is_message_id_known(int id) {
  bool result = false;

  for (int i = 0; i < MAX_LENGTH_MESSAGE_ID_ARR; i++) {
    if (message_id_arr[i] == id) {
      result = true;
      break;
    }
  }

  return result;
}

/**
 * Checks if the given address equals MODUL_NAME or if the first and only letter of the given address 
 * equals the first letter of MODUL_NAME.
 *
 * @param addressee_of_pkt addressee of the data_packet
 * @return true if module is addressee of the data_packet, otherwise false 
 */
bool is_this_module_addressee_of_data_packet(String addressee_of_pkt) {
  bool result = false;

  char first_letter_of_module_name = MODUL_NAME.charAt(0);
  char first_letter_of_addressee_of_pkt = addressee_of_pkt.charAt(0);

  if (addressee_of_pkt.equals(MODUL_NAME)) {
    result = true;
  }

  if ((addressee_of_pkt.length() == 1) and (first_letter_of_module_name == first_letter_of_addressee_of_pkt)) {
    result = true;
  }

  return result;
}

/**
 * Checks if all criteria are met to transmit the data_packet.
 *
 * @param pkt data_packet, which needs to be checked
 * @return true if all criteria for the transmission are met, otherwise false
 */
bool is_data_packet_allowed_to_be_transmitted(data_packet pkt) {
  bool result = false;

  bool id_is_know = is_message_id_known(pkt.message_id);
  bool is_modul_addressee = is_this_module_addressee_of_data_packet(pkt.addressee);

  if (not id_is_know and not is_modul_addressee) {
    result = true;
  }

  return result;
}

/**
 * Transmitts the given data_packet to the broadcast_address.
 * 
 * @param pkt data_packet which needs to be transmitted
 */
void transmit_data_packet(data_packet pkt) {
  store_message_id(pkt.message_id);

  // transmit data_packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_send(broadcast_address, (uint8_t*)&pkt, sizeof(pkt));
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
}

/**
 * Creates a new object of the struct data_packet and transmits it to the broadcast_address.
 * Prints out feedback.
 * 
 * @param addr_name the addressee of the new data_packet, should be a MODUL_NAME of a different module
 * @param mes the message of the new data_packet which will be transmitted
 */
void transmit_new_data_packet(String addr_name, float mes) {
  long new_message_id = random(1, 2111222333);

  data_packet pkt = {new_message_id, addr_name, mes};

  Serial.println("-----");

  Serial.println("Trying to transmit new data_packet... ");
  transmit_data_packet(pkt);
  Serial.println("New data_packet transmitted!");

  Serial.print("Transmitted message: ");
  Serial.println(mes);

  digitalWrite(NODEMCU_LED, HIGH);
  delay(DELAY_LED_BLINKEN);
  digitalWrite(NODEMCU_LED, LOW);

  Serial.println("-----");
}

/**
 * Returns feedback whether the transmission was successful or not.
 * Can be used as a callback function if data was transmitted.
 *
 * @param addr addressee of the transmission
 * @param status status of the transmission
 */
void if_data_packet_transmitted(uint8_t* addr, uint8_t status) {
  Serial.print("Status of transmitted data_packet: ");
  if (status == 0) {
    Serial.println("Success");
  } else {
    Serial.println("Failure");
  }
}

/**
 * Returns feedback about the received data_packet.
 * Can be used as a callback function if data was received.
 *
 * @param addr ?
 * @param data received data, should be a data_packet
 * @param received_bytes size of the received data
 */
void if_data_packet_received(uint8_t* addr, uint8_t* data, uint8_t received_bytes) {
  memcpy(&packet, data, sizeof(packet));

  // feedback
  digitalWrite(ESP8266_LED, HIGH);
  Serial.println("-----------------------------");
  Serial.print("Received bytes: ");
  Serial.println(received_bytes);
  Serial.print("The message ID: ");
  Serial.println(packet.message_id);
  Serial.print("Received message: ");
  Serial.println(packet.message);
  delay(DELAY_LED_BLINKEN);
  digitalWrite(ESP8266_LED, LOW);

  if (is_data_packet_allowed_to_be_transmitted(packet)) {
    Serial.println("Trying to transmit... ");
    transmit_data_packet(packet);
    Serial.println("Transmitted!");
    digitalWrite(NODEMCU_LED, HIGH);
    delay(DELAY_LED_BLINKEN);
    digitalWrite(NODEMCU_LED, LOW);
  }
}

/**
 * Sets up the module on which this code is running.
 */
void setup() {
  WiFi.mode(WIFI_STA);

  // needed in order to give feedback
  pinMode(ESP8266_LED, OUTPUT);
  pinMode(NODEMCU_LED, OUTPUT);
  Serial.begin(115200);

  // initialize esp_now
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // set up esp_now
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(if_data_packet_received);
  esp_now_register_send_cb(if_data_packet_transmitted);
  esp_now_add_peer(broadcast_address, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

/**
 * Can transmit a new data_packet every DISTANCE_BETWEEN_TRANSMITTING_DATA_PACKETS
 * milliseconds to the broadcast_address.
 */
void loop() {
  unsigned long difference = millis() - old_millis;

  if (difference == DISTANCE_BETWEEN_TRANSMITTING_DATA_PACKETS) {
    float new_message = random(10.0, 31.0);

    transmit_new_data_packet("G01", new_message);

    old_millis = millis();
  }
}
