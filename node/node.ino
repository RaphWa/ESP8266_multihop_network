#include <ESP8266WiFi.h>
#include <espnow.h>

//-----variables-----
// pins
const int ESP8266_LED = 2;   // LED of ESP8266
const int NODEMCU_LED = 16;  // LED of NodeMCU

// to keep track of known data_packet IDs
const int MAX_LENGTH_DATA_PACKET_ID_ARR = 100;
long data_packet_id_arr[MAX_LENGTH_DATA_PACKET_ID_ARR];
int data_packet_id_arr_index_counter = 0;

// to emulate parallel working
const unsigned long MINIMUM_DISTANCE_BETWEEN_TRANSMITTING_DATA_PACKETS = 2000;  // in milliseconds
unsigned long old_millis = 0;

// other constants and variables
const int DELAY_LED_BLINKEN = 50;  // in milliseconds
const String MODUL_NAME = "N01";   // the name of this modul, it is a equivalent to an ip address
uint8_t broadcast_address[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
//-----variables-----

//-----structs-----
/*
 * This structure contains the metadata of a data_packet
 * and represents the head of the data_packet.
 */
struct head_struct {
  unsigned long data_packet_id;
  String sender;
  String addressee;
}

/*
 * This structure contains the payload of a data_packet
 * and represents the body of the data_packet.
 */
struct payload_struct {
  float temp;  // temperature
  float humi;   // humidity
};

/*
 * This structure represents a data packet
 * which can be received and transmitted.
 */
struct data_packet {
  unsigned long data_packet_id;
  String sender;
  String addressee;
  payload_struct payload;
};
data_packet packet;  // the received data_packet, which may be transmitted
//-----structs-----

/**
 * Stores the given data_packet id.
 * 
 * @param id data_packet id, which needs to be stored
 */
void store_data_packet_id(long id) {
  if (data_packet_id_arr_index_counter == MAX_LENGTH_DATA_PACKET_ID_ARR) {
    data_packet_id_arr_index_counter = 0;
  }

  data_packet_id_arr[data_packet_id_arr_index_counter] = id;
  data_packet_id_arr_index_counter++;
}

/**
 * Checks, if the given data_packet id is known.
 * 
 * @param id data_packet id, which may be known
 * @return true if given data_packet id is known, otherwise false
 */
bool is_data_packet_id_known(int id) {
  bool result = false;

  for (int i = 0; i < MAX_LENGTH_DATA_PACKET_ID_ARR; i++) {
    if (data_packet_id_arr[i] == id) {
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

  bool id_is_know = is_data_packet_id_known(pkt.data_packet_id);
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
  store_data_packet_id(pkt.data_packet_id);

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
 * @param pyld the payload of the new data_packet which will be transmitted
 */
void transmit_new_data_packet(String addr_name, payload_struct pyld) {
  long new_data_packet_id = random(1, 2111222333);

  data_packet pkt = { new_data_packet_id, MODUL_NAME, addr_name, pyld };

  Serial.println("-----");

  Serial.println("Trying to transmit new data_packet... ");
  transmit_data_packet(pkt);
  Serial.println("New data_packet transmitted!");

  Serial.println("Transmitted payload: ");
  Serial.print("Temp: ");
  Serial.print(pkt.payload.temp);
  Serial.print("C, humi: ");
  Serial.print(pkt.payload.humi);
  Serial.println("%");

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
  Serial.println("Received payload: ");
  Serial.print("Temp: ");
  Serial.print(packet.payload.temp);
  Serial.print("C, humi: ");
  Serial.print(packet.payload.humi);
  Serial.println("%");
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

//-----setup and loop-----
/**
 * Init
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
 * Can transmit a new data_packet every MINIMUM_DISTANCE_BETWEEN_TRANSMITTING_DATA_PACKETS
 * milliseconds to the broadcast_address.
 */
void loop() {
  unsigned long difference = millis() - old_millis;

  if (difference == MINIMUM_DISTANCE_BETWEEN_TRANSMITTING_DATA_PACKETS) {
    float new_temp = random(10.0, 31.0); // TODO get real data
    float new_humi = random(35.0, 71.0); // TODO get real data
    
    payload_struct new_pyld = { new_temp, new_humi };

    transmit_new_data_packet("G01", new_pyld);

    old_millis = millis();
  }
}
//-----setup and loop-----
