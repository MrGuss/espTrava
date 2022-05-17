#include "travaCell.h"

cell::cell(uint8_t waterS, uint8_t dhtPin, uint8_t pumpPin, uint8_t coolerPin, uint8_t lightPin, uint32_t lightTimeUp, uint32_t lightTimeDown, uint32_t waterPeriod, uint8_t desiredHum) {
    this->_dhtPin = dhtPin;
    this->_dht = DHT(this->_dhtPin, DHT11);
    this->_pumpPin = pumpPin;
    this->_coolerPin = coolerPin;
    this->_desiredHum = desiredHum;
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

uint8_t cell::getWater() {
    return digitalRead(this->_waterS);
}

uint8_t cell::getHum() {
    uint8_t h = this->_dht.readHumidity();
    if (isnan(h)) {
        Serial.println("Failed to address DHT11 to get humidity.");
        return 0;
    }
    return h;
}

uint8_t cell::getTemp() {
    uint8_t t = this->_dht.readTemperature();
    if (isnan(t)) {
        Serial.println("Failed to address DHT11 to get temperature.");
        return 0;
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

void cell::timersInit(uint32_t lightUp, uint32_t lightDown, uint32_t waterPeriod) {
    this->_lightTimeUp = lightUp;
    this->_lightTimeDown = lightDown;
    this->_waterPeriod = waterPeriod;
    //this->_lastMilHB = -100000;
    this->_lastMilLight = -100000;
    this->_lastMilPump = -100000;
}

void cell::lightLoop() {
    //DateTime timeLight = this->_RTC.now();
    if (this->_lightStateHard == 0) {
        //if (!this->_lightState && timeLight.unixtime() - this->_lastMilLight >= this->_lightTimeUp) {
        if (!this->_lightState && millis() - this->_lastMilLight >= this->_lightTimeUp) {
            digitalWrite(this->_lightPin, HIGH);
            //this->_lastMilLight = timeLight.unixtime();
            this->_lastMilLight = millis();
            this->_lightState = 1;
        }
        //else if (this->_lightState && timeLight.unixtime() - this->_lastMilLight >= this->_lightTimeDown) {
        else if (this->_lightState && millis() - this->_lastMilLight >= this->_lightTimeDown) {
            digitalWrite(this->_lightPin, LOW);
            //this->_lastMilLight = timeLight.unixtime();
            this->_lastMilLight = millis();
            this->_lightState = 0;
        }
    }
}

void cell::pumpLoop() {
    //DateTime timePump = this->_RTC.now();
    if (this->_pumpStateHard == 0) {
        //if (!this->_pumpState && timePump.unixtime() - this->_lastMilPump >= this->_waterPeriod) {
        if (!this->_pumpState && millis() - this->_lastMilPump >= this->_waterPeriod) {
            //digitalWrite(this->_pumpPin, HIGH);
            analogWrite(this->_pumpPin, 255);
            //this->_lastMilPump = timePump.unixtime();
            this->_lastMilPump = millis();
            this->_pumpState = 1;
        }
        //else if (this->_pumpState && timePump.unixtime() - this->_lastMilPump >= this->_waterFlowTime) {
        else if (this->_pumpState && millis() - this->_lastMilPump >= this->_waterFlowTime) {
            //digitalWrite(this->_pumpPin, LOW);
            analogWrite(this->_pumpPin, 0);
            //this->_lastMilPump = timePump.unixtime();
            this->_lastMilPump = millis();
            this->_pumpState = 0;
        }
    }
}

void cell::coolerLoop() {
    if (this->_coolerStateHard == 0 && this->getHum() && this->_desiredHum)
    {
        if (!this->_coolerState && this->getHum() > 1.1 * this->_desiredHum) {
            analogWrite(this->_coolerPin, 255);
            this->_coolerState = 1;
        }
        else if (this->_coolerState && this->getHum() <= 0.9 * this->_desiredHum) {
            analogWrite(this->_coolerPin, 0);
            this->_coolerState = 0;
        }
    }
    else if (!this->_desiredHum || (this->_coolerStateHard && !this->getHum())) {
        analogWrite(this->_coolerPin, 0);
        this->_coolerState = 0;
    }
}

void cell::lightHardSet(uint8_t state) {
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

void cell::pumpHardSet(uint8_t state) {
    if (state == 1) {
        this->_pumpStateHard = 1;
        digitalWrite(this->_pumpPin, HIGH);
    }
    else if (state == 2) {
        this->_pumpStateHard = 2;
        digitalWrite(this->_pumpPin, LOW);
    }
    else {
        this->_pumpStateHard = 0;
    }
}

void cell::coolerHardSet(uint8_t state) {
    if (state == 1) {
        this->_coolerStateHard = 1;
        digitalWrite(this->_coolerPin, HIGH);
    }
    else if (state == 2) {
        this->_coolerStateHard = 2;
        digitalWrite(this->_coolerPin, LOW);
    }
    else {
        this->_coolerStateHard = 0;
    }
}

void cell::updateLoops() {
    this->lightLoop();
    this->pumpLoop();
    this->coolerLoop();
}

/*uint32_t cell::unixtime()
{
    DateTime dnow = this->_RTC.now();
    uint32_t ret = dnow.unixtime();
    return ret;
}*/
