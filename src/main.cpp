/**
 * mac id : CC:7B:5C:28:B4:6C
 * esp-now with esp 32
 * mode: reaceive data from mutiple boards (many-to-one)
 * note slave read sensor co, sensor sound
 * master
 * authors:oopsan
 */

#include <esp_now.h>
#include <WiFi.h>
#include <WiFiType.h>
#include "WiFi.h"
#include "FS.h"
#include <chrono>
#include <thread>

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message
{
  int id; // must be unique for each sender board
  int MQ7SensorValue;
  int MAX9814SensorValue;
  int sendSS1;
} struct_message;

// Create a struct_message called dataMaster
struct_message dataMaster;

// Create a structure to hold the readings from each board
struct_message board1;
struct_message board2;
struct_message board3;

// Create an array with all the structures
struct_message boardsStruct[3] = {board1, board2, board3};

const int LED_RED = 25;
const int LED_GREEN = 26;
int Received = 0;
auto lastUpdateTime = std::chrono::steady_clock::now();
// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len)
{

  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&dataMaster, incomingData, sizeof(dataMaster));
  Serial.printf("Board ID %u: %u bytes\n", dataMaster.id, len);
  // Update the structures with the new incoming data
  boardsStruct[dataMaster.id - 1].MQ7SensorValue = dataMaster.MQ7SensorValue;
  boardsStruct[dataMaster.id - 1].MAX9814SensorValue = dataMaster.MAX9814SensorValue;
  Serial.printf("MQ7SensorValue value: %d \n", boardsStruct[dataMaster.id - 1].MQ7SensorValue);
  Serial.printf("MAX9814SensorValue value: %d \n", boardsStruct[dataMaster.id - 1].MAX9814SensorValue);
  Serial.println();
  Received++;
}
void checkReceivedAndUpdateLED()
{
  while (true)
  {
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastUpdateTime).count();

    // Nếu đã qua 5 giây và giá trị Received không thay đổi
    if (elapsedTime >= 5)
    {
      // Bật tắt LED ở đây
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_GREEN, HIGH);
      delay(500); // Giữ LED trong 0.5 giây
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_GREEN, LOW);

      // Reset đồng hồ
      lastUpdateTime = std::chrono::steady_clock::now();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Đợi 100ms trước khi kiểm tra lại
  }
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
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_RED,LOW);
  digitalWrite(LED_GREEN,LOW);

  std::thread ledThread(checkReceivedAndUpdateLED);
  ledThread.detach(); // Cho phép thread chạy độc lập
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

  // Giả sử Received được cập nhật ở đây
  int newReceivedValue = 1; // Giá trị mới của Received
  if (newReceivedValue != Received)
  {
    Received = newReceivedValue;
    lastUpdateTime = std::chrono::steady_clock::now(); // Cập nhật thời gian
  }
  Serial.println(Received);
  int board1MAX9814SensorValue = boardsStruct[0].MAX9814SensorValue;
  int board1MQ7SensorValue = boardsStruct[0].MQ7SensorValue;
  // int board1SendSS= boardsStruct[0].sendSS;
  Serial.print("MQ7SensorValue: ");
  Serial.println(board1MQ7SensorValue);
  Serial.print("MAX9814SensorValue:");
  Serial.println(board1MAX9814SensorValue);

  if (board1MQ7SensorValue > 1500 || board1MAX9814SensorValue > 2000)
  {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, HIGH);
  }
  else
  {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
  }

  delay(2000);
}
