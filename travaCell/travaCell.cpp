/*
We have two kits of our system:
with only pump and water sensor - ONLY_PUMP
and with everything - EVERYTHING
*/
#include "travaCell.h"




cell::cell(uint8_t piezaPin, uint8_t waterS, uint8_t dhtPin, uint8_t pumpPin, uint8_t coolerPin, uint8_t lightPin, uint8_t lightS, uint32_t lightTimeUp, uint32_t lightTimeDown, uint32_t waterPeriod, uint32_t waterFlow, uint8_t desiredHum) {
  this->_piezaPin = piezaPin;
  this->_dhtPin = dhtPin;
  this->_dht = DHT(this->_dhtPin, DHT11);
  this->_pumpPin = pumpPin;
  this->_coolerPin = coolerPin;
  this->_desiredHum = desiredHum;
  this->_waterPeriod = waterPeriod;
  this->_waterFlowTime = waterFlow;
  this->_waterS = waterS;
  this->_lightS = lightS;
  this->_lightPin = lightPin;
  this->_lightTimeUp = lightTimeUp;
  this->_lightTimeDown = lightTimeDown;
  pinMode(this->_waterS, INPUT);
  pinMode(this->_dhtPin, INPUT);
  pinMode(this->_pumpPin, OUTPUT);
  pinMode(this->_lightPin, OUTPUT);
  this->_dht.begin();
  this->_pwm.begin();
  this->_pwm.setPWMFreq(1600);
  digitalWrite(this->_coolerPin, LOW);
  //Wire.begin();  // Походу это нужно
  //this->_RTC.begin();  // Init RTC
  //this->_RTC.adjust(DateTime(__DATE__, __TIME__));
}

bool cell::lightState() {
  return this->_lightState;
}

uint8_t cell::getWater() {
  if (digitalRead(this->_waterS) > 0) {
    return 0;
  }
  else {
    return 1;
  }
}

uint8_t cell::getLight() {
  //Serial.println(analogRead(this->_lightS));
  if (analogRead(this->_lightS) > 700) {
    return 0;
  }
  else if (analogRead(this->_lightS) < 400) {
    return 1;
  }
  else {
    return _lightState;
  }
}

