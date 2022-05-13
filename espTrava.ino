

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

int _hbDelay = 5000;
int _lastMilHB = -100000;                   // from cell.TimersInit

#define BUFFER_SIZE 100

WiFiClient wclient;
PubSubClient client(wclient, mqtt_server, mqtt_port);

//RTC clock;

uint8_t dhtPin = 14; //D5, 12 = D6
cell cell1 = cell(14, dhtPin, 12, LED_BUILTIN, 5, 2, 2);

void sendHeartbeat(bool hard) {
    if (hard) {
        String json = "{\"ID\": \"" + String(WiFi.macAddress()) + "\", \"Humidity\": " + String(cell1.getHum()) + ", \"Temperature\": " + String(cell1.getTemp()) + "}";
        client.publish("test/heartbeat", json);
    }
    else {
        if ((millis() - _lastMilHB) >= _hbDelay) {
            Serial.println(String(cell1.unixtime()));
            String json = "{\"ID\": \"" + String(WiFi.macAddress()) + "\", \"Humidity\": " + String(cell1.getHum()) + ", \"Temperature\": " + String(cell1.getTemp()) + ", \"Light\": " + String(int(cell1.lightState())) + "}";
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

    if (String(pub.topic()) == "test/modeChange") {

        DeserializationError error = deserializeJson(doc, payload);
        const char* buf = doc["MAC"]; // "98:7a:7f:45:d4:r3:8d"
        String MAC = buf;

        if (MAC == String(WiFi.macAddress())) {
            int lightUp = doc["lightUp"]; // 1351824120
            int lightDown = doc["lightDown"]; // 1351824120
            int waterPer = doc["waterPer"];
            cell1.TimersInit(lightUp, lightDown, 1);
            Serial.println(lightUp);
            Serial.println(lightDown);
            Serial.println(waterPer);
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
    pinMode(LED_BUILTIN, OUTPUT);
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
        sendHeartbeat(false);
    }

    /*String timeStr;
      String dateStr;
      String weekDayStr;
      clock.read();
      clock.getTimeStamp(timeStr, dateStr, weekDayStr);*/
      //Serial.print(
}
