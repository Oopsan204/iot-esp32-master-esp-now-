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
#include "SPIFFS.h"

const char *ssid = "Lalala";
const char *password = "son56789";

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
  <div class = "logo">
  <img
  src ="data:image/jpeg;base64,/9j/4AAQSkZJRgABAQAAAQABAAD/7QCEUGhvdG9zaG9wIDMuMAA4QklNBAQAAAAAAGgcAig
  AYkZCTUQwYTAwMGE2ZjAxMDAwMDhkMDQwMDAwNjEwODAwMDA0MDA5MDAwMDRjMGEwMDAwM2QxMDAwMDAzMTE1MDAwMDkyMTYwMDAw
  YTIxNzAwMDBhMzE4MDAwMDAyMjAwMDAwAP/bAEMABgQFBgUEBgYFBgcHBggKEAoKCQkKFA4PDBAXFBgYFxQWFhodJR8aGyMcFhYgLC
  AjJicpKikZHy0wLSgwJSgpKP/bAEMBBwcHCggKEwoKEygaFhooKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKC
  goKCgoKCgoKP/CABEIATUBNgMBIgACEQEDEQH/xAAcAAEAAgMBAQEAAAAAAAAAAAAABQYDBAcIAQL/xAAYAQEAAwEAAAAAAAAAAAAAAAAA
  AQIDBP/aAAwDAQACEAMQAAAB6oAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
  AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA1Y4mofDNmpv1+VNtqDbamIkGPIfniHTOJdGF2655u9BRbfGGvKYf
  svnXq57L2Lzz1uJu5g5t+awUBi7uSydeq145t+P6XafOeudju9Z65S30YbAAAVqehJw0d6A2yU0tTGb7TG5jxzZhzPwcrj6/2Hq5+J9Op2K9e9Pn3
  i6sPnL0b5y6MN2xbNO1p6Mp9r5PzbVPWuNO6+frl2pN24un55u9I+bts7x1rkvWqXDLQABX7BBGxp2GBJrFX+ZHcMnK+nmdV9om/3AfsnKrauRXpTus0
  Ke6Mo6pXqi3p3Kwcp6tydGHzl6N85a59m416D4VLqvJt/JavQeQ9u4inrl2pN25tfnm70j5v2zu/WuS9apcjIXO9tESAi5QV58nj868T+yX0NXbN7a1q
  0W78wmoSXC7/AKHThYLxybqeV8HCukQOlKj3PnW8XTgl9xzF7o/4vVL8N6THXbSkfyG8fCW6FyjoGWmfgN+waUjujUobWLFZYXUc+4AFazzv4IKwaMYSUd
  jspX93eyg+GH5F4MOiY2oDPNZn5XdpExqaY39iryyZL7EasxYNfLDTEpsRWKJmtfR1omfQX6mJtA/sm/kRqRNkQe7au+NMwAH5/Q+fQAfPohJPYUvBYbGpe
  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
  5IPtfDXSXQjJ4mbx1O7ghbiH0F+eHyXLg8aOrPi9H8BZBd2t+HesfaonDANDNyn0BAKOxWdIrqsGW3/JdtgCPwf4nw/4lsBc43+OFsN7958VGxEFK2vBYxMah30cLE/Bnp7c
  PI+0YAFLG37Iym8HbbxFtqs73nwspNfovL3qeD7PD5LlweNHVnxejgTGEVerLVSra1zNmvp9CDyznmSI5I2ja8O3JnQd8GeV2RFaaYJc7BvvAK5T334jSwXzY/rEZeypqQ1+
  ses7hJds9IRIKyJvG6qV0xrtLoEnQU5mlOO/nNqDsf2SyUxqunKbRIBs6t1yZWX4Y9PbgWMGEeWv9ITn8iaOXAABTQKIBAmirIOc4M/tHWURpXUWQo+LxMuDrhQGNZ4X6i5p
  MAnxKRgnVNi8UcSDNi8eQY+5zbdJOhOYmB9Lbo4TepZZ2O0Nrosz1xwhKCQFDahrTBCSaom7F5HXjCgBsCStR6QrIfuHtwgF5f3vwgwj9N6+OADmGi1vfQAhAFaKl0ZbhH42
  21LfL6YUCFAGUg3ed7n1VANGmrbzW/dnejZM2rsdxKAOGd8DgRLzZbFAgaAcj1nrdS3LicXucqiFI8rIk5G0US+NzsJXHJIrWVwvgtawGy7EdfMsuF1HGhcxWeF8L4AurMfU
  AQCOoygF8hUYNf3KDWMp6ku+CqKLcFgK1GXFhFULkY2eWx1gB1+WTUoEQqyWXcach35yp8VssY3hWIYEhV2cqLhlLRXdy7rXYemUEsQquY+hRWVXzmZXchKxhpiKxM7c3aVl
  ojrBorhvpE6ycYItvnBv6TGAw+0BMDrLSxrlMjhCOF5x8XLausSnCMrAyh4k9AuAUo+tVynpAGgH0sRXVuMYlTeuo0xlYGL0QNCrxB3jLeDUU9bl5+EU5uUfm+Dn2gLVb2CL
  My7V0mUyYMW/iMRg0FKt5fgNua87RmqV1CEyDL1KIFVMNaYZP7cNe0BEqexLKMBwVAlbnVfwlxaUQKpZPkAMdWUYk09QhN1rFToqETUwVQTABX3EINAwf8M9szAs12/7/f/a
  AAwDAQACAAMAAAAQ88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
  88888888888488408888888888IQww09osFCsBUhd88884QQ804dp8/WRD/X8888o8swocucp5rOyI/T8688sYEgY4g55izgVGQAgn8888Ios8EZbXUkO1g0l+I888s8888+NN
  P9c/c/tcl888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
  888888888888888/8QAJREBAAIBAgYCAwEAAAAAAAAAAQARITFBEDBRYXGBoeFwkcGx/9oACAEDAQE/EPwmwgRwjwuXesZooxSRlSzdwJ3TIHlm5YuBt4WdJQE0Zs5desAWIbWJ
  TM2zSikENAmjNnCSRa8kJVYmChUL0EGCoh2REglwEQuESiBNJRoIQDkpBS78FVrKTsjW+fuZqs3+NZcLKgcOykx447+ILmLxXe9IL9i/kP7KgVtbfmoMouSt8ZlN2mL+PUSwpd1
  7mIgK89a6QhVMV855FLXT+yxkM0H60/yIm5fXfWLRNwHoiyINeelffmYYXjrtLLix1IVXGKr1EKVrR+oqkalffm8zQRvcJRMJW/W9ZWez8Hf/xAAlEQEAAgIBAwMFAQAAAAAAAA
  ABABEhMUEQMFFhcaFwgZHR8LH/2gAIAQIBAT8Q+gt94UQgoGulenSAQm4EEmklRGjpGq9s6xuFYdQbL5lWeJ7cei2jb2q4ipPUWRupYHSzRuPFbXz0W0bYIjo7IN7MN8q4HbJnm
  Se4Ig2h3LsblQQU5ywi9mG0h899luKxX3ucPm54xCu68fOpkpi1JlEtPEdK1+ooDn9XBpBzWI6p6/E9D+/M5y9XM2gt/wB5goAefjsBFfdfEry8r+YUyf5xCqPlcCFCzuIfceOZR
  M0k4Z5v7w0N6v5lIDhub0eKgURyNyyefod//8QALBABAAIBAwMDBAICAwEAAAAAAQARITFBUWFxgRCRoSCx0fDB8TDhQFCAkP/aAAgBAQABPxD/AOBVkG/ouXL+m5fpcuX/AI2FmS
  oBqrCEAKWw9mpSKNqr5BM+P8BSBqNANWZMKryoBXFQqtuCZhcZKhWemA9oMCKwFrGRm9/R1p5+9KuGLGyhZ4bPHoqIHgasVtWjuHj0dLgq0DPtN10qBYVoqs7QQZICjKOUqs7+htOs2
  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"
  alt ="logo AML"
   <div class="content">
    <div class="cards">
      <div class="card co">
        <h4><i class="fas fa-wind"></i> BOARD #1 - CO LEVEL</h4><p><span class="reading"><span id="co1"></span> </span></p><p class="packet">Reading ID: <span id="rc1"></span></p>
      </div>
      <div class="card sound">
        <h4><i class="fas fa-volume-up"></i> BOARD #1 - SOUND LEVEL</h4><p><span class="reading"><span id="db1"></span> </span></p><p class="packet">Reading ID: <span id="rd1"></span></p>
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
  digitalWrite(LED_Yellow, LOW);

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

  if (board1MQ7SensorValue > 1000)
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