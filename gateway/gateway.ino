#include <ESP8266WiFi.h>
#include <espnow.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// pins
const int ESP8266_LED = 2;   // LED of ESP8266
const int NODEMCU_LED = 16;  // LED of NodeMCU

// to keep track of known data_packet IDs
const int MAX_LENGTH_DATA_PACKET_ID_ARR = 100;
long data_packet_id_arr[MAX_LENGTH_DATA_PACKET_ID_ARR];
int data_packet_id_arr_index_counter = 0;

// to emulate parallel working
const unsigned long MINIMUM_DISTANCE_BETWEEN_SCREENS = 3000;  // in milliseconds
unsigned long old_millis = 0;

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

// contents of one lcd screen
struct screen {
  String sender;
  payload_struct payload;
};
const int LENGTH_OF_SCREEN_ARR = 4;  // number of different screens, which equals the number of nodes from which some of the transmitted data will be stored
screen screen_arr[LENGTH_OF_SCREEN_ARR];
int screen_arr_index_counter_free_space_in_arr = 0;
int screen_arr_index_counter = 0;

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
 * Returns the index in the screen_arr where the payload of the given sender is stored.
 *
 * @param sen sender, which payload may be stored in the screen_arr
 * @return index of the payload in screen_arr of given sender, if sender is not in screen_arr -1 will be returned
 */
int get_index_of_screen_in_screen_arr_from_sender(String sen) {
  int result = -1;

  for (int i = 0; i < LENGTH_OF_SCREEN_ARR; i++) {
    if (screen_arr[i].sender == sen) {
      result = i;
      break;
    }
  }

  return result;
}

/**
 * Stores temp, hum and, if the sender is new, the sender of the given data_packet. 
 * No data of the given data_packet will be stored if the screen_arr is full.
 *
 * @param pkt data_packet, which contents may be stored in the screen_arr
 */
void store_data_from_data_packet_to_screen_arr(data_packet pkt) {
  int index_in_screen_arr = get_index_of_screen_in_screen_arr_from_sender(pkt.sender);

  if (index_in_screen_arr != -1) {  // if sender is already known
    screen_arr[index_in_screen_arr].payload.temp = pkt.payload.temp;
    screen_arr[index_in_screen_arr].payload.hum = pkt.payload.hum;

  } else if (screen_arr_index_counter_free_space_in_arr != LENGTH_OF_SCREEN_ARR) {  // if sender is new and the screen_arr is not full
    screen_arr[screen_arr_index_counter_free_space_in_arr].sender = pkt.sender;
    screen_arr[screen_arr_index_counter_free_space_in_arr].payload.temp = pkt.payload.temp;
    screen_arr[screen_arr_index_counter_free_space_in_arr].payload.hum = pkt.payload.hum;

    screen_arr_index_counter_free_space_in_arr++;
  }
}

/**
 * Shows the payload and its initial sender on the i2c lcd screen.
 * The parameter index_of_screen_content must be smaller than LENGTH_OF_SCREEN_ARR.
 *
 * @param index_of_screen_content index in the screen_arr, where the data, which needs to be shown, is stored
 */
void show_lcd_screen_of_a_payload(int index_of_screen_content) {
  screen scr = screen_arr[index_of_screen_content];

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("S: ");
  lcd.print(scr.sender);

  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(scr.payload.temp);
  lcd.print("  ");
  lcd.print("H:");
  lcd.print(scr.payload.hum);
}

/**
 * Shows the next screen on the i2c lcd screen.
 */
void show_next_screen() {
  if (screen_arr_index_counter == LENGTH_OF_SCREEN_ARR) {
    screen_arr_index_counter = 0;
  }

  if (screen_arr_index_counter < screen_arr_index_counter_free_space_in_arr) {
    show_lcd_screen_of_a_payload(screen_arr_index_counter);
    screen_arr_index_counter++;
  } else {  // start from the beginning if all non free spaces have been shown
    screen_arr_index_counter = 0;
    if (screen_arr[0].sender != "") {  //if first entry is not empty
      show_lcd_screen_of_a_payload(screen_arr_index_counter);
      screen_arr_index_counter++;
    }
  }
}

/**
 * Returns feedback about the received data_packet.
 * Can be used as a callback function if data was received.
 *
 * @param addr questionmark
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

    store_data_from_data_packet_to_screen_arr(packet);
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
 * Loops through all available screens.
 */
void loop() {
  unsigned long difference = millis() - old_millis;

  if (difference == MINIMUM_DISTANCE_BETWEEN_SCREENS) {
    show_next_screen();

    old_millis = millis();
  }
}
