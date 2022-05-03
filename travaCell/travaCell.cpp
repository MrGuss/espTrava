#include "travaCell.h"

cell::cell(word waterS, uint8_t dhtPin, byte pump, byte lightPin, int lightTimeUp, int lightTimeDown, int waterPeriod) {
  this->_waterS = waterS;
  this->_dhtPin = dhtPin;
  this->_pump = pump;
  this->_dht = DHT(this->_dhtPin, DHT11);
  this->_lightPin = lightPin;
  this->_lightTimeUp = lightTimeUp;
  this->_lightTimeDown = lightTimeDown;
  this->_waterPeriod = waterPeriod;
  pinMode(this->_waterS, INPUT);
  pinMode(this->_dhtPin, INPUT);
  pinMode(this->_pump, OUTPUT);
  pinMode(this->_lightPin, OUTPUT);
  this->_dht.begin();
}

bool cell::lightState(){
	return this->_lightState;
}

byte cell::getWater() {
  return digitalRead(this->_waterS);
}

int cell::getHum() {
  int h = this->_dht.readHumidity();
  if(isnan(h)) {
    Serial.println("Failed to address DHT11 to get humidity.");
    return -1;
  }
  return h;
}

int cell::getTemp() {
  int t = this->_dht.readTemperature();
  if(isnan(t)) {
    Serial.println("Failed to address DHT11 to get temperature.");
    return -1;
  }
  return t;
}

void cell::checkDHT() {    // Not used in project; prints data from DHT into Serial.
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

void cell::checkWater() {
  Serial.println(this->getWater());
  if (this->getWater() == 0) {
    digitalWrite(this->_pump, HIGH);
  }
  else {
    digitalWrite(this->_pump, LOW);
  }
}

void cell::TimersInit(int lightUp, int lightDown, int waterPeriod) {
  this->_lightTimeUp = lightUp;
  this->_lightTimeDown = lightDown;
  this->_waterPeriod = waterPeriod;
    //this->_lastMilHB = -100000;
  this->_lastMilLight = -100000;

}

void cell::lightLoop() {
  if (_lightStateHard==0){
    if (!(this->_lightState) && ((millis() - this->_lastMilLight) >= this->_lightTimeUp)) {
      digitalWrite(this->_lightPin, HIGH);
      this->_lastMilLight = millis();
      this->_lightState = 1;
    }
    else if (this->_lightState && ((millis() - this->_lastMilLight) >= this->_lightTimeDown)) {
      digitalWrite(this->_lightPin, LOW);
      this->_lastMilLight = millis();
      this->_lightState = 0;
    }
  }
}

void cell::lightHardSet(byte state) {
  if (state==1) {
    this->_lightStateHard = 1;
    digitalWrite(this->_lightPin, HIGH);
  }
  else if (state==2) {
    this->_lightStateHard = 2;
    digitalWrite(this->_lightPin, LOW);
  }
  else{
    _lightStateHard = 0;
  }
}

void cell::updateLoops() {
  this->lightLoop();
}
