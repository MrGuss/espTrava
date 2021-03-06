#ifndef TRAVA_CELL_H
#define TRAVA_CELL_H

#define LEN 10000

#include "DHT.h"
#include <Wire.h> // Library for I2C communication
#include <SPI.h>  // not used here, but needed to prevent a RTClib compile error
#include "RTClib.h"
#include <Adafruit_PWMServoDriver.h>


//#include <OneWire.h>
//#include <DallasTemperature.h>

//#define ONE_WIRE_BUS 2

class cell {
public:
    cell(uint8_t piezaPin, uint8_t waterS, uint8_t dhtPin, uint8_t pumpPin, uint8_t coolerPin, uint8_t lightPin, uint8_t lightS, uint32_t lightTimeUp, uint32_t lightTimeDown, uint32_t waterPeriod, uint32_t waterFlow, uint8_t desiredHum);
    uint8_t getWater();
    bool lightState();
    uint8_t getHum();
    uint8_t getTemp();
    uint8_t getLight();
    void checkDHT();     // Not used in project; prints data from DHT into Serial.
    void timersInit(long lightUp, long lightDown, long waterPeriod, long waterL, int hum);
    void pumpLoop();
    void lightLoop();
    void coolerLoop();
    void piezaLoop();
    void lightHardSet(uint8_t state);
    void pumpHardSet(uint8_t state);
    void coolerHardSet(uint8_t state);
    void updateLoops();
    //uint32_t unixtime(); // temp
private:
    //RTC_DS1307 _RTC;     // Setup an instance of DS1307 naming it RTC
    uint8_t _piezaPin;
    uint8_t _lightPin;
    uint32_t _lastMilLight;
    uint32_t _lightTimeUp;
    uint32_t _lightTimeDown;
    uint32_t _waterPeriod;
    uint32_t _waterFlowTime;
    bool _lightState;
   // uint8_t _sqr[LEN];
    uint8_t _piezaState = 0;
    uint8_t _pumpStateHard = 0;
    uint8_t _lightStateHard = 0;
    uint8_t _pumpPin;
    uint32_t _lastMilPump;
    bool _pumpState = 0;
    uint8_t _coolerPin;
    uint8_t _coolerStateHard = 0;
    bool _coolerState = 0;
    uint8_t _waterS;       // whether there is still water for sprinkling ( reads boolean humidity sensor )
    uint8_t _lightS;
    uint8_t _dhtPin;
    uint8_t _desiredHum;
    DHT _dht = DHT(0, DHT11);
    Adafruit_PWMServoDriver _pwm = Adafruit_PWMServoDriver(0x40);
};

#endif
