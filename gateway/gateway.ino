#include <ESP8266WiFi.h>
#include <espnow.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

// pins
const int ESP8266_LED = 2;   // LED of ESP8266
const int NODEMCU_LED = 16;  // LED of NodeMCU

// to keep track of known data_packet IDs
const int MAX_LENGTH_DATA_PACKET_ID_ARR = 100;
long data_packet_id_arr[MAX_LENGTH_DATA_PACKET_ID_ARR];
int data_packet_id_arr_index_counter = 0;

// other constants and variables
const int DELAY_LED_BLINKEN = 50;  // in milliseconds
const String MODUL_NAME = "G01";   // the name of this modul, it is a equivalent to an ip address
uint8_t broadcast_address[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

// the structure of the payload of a data_packet
struct payload_struct {
  float temp;  // temperature
  float hum;   // humidity
};

// the sructure of the received and transmitted data_packet
struct data_packet {
  unsigned long data_packet_id;
  String sender;
  String addressee;
  payload_struct payload;
};
data_packet packet;  // the received data_packet, which may be transmitted


/**
 * Stores the given data_packet id.
 * 
 * @param id data_packet id, which needs to be stored
 */
void store_data_packet_id(int id) {
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
bool is_data_packet_id_known(long id) {
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
  delay(DELAY_LED_BLINKEN);
  digitalWrite(ESP8266_LED, LOW);

  if (is_this_module_addressee_of_data_packet(packet.addressee) and not is_data_packet_id_known(packet.data_packet_id)) {
    store_data_packet_id(packet.data_packet_id);

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("S: ");
    lcd.print(packet.sender);

    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.print(packet.payload.temp);
    lcd.print("  ");
    lcd.print("H:");
    lcd.print(packet.payload.hum);
  }
}

/**
 * Sets up the module on which this code is running.
 */
void setup() {
  lcd.init();
  lcd.clear();         
  lcd.backlight();

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
  esp_now_add_peer(broadcast_address, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  lcd.setCursor(0, 0);
  lcd.print("Set up finished.");
  lcd.setCursor(0, 1);
  lcd.print("Waiting.........");
}

/**
 * Empty method.
 */
void loop() {

}
