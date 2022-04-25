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

const char *ssid = "RaspiNetwork";          // Имя вайфай точки доступа
const char *pass = "raspberry";             // Пароль от точки доступа

const char *mqtt_server = "192.168.4.1";    // Имя сервера MQTT
const int mqtt_port = 1883;                 // Порт для подключения к серверу MQTT
const char *mqtt_user = "";                 // Логи от сервер
const char *mqtt_pass = "";                 // Пароль от сервера

#define BUFFER_SIZE 100

WiFiClient wclient;
PubSubClient client(wclient, mqtt_server, mqtt_port);

bool LedState = false;
int tm = 300;
float temp = 0;


class cell {
    public:      
        cell(word waterS, uint8_t dhtPin, byte pump, byte lightPin, int lightTimeUp, int lightTimeDown,  int hbDelay) {
            this->_waterS = waterS;
            this->_dhtPin = dhtPin;
            this->_pump = pump;
            this->_hbDelay = hbDelay;
            this->_dht = DHT(this->_dhtPin, DHT11);
            this->_lightPin = lightPin;
            this->_lightTimeUp = lightTimeUp;
            this->_lightTimeDown = lightTimeDown;
            pinMode(this->_waterS, INPUT);
            pinMode(this->_dhtPin, INPUT);
            pinMode(this->_pump, OUTPUT);
            pinMode(this->_lightPin, OUTPUT);
            this->_dht.begin();
        }

        byte getWater(){
            return digitalRead(this->_waterS);
        }
      
        int getHum() {
            int h = this->_dht.readHumidity();
            if(isnan(h)) {
                Serial.println("Failed to address DHT11 to get humidity.");
                return -1;
            }
            return h;
        }

        int getTemp() {
            int t = this->_dht.readTemperature();
            if(isnan(t)) {
              Serial.println("Failed to address DHT11 to get temperature.");
              return -1;
            }
            return t;
        }

        void checkDHT()    // Not used in project; prints data from DHT into Serial.
        {
            float h, t;
            h = this->getHum();
            t = this->getTemp();
            if(h != -1.0) {
              Serial.print("Humidity:    ");
              Serial.println(h);
            }
            if(t != -1.0) {
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
        this->_lastMilHB = -100000;
        this->_lastMilLight = -100000;
      }
        
        void sendHeartbeat(bool now){
            if (now){
                String json = "{\"ID\": \"" + String(WiFi.macAddress()) + "\", \"Humidity\": " + String(this->getHum()) + ", \"Temperature\": " + String(this->getTemp()) + "}";
                client.publish("test/heartbeat", json);
            }
            else{
                if ((millis() - this->_lastMilHB) >= this->_hbDelay){
                    String json = "{\"ID\": \"" + String(WiFi.macAddress()) + "\", \"Humidity\": " + String(this->getHum()) + ", \"Temperature\": " + String(this->getTemp()) +", \"Light\": " + String(int(_lightState)) + "}";
                    client.publish("test/heartbeat", json);
                    this->_lastMilHB = millis();
                }
            }
        }
        
        void lightLoop(){
            if (!this->_lightState && (millis() - this->_lastMilLight) >= this->_lightTimeDown){
                digitalWrite(this->_lightPin, HIGH);
                this->_lastMilLight = millis();
                this->_lightState = 1;
            }
            else if (this->_lightState && (millis() - this->_lastMilLight) >= this->_lightTimeDown){
                digitalWrite(this->_lightPin, LOW);
                this->_lastMilLight = millis();
                this->_lightState = 0;
            }
        }

        void updateLoops(){
            this->lightLoop();
            this->sendHeartbeat(false);
        }
    private:
        int _lastMilHB;

        byte _lightPin;
        int _lastMilLight;
        int _lightTimeUp;
        int _lightTimeDown;
        bool _lightState;
        
        byte _pump;
       
        byte _waterS; 
        uint8_t _dhtPin;
        DHT _dht = DHT(0, DHT11);
        
        int _hbDelay;
};


uint8_t dhtPin = 14; //D5
cell cell1 = cell(14, dhtPin, 12, LED_BUILTIN, 5000, 2000, 5000);


void callback(const MQTT::Publish& pub)                      // Функция получения данных от сервера:
{
    Serial.print(pub.topic());                               //   выводим в сериал порт название топика
    Serial.print(" => ");

    Serial.println(String(pub.payload_string().toInt()));    //   выводим в сериал порт значение полученных данных
    String payload = pub.payload_string();

    if (String(pub.topic()) == "test/blink")                 //   проверяем из нужного ли нам топика пришли данные
    {
        int stled = payload.toInt();                         //   преобразуем полученные данные в тип integer
        digitalWrite(LED_BUILTIN,  abs(stled-1));            //   включаем или выключаем светодиод в зависимоти от полученных значений данных
    }

    if (String(pub.topic()) == "test/heartbeat"){
        cell1.sendHeartbeat(true);
    }
}


void setup() {
    //sensors.begin();
    cell1.TimersInit();
    Serial.begin(115200);
    delay(10);
    Serial.println();
    Serial.println();
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() 
{
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
                client.subscribe("test/blink");    // подписывааемся по топик с данными для светодиода
            } 
            else {
                Serial.println("Could not connect to MQTT server");
            }
        }

        if (client.connected()) {
            client.loop();
        }
        cell1.updateLoops();
    }
} 
