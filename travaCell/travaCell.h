#ifndef TRAVA_CELL_H
#define TRAVA_CELL_H

#include "DHT.h"
//#include <OneWire.h>
//#include <DallasTemperature.h>

//#define ONE_WIRE_BUS 2

class cell {
    public:      
        cell(word waterS, uint8_t dhtPin, byte pump, byte lightPin, int lightTimeUp, int lightTimeDown);
        byte getWater();
		bool lightState();
        int getHum();
        int getTemp();
        void checkDHT();     // Not used in project; prints data from DHT into Serial.
        void checkWater();   // ?
        //void TimersInit();
        void sendHeartbeat(bool now);
        void lightLoop();
        void updateLoops();
    private:
        byte _lightPin;
        int _lastMilLight;
        int _lightTimeUp;
        int _lightTimeDown;
        int _waterPeriod;
        bool _lightState;
        byte _lightStateHard = 0;
        byte _pump;
        byte _waterS; 
        uint8_t _dhtPin;
        DHT _dht = DHT(0, DHT11);
};

#endif
