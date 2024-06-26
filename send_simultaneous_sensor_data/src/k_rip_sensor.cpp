//-------------------------------------------------------------------------------------------------

// Source File for K-RIP Sensor Class

//-------------------------------------------------------------------------------------------------

#include "../include/k_rip_sensor.hpp"

//-------------------------------------------------------------------------------------------------

KRipSensor::KRipSensor(pcnt_unit_t pulse_counter_unit, pcnt_channel_t pulse_counter_channel, 
                       uint8_t pulse_counter_signal, uint8_t pulse_counter_control)
{
    this->pulse_counter_unit = pulse_counter_unit;
    this->pulse_counter_channel = pulse_counter_channel;
    this->pulse_counter_signal = pulse_counter_signal;
    this->pulse_counter_control = pulse_counter_control;
}

//-------------------------------------------------------------------------------------------------

void KRipSensor::intialise()
{
    // PCNT unit instance
    pcnt_config_t pcnt_config = {};                                        

    // Pulse input GPIO - Freq Meter Input
    pcnt_config.pulse_gpio_num = this->pulse_counter_signal; 
 
    //  Pulse Counter, Counting Unit                      
    pcnt_config.unit = this->pulse_counter_unit;   

    // Pulse Counter, Channel - 0                                             
    pcnt_config.channel = this->pulse_counter_channel;       

    // Maximum counter value                        
    pcnt_config.counter_h_lim = this->pulse_counter_h_lim;  

    // PCNT positive edge count mode - inc
    pcnt_config.pos_mode = PCNT_COUNT_INC;                                  

    // Configure Pulse Counter control mode
    pcnt_config.lctrl_mode = PCNT_MODE_DISABLE;                             
    pcnt_config.hctrl_mode = PCNT_MODE_KEEP;    

    // Initialize PCNT unit
    pcnt_unit_config(&pcnt_config);                                         

    // Pause PCNT unit
    pcnt_counter_pause(this->pulse_counter_unit);
    
    // Clear PCNT unit                                               
    pcnt_counter_clear(this->pulse_counter_unit);                                               

    // Start pulse counter
    pcnt_counter_resume(this->pulse_counter_unit);                                                
}

//-------------------------------------------------------------------------------------------------

void KRipSensor::read()
{
    pcnt_counter_pause(this->pulse_counter_unit);

    int16_t pulses = 0;    

    // Read Pulse Counter value                                                  
    pcnt_get_counter_value(this->pulse_counter_unit, &pulses);                                                                                                                          

    frequency = pulses;

    // Clear Pulse Counter
    pcnt_counter_clear(this->pulse_counter_unit);   

    // Start pulse counter
    pcnt_counter_resume(this->pulse_counter_unit);         
}

//-------------------------------------------------------------------------------------------------
