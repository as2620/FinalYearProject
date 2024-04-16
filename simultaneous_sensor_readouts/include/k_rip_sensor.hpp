
#pragma once 

//----------------------------------------------------------------------------------

#include "Arduino.h"
#include "stdio.h"     
#include "driver/pcnt.h"                                                  
#include "soc/pcnt_struct.h"

//----------------------------------------------------------------------------------

// Pulse Counters in the ESP32 
// This piece of code makes use of the Pulse Counter Peripheral in the ESP32. The 
// Pulse Counters count the number edges of the waveform that pass during a set interval 
// of time. This changes as the frequency of the waveform changes. 

// How Do Pulse Counters Work? 
// Pulse Counters in the ESP32 require the programmer to select a unit. 
// A unit has two channels, one that can increment the counter and the other that can decrement 
// the counter on a falling or rising edge.
// Pulse Counters can be configured to do something on either or both edges. 
// The Control Signal is used to control the counting mode of the edge signals that are 
// attached to the same channel. 


//----------------------------------------------------------------------------------

class KRipSensor
{
    public: 
        KRipSensor(pcnt_unit_t pulse_counter_unit, pcnt_channel_t pulse_counter_channel, uint8_t pulse_counter_signal, uint8_t pulse_counter_control);
        void intialise();
        void start_pulse_counter();
        void read();

        uint32_t mult_pulses = 0;
        uint32_t frequency = 0;

    private: 

        pcnt_unit_t pulse_counter_unit;
        pcnt_channel_t pulse_counter_channel;
        uint16_t pulse_counter_h_lim = 32000;

        uint8_t pulse_counter_signal;
        uint8_t pulse_counter_control;

        uint32_t overflow = 32000;
};

//----------------------------------------------------------------------------------