uint8_t cell::getHum() {
  uint8_t h = this->_dht.readHumidity();

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

void cell::timersInit(long lightUp, long lightDown, long waterPeriod, long waterL, int hum) {
  this->_lightTimeUp = lightUp;
  this->_lightTimeDown = lightDown;
  this->_waterPeriod = waterPeriod;
  this->_desiredHum = hum;
  //this->_lastMilHB = -100000;
  this->_lastMilLight = -100000;
  this->_lastMilPump = -100000;
}

void cell::lightLoop() {
  //DateTime timeLight = this->_RTC.now();
  if (this->_lightStateHard == 0) {
    if (!this->_lightState && this->getLight() == 1) {
      this->_pwm.setPWM(this->_lightPin, 0, 4095);
      //analogWrite(this->_coolerPin, 0);
      this->_lightState = 1;
    }
    else if (this->_lightState && this->getLight() == 0) {
      this->_pwm.setPWM(this->_lightPin, 0, 0);
      this->_lightState = 0;
    }
    /*
    //if (!this->_lightState && timeLight.unixtime() - this->_lastMilLight >= this->_lightTimeUp) {
    if (!this->_lightState && millis() - this->_lastMilLight >= this->_lightTimeUp) {
    this->_pwm.setPWM(this->_lightPin, 0, 4095);
    //Serial.println("Light is on");
    //this->_lastMilLight = timeLight.unixtime();
    this->_lastMilLight = millis();
    this->_lightState = 1;
    }
    //else if (this->_lightState && timeLight.unixtime() - this->_lastMilLight >= this->_lightTimeDown) {
    else if (this->_lightState && millis() - this->_lastMilLight >= this->_lightTimeDown) {
    this->_pwm.setPWM(this->_lightPin, 0, 0);
    //this->_lastMilLight = timeLight.unixtime();
    this->_lastMilLight = millis();
    this->_lightState = 0;
    }
    */
  }

}

void cell::pumpLoop() {
  //DateTime timePump = this->_RTC.now();
  //while
  if (this->_pumpStateHard == 0) {
    //if (!this->_pumpState && timePump.unixtime() - this->_lastMilPump >= this->_waterPeriod) {
    if (!this->_pumpState && millis() - this->_lastMilPump >= this->_waterPeriod) {
      //digitalWrite(this->_pumpPin, HIGH);
      this->_pwm.setPWM(this->_pumpPin, 0, 4095);
      //this->_lastMilPump = timePump.unixtime();
      Serial.println(_pumpStateHard);
      this->_lastMilPump = millis();
      this->_pumpState = 1;
    }
    //else if (this->_pumpState && timePump.unixtime() - this->_lastMilPump >= this->_waterFlowTime) {
    else if (this->_pumpState && millis() - this->_lastMilPump >= this->_waterFlowTime) {
      //digitalWrite(this->_pumpPin, LOW);
      this->_pwm.setPWM(this->_pumpPin, 0, 0);
      //this->_lastMilPump = timePump.unixtime();
      this->_lastMilPump = millis();
      this->_pumpState = 0;
    }
  }
}

void cell::coolerLoop() {
  if (this->_coolerStateHard == 0)
  {
    if ((this->getHum() > (this->_desiredHum + 10))) {
      this->_pwm.setPWM(this->_coolerPin, 0, 4095);
      //analogWrite(this->_coolerPin, 0);
      this->_coolerState = 1;
    }
    else if ((this->getHum() <= (this->_desiredHum + 10))) {
      this->_pwm.setPWM(this->_coolerPin, 0, 0);
      this->_coolerState = 0;
    }
  }
  /*
  else if (!this->_desiredHum || (this->_coolerStateHard && !this->getHum())) {
    this->_pwm.setPWM(this->_coolerPin, 0, 0);
    this->_coolerState = 0;
  }
  */
}

void cell::piezaLoop() {
  //if (this->_StateHard == 0)
  //{
  if ((this->getHum() < (this->_desiredHum - 10)) & (_piezaState==0)) {
    this->_pwm.setPWM(this->_piezaPin, 0, 0);
    delay(1000);
    this->_pwm.setPWM(this->_piezaPin, 0, 4095);
    Serial.println("PiezaOn");
    //analogWrite(this->_coolerPin, 0);
    this->_piezaState = 1;
  }
  else if ((this->getHum() >= (this->_desiredHum - 10)) & (_piezaState==1)){
    this->_pwm.setPWM(this->_piezaPin, 0, 0);
    delay(1000);
    this->_pwm.setPWM(this->_piezaPin, 0, 4095);
    Serial.println("PiezaOff");
    this->_piezaState = 0;
  }
  //}
  /*
  else if (!this->_desiredHum || (this->_coolerStateHard && !this->getHum())) {
    this->_pwm.setPWM(this->_coolerPin, 0, 0);
    this->_coolerState = 0;
  }
  */
}

void cell::lightHardSet(uint8_t state) {
  if (state == 1) {
    this->_lightStateHard = 1;
    this->_pwm.setPWM(this->_lightPin, 0, 0);
  }
  else if (state == 2) {
    this->_lightStateHard = 2;
    this->_pwm.setPWM(this->_lightPin, 0, 4095);
  }
  else {
    this->_lightStateHard = 0;
  }
}

void cell::pumpHardSet(uint8_t state) {
  if (state == 1) {
    this->_pumpStateHard = 1;
    this->_pwm.setPWM(this->_pumpPin, 0, 4095);
  }
  else if (state == 2) {
    this->_pumpStateHard = 2;
    this->_pwm.setPWM(this->_pumpPin, 0, 0);
  }
  else {
    //digitalWrite(this->_pumpPin, LOW);
    this->_pumpStateHard = 0;
  }
}

void cell::coolerHardSet(uint8_t state) {
  if (state == 1) {
    this->_coolerStateHard = 1;
    this->_pwm.setPWM(this->_coolerPin, 0, 4095);
  }
  else if (state == 2) {
    this->_coolerStateHard = 2;
    this->_pwm.setPWM(this->_coolerPin, 0, 0);
    (this->_coolerPin, LOW);
  }
  else {
    //this->_pwm.setPWM(this->_coolerPin, 0, 0)
    Serial.println("State 0 achiewed");
    this->_coolerStateHard = 0;
  }
}

void cell::updateLoops() {
  //this->_sqr[LEN - 1] = this->_sqr[0];
  //this->_sqr[0] = digitalRead(this->_lightS);
  this->lightLoop();
  this->pumpLoop();
  this->coolerLoop();
  this->piezaLoop();
}

/*uint32_t cell::unixtime()
{
    DateTime dnow = this->_RTC.now();
    uint32_t ret = dnow.unixtime();
    return ret;
}*/
