#include "travaCell.h"

cell::cell(word waterS, uint8_t dhtPin, byte pumpPin, byte lightPin, int lightTimeUp, int lightTimeDown, int waterPeriod) {
    this->_dhtPin = dhtPin;
    this->_dht = DHT(this->_dhtPin, DHT11);
    this->_pumpPin = pumpPin;
    this->_waterPeriod = waterPeriod;
    this->_waterS = waterS;
    this->_lightPin = lightPin;
    this->_lightTimeUp = lightTimeUp;
    this->_lightTimeDown = lightTimeDown;
    pinMode(this->_waterS, INPUT);
    pinMode(this->_dhtPin, INPUT);
    pinMode(this->_pumpPin, OUTPUT);
    pinMode(this->_lightPin, OUTPUT);
    this->_dht.begin();
    Wire.begin();  // Start the I2C
    this->_RTC.begin();  // Init RTC
    this->_RTC.adjust(DateTime(__DATE__, __TIME__));
}

bool cell::lightState() {
    return this->_lightState;
}

byte cell::getWater() {
    return digitalRead(this->_waterS);
}

int cell::getHum() {
    int h = this->_dht.readHumidity();
    if (isnan(h)) {
        Serial.println("Failed to address DHT11 to get humidity.");
        return -1;
    }
    return h;
}

int cell::getTemp() {
    int t = this->_dht.readTemperature();
    if (isnan(t)) {
        Serial.println("Failed to address DHT11 to get temperature.");
        return -1;
    }
    return t;
}

void cell::checkDHT() {    // Not used in project; prints data from DHT into Serial.
    float h, t;
    h = this->getHum();
    t = this->getTemp();
    if (h != -1.0) {
        Serial.print("Humidity:    ");
        Serial.println(h);
    }
    if (t != -1.0) {
        Serial.print("Temperature: ");
        Serial.println(t);
    }
}

void cell::checkWater() {
    Serial.println(this->getWater());
    if (this->getWater() == 0) {
        digitalWrite(this->_pumpPin, HIGH);
    }
    else {
        digitalWrite(this->_pumpPin, LOW);
    }
}

void cell::TimersInit(int lightUp, int lightDown, int waterPeriod) {
    this->_lightTimeUp = lightUp;
    this->_lightTimeDown = lightDown;
    this->_waterPeriod = waterPeriod;
    //this->_lastMilHB = -100000;
    this->_lastMilLight = -100000;
    this->_lastMilPump = -100000;
}

void cell::lightLoop() {
    DateTime timeLight = this->_RTC.now();
    if (_lightStateHard == 0) {
        if (!this->_lightState && timeLight.unixtime() - this->_lastMilLight >= this->_lightTimeUp) {
            digitalWrite(this->_lightPin, HIGH);
            this->_lastMilLight = timeLight.unixtime();
            this->_lightState = 1;
        }
        else if (this->_lightState && timeLight.unixtime() - this->_lastMilLight >= this->_lightTimeDown) {
            digitalWrite(this->_lightPin, LOW);
            this->_lastMilLight = timeLight.unixtime();
            this->_lightState = 0;
        }
    }
}

void cell::pumpLoop() {
    DateTime timePump = this->_RTC.now();
    if (_pumpStateHard == 0) {
        if (!this->_pumpState && timePump.unixtime() - this->_lastMilPump >= this->_waterPeriod) {
            digitalWrite(this->_pumpPin, HIGH);
            this->_lastMilPump = timePump.unixtime();
            this->_pumpState = 1;
        }
        else if (this->_pumpState && timePump.unixtime() - this->_lastMilPump >= this->_waterFlowTime) {
            digitalWrite(this->_pumpPin, LOW);
            this->_lastMilPump = timePump.unixtime();
            this->_pumpState = 0;
        }
    }
}

void cell::lightHardSet(byte state) {
    if (state == 1) {
        this->_lightStateHard = 1;
        digitalWrite(this->_lightPin, HIGH);
    }
    else if (state == 2) {
        this->_lightStateHard = 2;
        digitalWrite(this->_lightPin, LOW);
    }
    else {
        this->_lightStateHard = 0;
    }
}

void cell::updateLoops() {
    this->lightLoop();
    this->pumpLoop();
}

uint32_t cell::unixtime()
{
    DateTime dnow = this->_RTC.now();
    uint32_t ret = dnow.unixtime();
    return ret;
}
