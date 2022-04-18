// Светодиод подлкючен к 5 пину
// Датчик температуры ds18b20 к 2 пину

#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
//#include <OneWire.h>
//#include <DallasTemperature.h>

//#define ONE_WIRE_BUS 2
//OneWire oneWire(ONE_WIRE_BUS);
//DallasTemperature sensors(&oneWire);

const char *ssid = "RaspiNetwork"; // Имя вайфай точки доступа
const char *pass = "raspberry"; // Пароль от точки доступа

const char *mqtt_server = "192.168.4.1"; // Имя сервера MQTT
const int mqtt_port = 1883; // Порт для подключения к серверу MQTT
const char *mqtt_user = ""; // Логи от сервер
const char *mqtt_pass = ""; // Пароль от сервера

#define BUFFER_SIZE 100

bool LedState = false;
int tm = 300;
float temp = 0;


class cell {

    public:      
      cell(int id, word waterS, uint8_t dhtPin, word pump, int sleep, int work, int hbDelay) {
        this->_id = id;
        this->_waterS = waterS;
        this->_dhtPin = dhtPin;
        this->_pump = pump;
        this->_sleep = sleep;
        this->_work = work;
        this->_hbDelay = hbDelay;
        this->_dht = DHT(this->_dhtPin, DHT11);
        pinMode(this->_waterS, INPUT);
        pinMode(this->_dhtPin, INPUT);
        pinMode(this->_pump, OUTPUT);
        this->_dht.begin();
      }

      byte getWater(){
        return digitalRead(this->_waterS);
      }
      float getHum()
      {
        float h = this->_dht.readHumidity();
        if(isnan(h))
        {
          Serial.println("Failed to address DHT11 to get humidity.");
          return -1.0;
        }
        return h;
      }

      float getTemp()
      {
        float t = this->_dht.readTemperature();
        if(isnan(t))
        {
          Serial.println("Failed to address DHT11 to get temperature.");
          return -1.0;
        }
        return t;
      }

      void checkDHT()
      {
        float h, t;
        h = this->getHum();
        t = this->getTemp();
        if(h != -1.0)
        {
          Serial.print("Humidity:    ");
          Serial.println(h);
        }
        if(t != -1.0)
        {
          Serial.print("Temperature: ");
          Serial.println(t);
        }
      }
      
      void checkWater(){
        Serial.println(this->getWater());
        if (this->getWater() == 0){
          digitalWrite(this->_pump, HIGH);
        }
        else{
          digitalWrite(this->_pump, LOW);
        }
      }

      void TimersInit(){
        //this->_sleep = sleep;
        //this->_work = work;
        _lastMilHB = -100000;
      }
        
        void sendHeartbeat(){
            if ((millis()-_lastMilHB)>=_hbDelay){
                String json = "{\n\"ID\": " + String(_id) + ",\n\"Humidity\": " + String(getHum) + ",\n\"Temerature\": " + String(getTemp) + "\n}";
                client.publish("test/heartbeat", json);
                _lastMilHB = millis();
            }
        }
    private:
        int _id;
        int _lastMilHB;
      byte _waterS;
      byte _pump;
      uint8_t _dhtPin;
      DHT _dht = DHT(0, DHT11);
      int _sleep;
      int _work;
        int _hbDelay;
};

uint8_t dhtPin = 14; //D5
cell cell1 = cell(14, dhtPin, 12, 1000, 1000);

// Функция получения данных от сервера

void callback(const MQTT::Publish& pub)
{
  Serial.print(pub.topic()); // выводим в сериал порт название топика
  Serial.print(" => ");

  Serial.println(String(pub.payload_string().toInt())); // выводим в сериал порт значение полученных данных
  String payload = pub.payload_string();

  if (String(pub.topic()) == "test/blink") // проверяем из нужного ли нам топика пришли данные
  {
    int stled = payload.toInt(); // преобразуем полученные данные в тип integer
    digitalWrite(LED_BUILTIN,  abs(stled-1)); // включаем или выключаем светодиод в зависимоти от полученных значений данных
  }

    if (String(pub.topic()) == "test/heartbeat"){
        cell1.sendHeartbeat();
    }
}

WiFiClient wclient;
PubSubClient client(wclient, mqtt_server, mqtt_port);

void setup() {
    cell1.TimersInit();
  //sensors.begin();
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // подключаемся к wi-fi
  if (WiFi.status() != WL_CONNECTED) {
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
        client.subscribe("test/blink"); // подписывааемся по топик с данными для светодиода
      } else {
        Serial.println("Could not connect to MQTT server");
      }
    }

    if (client.connected()) {
        client.loop();
        cell1.sendHeartbeat();

    }

  }
} // конец основного цикла

