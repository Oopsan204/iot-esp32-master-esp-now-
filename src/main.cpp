/**
 * mac id : CC:7B:5C:27:88:24
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
#include "ESPAsyncWebServer.h"
#include <Arduino_JSON.h>

const char *ssid = "AML 2.4";
const char *password = "aml305b4";

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message
{
  int id; // must be unique for each sender board
  int MQ7SensorValue;
  int MAX9814SensorValue;
  unsigned int readingId;
} struct_message;

// Create a struct_message called dataMaster
struct_message dataMaster;

// Create a structure to hold the readings from each board
struct_message board1;
struct_message board2;
struct_message board3;

// Create an array with all the structures
struct_message boardsStruct[3] = {board1, board2, board3};

JSONVar board;
AsyncWebServer server(80);
AsyncEventSource events("/events");

const int LED_RED = 5;
const int LED_GREEN = 22;
const int LED_Yellow = 19;
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

  board["id"] = dataMaster.id;
  board["MQ7SensorValue"] = dataMaster.MQ7SensorValue;
  board["MAX9814SensorValue"] = dataMaster.MAX9814SensorValue;
  board["readingId"] = String(dataMaster.readingId);
  String jsonString = JSON.stringify(board);
  events.send(jsonString.c_str(), "new_readings", millis());

  Serial.printf("Board ID %u: %u bytes\n", dataMaster.id, len);
  Serial.printf("co value: %4.2f \n", dataMaster.MQ7SensorValue);
  Serial.printf("db value: %4.2f \n", dataMaster.MAX9814SensorValue);
  Serial.printf("readingID value: %d \n", dataMaster.readingId);
  Serial.println();

  Received++;
}
const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
<html>
<head>
  <title>AML-PHENIKAA</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p {  font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #2f4468; color: white; font-size: 1.7rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);transition: transform 0.3s, box-shadow 0.3s; }
    .cards { max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); }
    .reading { font-size: 2.8rem; }
    .packet { color: #bebebe; }
    .card.co { color: #e76f51; }
    .card.sound { color: #2a9d8f; }
    .reading {
      animation: pulse 2s infinite;
    }
    @keyframes pulse {
      0% {
        transform: scale(1);
        box-shadow: 0 0 0 0 rgba(0, 0, 0, 0.7);
      }
      70% {
        transform: scale(1.05);
        box-shadow: 0 0 10px 10px rgba(0, 0, 0, 0);
      }
      100% {
        transform: scale(1);
        box-shadow: 0 0 0 0 rgba(0, 0, 0, 0);
      }
    }
  </style>
</head>
<body>
  <div class="topnav">
    <h3>ESP-NOW AML</h3>
  </div>
   <div class="content">
    <div class="cards">
      <div class="card co">
        <h4><i class="fas fa-wind"></i> BOARD #1 - CO LEVEL</h4><p><span class="reading"><span id="co1"></span> ppm</span></p><p class="packet">Reading ID: <span id="rc1"></span></p>
      </div>
      <div class="card sound">
        <h4><i class="fas fa-volume-up"></i> BOARD #1 - SOUND LEVEL</h4><p><span class="reading"><span id="db1"></span> dB</span></p><p class="packet">Reading ID: <span id="rd1"></span></p>
      </div>
      <div class="card co">
        <h4><i class="fas fa-wind"></i> BOARD #2 - CO LEVEL</h4><p><span class="reading"><span id="co2"></span> ppm</span></p><p class="packet">Reading ID: <span id="rc2"></span></p>
      </div>
      <div class="card sound">
        <h4><i class="fas fa-volume-up"></i> BOARD #2 - SOUND LEVEL</h4><p><span class="reading"><span id="db2"></span> dB</span></p><p class="packet">Reading ID: <span id="rd2"></span></p>
      </div>
    </div>
  </div>
  <script>
    if (!!window.EventSource) {
  var source = new EventSource('/events');
  
  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);
  
  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);
  
  source.addEventListener('message', function(e) {
    console.log("message", e.data);
  }, false);
  
  source.addEventListener('new_readings', function(e) {
    console.log("new_readings", e.data);
    var obj = JSON.parse(e.data);
    document.getElementById("co"+obj.id).innerHTML = obj.MQ7SensorValue.toFixed(2);
    document.getElementById("db"+obj.id).innerHTML = obj.MAX9814SensorValue.toFixed(2);
    document.getElementById("rc"+obj.id).innerHTML = obj.readingId;
    document.getElementById("rd"+obj.id).innerHTML = obj.readingId; // Changed from readingID to readingId for consistency
  }, false);
}
  </script>
</body>
</html>
)rawliteral";

void checkReceivedAndUpdateLED()
{
  while (true)
  {
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastUpdateTime).count();

    // Nếu đã qua 3 giây và giá trị Received không thay đổi
    if (elapsedTime >= 3)
    {
      // Bật tắt LED ở đây
      digitalWrite(LED_Yellow, HIGH);
      delay(500);
      // digitalWrite(LED_Yellow, LOW);

      // Reset đồng hồ
      lastUpdateTime = std::chrono::steady_clock::now();
    }
    else
    {
      digitalWrite(LED_Yellow, LOW);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Đợi 100ms trước khi kiểm tra lại
  }
}

void setup()
{
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_AP_STA);
  // Set device as a Wi-Fi Station
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());


  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html); });

  events.onConnect([](AsyncEventSourceClient *client)
                   {
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000); });
  server.addHandler(&events);
  server.begin();

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_Yellow, OUTPUT);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);

  std::thread ledThread(checkReceivedAndUpdateLED);
  ledThread.detach(); // Cho phép thread chạy độc lập
}

void loop()
{

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


  static unsigned long lastEventTime = millis();
  static const unsigned long EVENT_INTERVAL_MS = 2000;
  if ((millis() - lastEventTime) > EVENT_INTERVAL_MS)
  {
    events.send("ping", NULL, millis());
    lastEventTime = millis();
  }

  if (board1MQ7SensorValue > 2500)
  {
    digitalWrite(LED_RED, HIGH);
  }
  else
  {
    digitalWrite(LED_RED, LOW);
  }
  if (board1MAX9814SensorValue > 1500)
  {
    digitalWrite(LED_GREEN, HIGH);
  }
  else
  {
    digitalWrite(LED_GREEN, LOW);
  }

  delay(2000);
}