
#pragma once 

//----------------------------------------------------------------------------------

#include "Arduino.h"
#include "stdio.h"     

//----------------------------------------------------------------------------------

class GsrSensor
{
    public: 
        GsrSensor(uint8_t gsr_pin);
        void read();
        void calculateAverageValue();

        uint32_t averaged_gsr_value = 0;
    
    private:
        uint8_t gsr_pin = 0;
        uint32_t sensorValue = 0;
        uint32_t sum = 0;
        uint32_t read_count = 0;
};

//----------------------------------------------------------------------------------