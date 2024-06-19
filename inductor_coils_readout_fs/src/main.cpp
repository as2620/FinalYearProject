//------------------------------------------------------------------------------------------------

// Readout Code for K-RIP Coils Connected to an ESP32.
// The code reads the frequency of the top and bottom coils and prints it to the serial monitor.
// This is achieved by using the Pulse Counter Peripheral in the ESP32 to count the number of edges.

//------------------------------------------------------------------------------------------------

// Pulse Counters in the ESP32 
// This piece of code makes use of the Pulse Counter Peripheral in the ESP32. The Pulse Counters 
// count the number edges of the waveform that pass during a set interval of time. This changes 
// as the frequency of the waveform changes. 

// How Do Pulse Counters Work? 
// Pulse Counters in the ESP32 require the programmer to select a unit. A unit has two channels, 
// one that can increment the counter and the other that can decrement the counter on a falling or 
// rising edge. Pulse Counters can be configured to do something on either or both edges. The Control 
// Signal is used to control the counting mode of the edge signals that are attached to the same 
// channel. 

//------------------------------------------------------------------------------------------------

#include "Arduino.h"
#include "stdio.h"                                                        
#include "driver/pcnt.h"                                                  
#include "soc/pcnt_struct.h"

//------------------------------------------------------------------------------------------------

// Define Pulse Counter Parameters
#define PCNT_COUNT_UNIT_0                     PCNT_UNIT_0             // Set Pulse Counter Unit
#define PCNT_COUNT_UNIT_1                     PCNT_UNIT_1             // Set Pulse Counter Unit
#define PCNT_COUNT_CHANNEL                    PCNT_CHANNEL_0          // Set Pulse Counter channel

#define PCNT_INPUT_SIG_IO_0                   GPIO_NUM_34             // Set Pulse Counter input - Freq Meter Input
#define PCNT_INPUT_SIG_IO_1                   GPIO_NUM_35             // Set Pulse Counter input - Freq Meter Input
const uint32_t PCNT_H_LIM_VAL                 = 32000;                // Max Pulse Counter value 32000

//------------------------------------------------------------------------------------------------

// Number of overflows of PCNT
volatile uint32_t mult_pulses_0               = 0;                // Number of overflows of PCNT
volatile uint32_t mult_pulses_1               = 0;                // Number of overflows of PCNT

// Time variables
volatile int64_t esp_time                     = 0;                // Time elapsed in microseconds since boot
volatile int64_t esp_time_interval            = 0;                // Actual time between 2 samples (should be close to sample_time)

const uint32_t sample_time                    = 10000;            // Sample time in microseconds to count pulses

// Coil frequency variables
volatile uint32_t frequency_top               = 0;                // Frequency value for top coil 
volatile uint32_t frequency_bottom            = 0;                // Frequency value for bottom coil

// Mutex to do synchronisation
portMUX_TYPE timerMux                         = portMUX_INITIALIZER_UNLOCKED;   // portMUX_TYPE to do synchronisation

//------------------------------------------------------------------------------------------------

// Interrupt handler for counting overflow pulses
static void IRAM_ATTR PcntIntrHandler(void *)                         
{
  // Disable interrupt
  portENTER_CRITICAL_ISR(&timerMux);    

  uint32_t intr_status = PCNT.int_st.val;

  // Increment appropriate Pulse Counter overflow count 
  if (intr_status == PCNT_COUNT_UNIT_0)
  {
    mult_pulses_0++;                                                    
  }
  else
  {
    mult_pulses_1++;
  }  

  // Clear Pulse Counter interrupt bit
  PCNT.int_clr.val = BIT(intr_status); 
  
  //Enable interupt                                 
  portEXIT_CRITICAL_ISR(&timerMux);                                    
}

//------------------------------------------------------------------------------------------------

void writeDataToSerial()
{
  Serial.print(esp_time);
  Serial.print(" ");
  Serial.print(frequency_top);
  Serial.print(" ");
  Serial.println(frequency_bottom);
}

//------------------------------------------------------------------------------------------------

void startCounters()
{
  esp_time = esp_timer_get_time();
  pcnt_counter_resume(PCNT_COUNT_UNIT_0); 
  pcnt_counter_resume(PCNT_COUNT_UNIT_1); 
}

//------------------------------------------------------------------------------------------------

