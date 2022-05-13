#ifndef TRAVA_CELL_H
#define TRAVA_CELL_H

#include "DHT.h"
#include <Wire.h> // Library for I2C communication
#include <SPI.h>  // not used here, but needed to prevent a RTClib compile error
#include "RTClib.h"

//#include <OneWire.h>
//#include <DallasTemperature.h>

//#define ONE_WIRE_BUS 2

class cell {
public:
    cell(word waterS, uint8_t dhtPin, byte pump, byte lightPin, int lightTimeUp, int lightTimeDown, int waterPeriod);
    byte getWater();
    bool lightState();
    int getHum();
    int getTemp();
    void checkDHT();     // Not used in project; prints data from DHT into Serial.
    void checkWater();   // ?
    void TimersInit(int lightUp, int lightDown, int waterPeriod);
    void pumpLoop();
    void lightLoop();
    void lightHardSet(byte state);
    void updateLoops();
    uint32_t unixtime(); // temp
private:
    RTC_DS1307 _RTC;     // Setup an instance of DS1307 naming it RTC
    byte _lightPin;
    uint32_t _lastMilLight;
    int _lightTimeUp;
    int _lightTimeDown;
    int _waterPeriod;
    int _waterFlowTime = 10;
    bool _lightState;
    int _pumpStateHard = 0;
    byte _lightStateHard = 0;
    byte _pumpPin;
    uint32_t _lastMilPump;
    bool _pumpState = 0;
    byte _waterS;       // whether there is still water for sprinkling ( reads boolean humidity sensor )
    uint8_t _dhtPin;
    DHT _dht = DHT(0, DHT11);
};

#endif
