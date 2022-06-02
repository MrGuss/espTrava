
// Светодиод подлкючен к 5 пину
// Датчик температуры ds18b20 к 2 пину

//#define ONE_WIRE_BUS 2
//OneWire oneWire(ONE_WIRE_BUS);
//DallasTemperature sensors(&oneWire);
#include <ArduinoJson.h>
#include <travaCell.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

StaticJsonDocument<128> doc;


const char *ssid = "RaspiNetwork";          // Имя вайфай точки доступа
const char *pass = "raspberry";             // Пароль от точки доступа

const char *mqtt_server = "192.168.4.1";    // Имя сервера MQTT
const int mqtt_port = 1883;                 // Порт для подключения к серверу MQTT
const char *mqtt_user = "";                 // Логи от сервер
const char *mqtt_pass = "";                 // Пароль от сервера

uint32_t _hbDelay = 5000;
uint32_t _lastMilHB = -100000;                   // from cell.TimersInit

#define BUFFER_SIZE 100

WiFiClient wclient;
PubSubClient client(wclient, mqtt_server, mqtt_port);

//RTC clock;
uint8_t dhtPin = 13;        // D7
uint8_t pumpPin = 0;       // D5
uint8_t coolerPin = 3;     // D6
uint8_t bottleHumPin = 12;  // D8
uint8_t lampPin = 1;        // D2
uint8_t lightPin = A0;
uint8_t pieza = 2;
//14 = D5, 12 = D6 - change
// pumpPin = motorASpeedPin (D5)
cell cell1 = cell(pieza, bottleHumPin, dhtPin, pumpPin, coolerPin, lampPin, lightPin, 5000000, 2000000, 300000000, 1000, 50);

void sendHeartbeat(bool hard) {
  if (hard) {
    String json = "{\"ID\": \"" + String(WiFi.macAddress()) + "\", \"Humidity\": " + String(cell1.getHum()) + ", \"Temperature\": " + String(cell1.getTemp()) + ", \"Light\": " + String(cell1.getLight()) + ", \"WaterS\": " + String(int(cell1.getWater())) + "}";
    client.publish("test/heartbeat", json);
  }
  else {
    if ((millis() - _lastMilHB) >= _hbDelay) {
      //Serial.println(String(cell1.unixtime()));
      String json = "{\"ID\": \"" + String(WiFi.macAddress()) + "\", \"Humidity\": " + String(cell1.getHum()) + ", \"Temperature\": " + String(cell1.getTemp()) + ", \"Light\": " + String(cell1.getLight()) + ", \"WaterS\": " + String(int(cell1.getWater())) + "}";
      Serial.println(json);
      client.publish("test/heartbeat", json);

      _lastMilHB = millis();
    }
  }
}

void callback(const MQTT::Publish& pub) {                    // Функция получения данных от сервера:
  Serial.print(pub.topic());                               //   выводим в сериал порт название топика
  Serial.print(" => ");

  Serial.println(String(pub.payload_string().toInt()));    //   выводим в сериал порт значение полученных данных
  char* payload = new char[pub.payload_string().length() + 1];
  //char* payload = pub.payload_string().c_str();
  strcpy(payload, pub.payload_string().c_str());

  if (String(pub.topic()) == "test/light") {

    DeserializationError error = deserializeJson(doc, payload);
    const char* buf = doc["MAC"]; // "98:7a:7f:45:d4:r3:8d"
    String MAC = buf;
    byte state = doc["state"]; // 1351824120
    Serial.print("light ");
    Serial.println(state);
    if (MAC == String(WiFi.macAddress())) {
      cell1.lightHardSet(state);
      Serial.println(state);
    }
    //delete [] buf;
  }

  if (String(pub.topic()) == "test/pump") {

    DeserializationError error = deserializeJson(doc, payload);
    const char* buf = doc["MAC"]; // "98:7a:7f:45:d4:r3:8d"
    String MAC = buf;
    byte state = doc["state"]; // 1351824120
    Serial.print("pump ");
    Serial.println(state);
    if (MAC == String(WiFi.macAddress())) {
      cell1.pumpHardSet(state);
      Serial.println(state);
    }
    //delete [] buf;
  }

  if (String(pub.topic()) == "test/cooler") {

    DeserializationError error = deserializeJson(doc, payload);
    const char* buf = doc["MAC"]; // "98:7a:7f:45:d4:r3:8d"
    String MAC = buf;
    byte state = doc["state"]; // 1351824120
    Serial.print("cooler ");
    Serial.println(state);
    if (MAC == String(WiFi.macAddress())) {
      cell1.coolerHardSet(state);
      Serial.println(state);
    }
    //delete [] buf;
  }

  if (String(pub.topic()) == "test/mode") {

    DeserializationError error = deserializeJson(doc, payload);
    const char* ID = doc["ID"];
    String MAC = ID;
    long IWater = doc["IWater"]; // 1000000
    long TWater = doc["TWater"]; // 1000000
    long ILight = doc["ILight"]; // 100000
    long TLight = doc["TLight"]; // 100000
    int Temperature = doc["Temperature"]; // 14
    int Humidity = doc["Humidity"]; // 60
    if (MAC == String(WiFi.macAddress())) {

      cell1.timersInit(ILight, TLight, IWater, TWater, Humidity);
      //Serial.print("Mode: ");
      //String output;
      //serializeJson(doc, output);
    }
    //delete [] buf;
  }

  delete[] payload;
  /*
    if (String(pub.topic()) == "test/heartbeat") {
      cell1.sendHeartbeat(true);
    }
  */
}


void setup() {
  //sensors.begin();
  //cell1.TimersInit();
  Serial.begin(115200);
  //clock.begin();

  delay(10);
  Serial.println();
  Serial.println();
  //pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // подключаемся к wi-fi
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.begin(ssid, pass);

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
      return;
    Serial.println("WiFi connected");
  }

  // подключаемся к MQTT серверу
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      Serial.println("Connecting to MQTT server");
      if (client.connect(MQTT::Connect("arduinoClient2")
                         .set_auth(mqtt_user, mqtt_pass))) {
        Serial.println("Connected to MQTT server");
        client.set_callback(callback);
        client.subscribe("test/#");    // подписывааемся по топик с данными для светодиода
      }
      else {
        Serial.println("Could not connect to MQTT server");
      }
    }

    if (client.connected()) {
      client.loop();
    }
    cell1.updateLoops();
    delay(20);
    sendHeartbeat(false);
  }

  /*String timeStr;
    String dateStr;
    String weekDayStr;
    clock.read();
    clock.getTimeStamp(timeStr, dateStr, weekDayStr);*/
  //Serial.print(
}
