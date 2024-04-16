#include "Arduino.h"
#include "stdio.h"     

//----------------------------------------------------------------------------------

#include "../include/k_rip_sensor.hpp"

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

#define PCNT_COUNT_UNIT_0       PCNT_UNIT_0             // Set Pulse Counter Unit
#define PCNT_COUNT_UNIT_1       PCNT_UNIT_1             // Set Pulse Counter Unit
#define PCNT_COUNT_CHANNEL      PCNT_CHANNEL_0          // Set Pulse Counter channel

#define PCNT_INPUT_SIG_IO_0     GPIO_NUM_34             // Set Pulse Counter input - Freq Meter Input
#define PCNT_INPUT_CTRL_IO_0    GPIO_NUM_32             // Set Pulse Counter Control GPIO pin - HIGH = count up, LOW = count down  
#define PCNT_INPUT_SIG_IO_1     GPIO_NUM_35            // Set Pulse Counter input - Freq Meter Input
#define PCNT_INPUT_CTRL_IO_1    GPIO_NUM_33             // Set Pulse Counter Control GPIO pin - HIGH = count up, LOW = count down  
#define PCNT_H_LIM_VAL          32000                   // Overflow of Pulse Counter 

//---------------------------------------------------------------------------------

KRipSensor top_coil(PCNT_COUNT_UNIT_0, PCNT_COUNT_CHANNEL, PCNT_INPUT_SIG_IO_0, PCNT_INPUT_CTRL_IO_0);
KRipSensor bottom_coil(PCNT_COUNT_UNIT_1, PCNT_COUNT_CHANNEL, PCNT_INPUT_SIG_IO_1, PCNT_INPUT_CTRL_IO_1);

//---------------------------------------------------------------------------------

static void IRAM_ATTR pcnt_intr_handler(void *)                         
{
  // Interrupt handler for counting overflow pulses    

  uint32_t intr_status = PCNT.int_st.val;

  // Increment appropriate Pulse Counter overflow count 
  if (intr_status == PCNT_COUNT_UNIT_0)
  {
    top_coil.mult_pulses++;                                                    
  }
  else
  {
    bottom_coil.mult_pulses++;
  }  

  // Clear Pulse Counter interrupt bit
  PCNT.int_clr.val = BIT(intr_status);                                   
}

//----------------------------------------------------------------------------------

void initialiseCoils()
{
  // Initialize and run Pulse Counter units
  top_coil.intialise();
  bottom_coil.intialise();

  // Setup Register ISR handler
  pcnt_isr_register(pcnt_intr_handler, NULL, 0, NULL);                                      

  top_coil.start_pulse_counter();
  bottom_coil.start_pulse_counter();
}

//---------------------------------------------------------------------------------

void writeDataToSerial()
{
  Serial.print(top_coil.frequency);
  Serial.print(" ");
  Serial.println(bottom_coil.frequency);
}

//----------------------------------------------------------------------------------

void setup()
{
  Serial.begin(115200);
  initialiseCoils();  
}

//----------------------------------------------------------------------------------

void loop()
{
  top_coil.read();
  bottom_coil.read();

  writeDataToSerial();
  
  delay(10); // in milliseconds
}

//----------------------------------------------------------------------------------
