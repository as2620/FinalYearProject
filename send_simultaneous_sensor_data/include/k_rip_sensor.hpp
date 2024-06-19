//-------------------------------------------------------------------------------------------------

// Header File for K-RIP Sensor Class

//-------------------------------------------------------------------------------------------------

#pragma once 

//-------------------------------------------------------------------------------------------------

#include "Arduino.h"
#include "stdio.h"     
#include "driver/pcnt.h"                                                  
#include "soc/pcnt_struct.h"

//-------------------------------------------------------------------------------------------------

class KRipSensor
{
    public: 
        KRipSensor(pcnt_unit_t pulse_counter_unit, pcnt_channel_t pulse_counter_channel, 
                   uint8_t pulse_counter_signal, uint8_t pulse_counter_control);
        void intialise();
        void read();

        uint32_t mult_pulses                        = 0;
        uint32_t frequency                          = 0;

    private: 

        pcnt_unit_t pulse_counter_unit;
        pcnt_channel_t pulse_counter_channel;
        uint16_t pulse_counter_h_lim                = 32000;

        uint8_t pulse_counter_signal;
        uint8_t pulse_counter_control;

        uint32_t overflow                           = 32000;
};

//-------------------------------------------------------------------------------------------------
