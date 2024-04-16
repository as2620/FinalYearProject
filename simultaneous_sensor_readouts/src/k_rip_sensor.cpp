
#include "../include/k_rip_sensor.hpp"

//----------------------------------------------------------------------------------

KRipSensor::KRipSensor(pcnt_unit_t pulse_counter_unit, pcnt_channel_t pulse_counter_channel, uint8_t pulse_counter_signal, uint8_t pulse_counter_control)
{
    this->pulse_counter_unit = pulse_counter_unit;
    this->pulse_counter_channel = pulse_counter_channel;
    this->pulse_counter_signal = pulse_counter_signal;
    this->pulse_counter_control = pulse_counter_control;
}

//----------------------------------------------------------------------------------

void KRipSensor::intialise()
{
    // PCNT unit instance
    pcnt_config_t pcnt_config = {};                                        

    // Pulse input GPIO - Freq Meter Input
    pcnt_config.pulse_gpio_num = this->pulse_counter_signal; 

    // Control signal input GPIO                        
    // pcnt_config.ctrl_gpio_num = this->pulse_counter_control;   

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

    // Enable event to watch - max count
    pcnt_event_enable(this->pulse_counter_unit, PCNT_EVT_H_LIM);     

    // Enable interrupts for Pulse Counter unit
    pcnt_intr_enable(this->pulse_counter_unit);                                                 
}

//----------------------------------------------------------------------------------

void KRipSensor::start_pulse_counter()
{
    pcnt_counter_resume(this->pulse_counter_unit);
}

//----------------------------------------------------------------------------------

void KRipSensor::read()
{
    pcnt_counter_pause(this->pulse_counter_unit);

    const int64_t esp_time_now = esp_timer_get_time();

    int16_t pulses = 0;    
    uint32_t mult = mult_pulses;

    // Clear overflow counter
    mult_pulses = 0;    

    // Read Pulse Counter value                                                  
    pcnt_get_counter_value(this->pulse_counter_unit, &pulses);                                                                   

    // Calculate the frequency based of the pulse counter readings
    frequency = (pulses + (mult * overflow)) ;                                                         
    
    // Clear Pulse Counter
    pcnt_counter_clear(this->pulse_counter_unit);   

    start_pulse_counter();              
}
