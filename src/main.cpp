/**
 * mac id : E8:68:E7:09:1A:CC
 * esp-now with esp 32
 * mode: reaceive data from mutiple boards (many-to-one)
 * note slave read sensor co, sensor sound
 * master
 * authors:oopsan
 */

#include <esp_now.h>
#include <WiFi.h>

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message
{
  int id; // must be unique for each sender board
  int MQ7SensorValue;
  int MAX9814SensorValue;
} struct_message;

// Create a struct_message called note1Slave
struct_message note1Slave;

// Create a structure to hold the readings from each board
struct_message board1;
struct_message board2;
struct_message board3;

// Create an array with all the structures
struct_message boardsStruct[3] = {board1, board2, board3};

const int LED_RED = 20;
const int LED_GREEN = 22;
// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len)
{
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&note1Slave, incomingData, sizeof(note1Slave));
  Serial.printf("Board ID %u: %u bytes\n", note1Slave.id, len);
  // Update the structures with the new incoming data
  boardsStruct[note1Slave.id - 1].MQ7SensorValue = note1Slave.MQ7SensorValue;
  boardsStruct[note1Slave.id - 1].MAX9814SensorValue = note1Slave.MAX9814SensorValue;
  Serial.printf("MQ7SensorValue value: %d \n", boardsStruct[note1Slave.id - 1].MQ7SensorValue);
  Serial.printf("MAX9814SensorValue value: %d \n", boardsStruct[note1Slave.id - 1].MAX9814SensorValue);
  Serial.println();
}

void setup()
{
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  pinMode(LED_RED, INPUT);
  pinMode(LED_GREEN, INPUT);
}

void loop()
{
  // Acess the variables for each board
  /*int board1X = boardsStruct[0].x;
  int board1Y = boardsStruct[0].y;
  int board2X = boardsStruct[1].x;
  int board2Y = boardsStruct[1].y;
  int board3X = boardsStruct[2].x;
  int board3Y = boardsStruct[2].y;*/

  int board1MAX9814SensorValue = boardsStruct[0].MAX9814SensorValue;
  int board1MQ7SensorValue = boardsStruct[0].MQ7SensorValue;

  if (board1MQ7SensorValue > 300)
  {
    digitalWrite(LED_RED, 1);
    digitalWrite(LED_GREEN,0);
  }
  else
  {
    digitalWrite(LED_RED,0);
    digitalWrite(LED_GREEN,1);
  }

  delay(5000);
}