void readCounters()
{
  // Pause Pulse Counters
  pcnt_counter_pause(PCNT_COUNT_UNIT_0); 
  pcnt_counter_pause(PCNT_COUNT_UNIT_1); 

  // Get the current time
  const int64_t esp_time_now = esp_timer_get_time();

  int16_t pulses_0 = 0;
  int16_t pulses_1 = 0;

  // Disable the interrupts
  portENTER_CRITICAL_ISR(&timerMux);   
                                     
  uint32_t mult_0 = mult_pulses_0;
  uint32_t mult_1 = mult_pulses_1;

  // Clear overflow counter
  mult_pulses_0 = 0;    
  mult_pulses_1 = 0;  

  // Read Pulse Counter value                                                  
  pcnt_get_counter_value(PCNT_COUNT_UNIT_0, &pulses_0);                                                                    
  pcnt_get_counter_value(PCNT_COUNT_UNIT_1, &pulses_1);       

  // Enable the interrupts
  portEXIT_CRITICAL_ISR(&timerMux);  

  // Calculate the frequency based of the pulse counter readings
  frequency_top = (pulses_0 + (mult_0 * PCNT_H_LIM_VAL)) ;                                                         
  frequency_bottom = (pulses_1 + (mult_1 * PCNT_H_LIM_VAL)) ;    
  
  // Clear Pulse Counter
  pcnt_counter_clear(PCNT_COUNT_UNIT_0);                   
  pcnt_counter_clear(PCNT_COUNT_UNIT_1);   

  // Calculate time interval                               
  esp_time_interval = esp_time_now - esp_time;
}

//------------------------------------------------------------------------------------------------

void initPulseCounter(pcnt_unit_t unit, int pcnt_input_sig_io)                                                      
{
  // PCNT unit instance
  pcnt_config_t pcnt_config = {};                                        

  // Pulse input GPIO - Freq Meter Input
  pcnt_config.pulse_gpio_num = pcnt_input_sig_io; 

  //  Pulse Counter, Counting Unit                      
  pcnt_config.unit = unit;   

  // Pulse Counter, Channel - 0                                             
  pcnt_config.channel = PCNT_COUNT_CHANNEL;       

  // Maximum counter value                        
  pcnt_config.counter_h_lim = PCNT_H_LIM_VAL;  

  // PCNT positive edge count mode - inc
  pcnt_config.pos_mode = PCNT_COUNT_INC;                                  

  // Configure Pulse Counter control mode
  pcnt_config.lctrl_mode = PCNT_MODE_DISABLE;                             
  pcnt_config.hctrl_mode = PCNT_MODE_KEEP;    

  // Initialize PCNT unit
  pcnt_unit_config(&pcnt_config);                                         

  // Pause PCNT unit
  pcnt_counter_pause(unit);
  
  // Clear PCNT unit                                               
  pcnt_counter_clear(unit);                                               

  // Enable event to watch - max count
  pcnt_event_enable(unit, PCNT_EVT_H_LIM);     

  // Enable interrupts for Pulse Counter unit
  pcnt_intr_enable(unit);                                                 
}

//------------------------------------------------------------------------------------------------

void readPulseCounter(void *)                                               
{
  readCounters();
  startCounters();                                                       
}

//------------------------------------------------------------------------------------------------

void initFrequencyMeter()
{
  // Initialize and run Pulse Counter units
  initPulseCounter(PCNT_COUNT_UNIT_0, PCNT_INPUT_SIG_IO_0);
  initPulseCounter(PCNT_COUNT_UNIT_1, PCNT_INPUT_SIG_IO_1);

  // Setup Register ISR handler
  pcnt_isr_register(PcntIntrHandler, NULL, 0, NULL);                    

  // Create periodic timer to read out the Pulse Counter arguements, every 10ms (sampling_time)
  esp_timer_create_args_t create_args;
  esp_timer_handle_t timer_handle;

  // When timer expires, call the readPulseCounter() function
  create_args.callback = readPulseCounter;   

  // Create an esp-timer instance
  esp_timer_create(&create_args, &timer_handle);                          
  esp_timer_start_periodic(timer_handle, sample_time);                    

  startCounters();
}

//------------------------------------------------------------------------------------------------

void setup()
{
  Serial.begin(115200);
  initFrequencyMeter();  
}

//------------------------------------------------------------------------------------------------

void loop()
{
  writeDataToSerial();
  delay(10);
}

//------------------------------------------------------------------------------------------------
