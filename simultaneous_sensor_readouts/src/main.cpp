
#include "../include/k_rip_sensor.hpp"
#include "../include/gsr_sensor.hpp"

//----------------------------------------------------------------------------------

#define PCNT_COUNT_UNIT_0       PCNT_UNIT_0             // Set Pulse Counter Unit
#define PCNT_COUNT_UNIT_1       PCNT_UNIT_1             // Set Pulse Counter Unit
#define PCNT_COUNT_CHANNEL      PCNT_CHANNEL_0          // Set Pulse Counter channel

#define PCNT_INPUT_SIG_IO_0     GPIO_NUM_34             // Set Pulse Counter input - Freq Meter Input
#define PCNT_INPUT_CTRL_IO_0    GPIO_NUM_32             // Set Pulse Counter Control GPIO pin - HIGH = count up, LOW = count down  
#define PCNT_INPUT_SIG_IO_1     GPIO_NUM_35            // Set Pulse Counter input - Freq Meter Input
#define PCNT_INPUT_CTRL_IO_1    GPIO_NUM_33             // Set Pulse Counter Control GPIO pin - HIGH = count up, LOW = count down  
#define PCNT_H_LIM_VAL          32000                   // Overflow of Pulse Counter 

uint8_t GSR_PIN = 25;

//---------------------------------------------------------------------------------

uint32_t LOOP_DELAY = 10;

uint32_t GSR_LOOP_COUNTER = 0;
uint32_t PPG_LOOP_COUNTER = 0;

uint32_t GSR_LOOP_COUNTER_LIMIT = 50 / LOOP_DELAY;
uint32_t PPG_LOOP_COUNTER_LIMIT = 0;

//---------------------------------------------------------------------------------

KRipSensor top_coil(PCNT_COUNT_UNIT_0, PCNT_COUNT_CHANNEL, PCNT_INPUT_SIG_IO_0, PCNT_INPUT_CTRL_IO_0);
KRipSensor bottom_coil(PCNT_COUNT_UNIT_1, PCNT_COUNT_CHANNEL, PCNT_INPUT_SIG_IO_1, PCNT_INPUT_CTRL_IO_1);

GsrSensor gsr_sensor(GSR_PIN);

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

void intialiseSensors()
{
  initialiseCoils();
}

//----------------------------------------------------------------------------------

void writeDataToSerial()
{
  Serial.print(top_coil.frequency);
  Serial.print(" ");
  Serial.print(bottom_coil.frequency);
  Serial.print(" ");
  Serial.println(gsr_sensor.averaged_gsr_value);
}

//----------------------------------------------------------------------------------

void setup()
{
  Serial.begin(115200);
  intialiseSensors();  
}

//----------------------------------------------------------------------------------

void loop()
{

  // fast
  top_coil.read();
  
  // fast
  bottom_coil.read();

  // 50ms
  if (GSR_LOOP_COUNTER < GSR_LOOP_COUNTER_LIMIT)
  {
    gsr_sensor.read();
    GSR_LOOP_COUNTER++;
  }
  else
  {
    gsr_sensor.calculateAverageValue();
    GSR_LOOP_COUNTER = 0;
  }

  writeDataToSerial();
  
  delay(LOOP_DELAY); // in milliseconds
}

//----------------------------------------------------------------------------------
