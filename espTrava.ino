// Светодиод подлкючен к 5 пину
// Датчик температуры ds18b20 к 2 пину

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
        cell(word waterS, word pump, int sleep, int work){
            _waterS = waterS;
            _pump = pump;
            _sleep = sleep;
            _work = work;
            pinMode(_waterS, INPUT);
            pinMode(_pump, OUTPUT);
        }

        byte getWater(){
            return digitalRead(_waterS);
        }


        void checkWater(){
            Serial.println(getWater());
            if (getWater()==0){
                digitalWrite(_pump, HIGH);
            }
            else{
                digitalWrite(_pump, LOW);
            }
        }

        void setTimers(int sleep, int work){
            _sleep = sleep;
            _work = work;
        }
        void sendHeartbeat(){

            String json = "{\n\"Humidity\": " + String(getHum) + ",\n\"Temerature\": " + String(getTemp) + "\n}";
            client.publish("test/heartbeat", json);
        }
    private:
        byte _waterS;
        byte _pump;
        int _sleep;
        int _work;
};

cell cell1 = cell(14, 12, 1000, 1000);
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

  //sensors.begin();
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  cell1.checkWater();
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

