//-------------------------------------------------------------------------------------------------

// Header File for PPG Sensor Class

//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------

#include <Arduino.h>
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

//-------------------------------------------------------------------------------------------------

class PpgSensor
{
    public: 
        PpgSensor(byte ledBrightness, byte sampleAverage, byte ledMode, byte sampleRate, int pulseWidth, 
                  int adcRange);
        bool intialise();
        void read();

        uint32_t ir_value;
        uint32_t red_value;

    private:
        MAX30105 particleSensor;

        byte ledBrightness;         // Options: 0=Off to 255=50mA
        byte sampleAverage;         // Options: 1, 2, 4, 8, 16, 32
        byte ledMode;               // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
        byte sampleRate;            // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
        int pulseWidth;             // Options: 69, 118, 215, 411
        int adcRange;               // Options: 2048, 4096, 8192, 16384 

};

//-------------------------------------------------------------------------------------------------
