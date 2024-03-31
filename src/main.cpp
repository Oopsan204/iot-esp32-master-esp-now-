#include <esp_now.h>
#include <WiFi.h>
#include <WiFiType.h>
#include "WiFi.h"

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message
{
  int rightButtonAvalue; // SW3 default =1
  int rightButtonBvalue; // Sw4
  int rightButtonCvalue; // SW2
  int rightButtonDvalue; // SW1

  int leftJoyXvalue;
  int leftJoyYvalue;
  int leftJoySWvalue;
} struct_message;

int joystickvaleX;
int joystickvaleY;
int rightbuttonA;
int rightbuttonB;
int rightbuttonC;
int rightbuttonD;
int leftJoySWvalue;

// Create a struct_message called myData
struct_message readingData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  memcpy(&readingData, incomingData, sizeof(readingData));
  Serial.print("Bytes received: ");
  Serial.println(len);

  joystickvaleX = readingData.leftJoyXvalue;
  joystickvaleY = readingData.leftJoyYvalue;
  rightbuttonA = readingData.rightButtonAvalue;
  rightbuttonB = readingData.rightButtonBvalue;
  rightbuttonC = readingData.rightButtonCvalue;
  rightbuttonD = readingData.rightButtonDvalue;
  leftJoySWvalue = readingData.leftJoySWvalue;
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
}

void loop()
{
}