// ESP32 Frequency Meter
// Code by Kristel Fobelets and Kris Thielemans 11/2/2023
// Adapted from:
// https://blog.eletrogate.com/esp32-frequencimetro-de-precisao
// Rui Viana and Gustavo Murta august/2020
// pins https://github.com/espressif/arduino-esp32/blob/master/variants/esp32s2/pins_arduino.h
// if the board disappears from Tools then follow the steps described in:
// https://blog.espressif.com/arduino-for-esp32-s2-and-esp32-c3-is-coming-f36d79967eb8

/* set pulse counters for the ESP32 WROOM 32D collpit oscillator implementation
//#define PCNT_INPUT_SIG_IO_0     GPIO_NUM_26                               
//#define PCNT_INPUT_CTRL_IO_0    GPIO_NUM_32                               
//#define PCNT_INPUT_SIG_IO_1     GPIO_NUM_27                              
//#define PCNT_INPUT_CTRL_IO_1    GPIO_NUM_33                             
//#define PCNT_H_LIM_VAL          overflow
*/

// (Un)comment next 2 lines according to where you want output to be written
// #define SC_CARD_OUT
#define Serial_OUT

#include "Arduino.h"
#include "stdio.h"                                                        // Library STDIO
#include "driver/pcnt.h"                                                  // Library ESP32 PCNT
#include "soc/pcnt_struct.h"
#ifdef SC_CARD_OUT
#include "FS.h"
#include "SD.h"
#endif
#include "SPI.h"

#ifdef SC_CARD_OUT
SPIClass spi = SPIClass(2);
File output;
const char * const SD_filename = "/output.csv";
#endif

#define PCNT_COUNT_UNIT_0       PCNT_UNIT_0                              // Set Pulse Counter Unit
#define PCNT_COUNT_UNIT_1       PCNT_UNIT_1                              // Set Pulse Counter Unit
#define PCNT_COUNT_CHANNEL      PCNT_CHANNEL_0                           // Set Pulse Counter channel

#define PCNT_INPUT_SIG_IO_0     GPIO_NUM_26                               // Set Pulse Counter input - Freq Meter Input
#define PCNT_INPUT_CTRL_IO_0    GPIO_NUM_32                               // Set Pulse Counter Control GPIO pin - HIGH = count up, LOW = count down  
#define PCNT_INPUT_SIG_IO_1     GPIO_NUM_27                              // Set Pulse Counter input - Freq Meter Input
#define PCNT_INPUT_CTRL_IO_1    GPIO_NUM_33                             // Set Pulse Counter Control GPIO pin - HIGH = count up, LOW = count down  
#define PCNT_H_LIM_VAL          overflow                                 // Overflow of Pulse Counter 

volatile bool   flag            = true;                                  // Flag to enable print frequency reading
const uint32_t  overflow        = 32000;                                 // Max Pulse Counter value 32000
volatile uint32_t multPulses_0  = 0;                                     // Number of overflows of PCNT
volatile uint32_t multPulses_1  = 0;                                     // Number of overflows of PCNT
volatile int64_t  esp_time      = 0;                                     // time elapsed in microseconds since boot
volatile int64_t  esp_time_interval = 0;                                 // actual time between 2 samples (should be close to sample_time)
const uint32_t  sample_time     = 10000;                                 // sample time in microseconds to count pulses
volatile uint32_t frequency_0   = 0;                                     // frequency value
volatile uint32_t frequency_1   = 0;                                     // frequency value
int             print_counter   = 0;                                     // count when we store results
const int       print_every     = 10;                                    // how many samples we skip before storing

portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;                    // portMUX_TYPE to do synchronisation

#ifdef SC_CARD_OUT

//--------SD card reader functions ---------------------------------------------------------------------

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

#endif

//----------------------------------------------------------------------------------

static void IRAM_ATTR pcnt_intr_handler(void *)                         // Interrupt handler for counting overflow pulses
{
 #if 1
  portENTER_CRITICAL_ISR(&timerMux);                                    // disabling the interrupts
  uint32_t intr_status = PCNT.int_st.val;
  if (intr_status == PCNT_COUNT_UNIT_0)
    multPulses_0++;                                                     // increment Overflow counter
  else
    multPulses_1++;  
  PCNT.int_clr.val = BIT(intr_status);                                  // Clear Pulse Counter interrupt bit
  portEXIT_CRITICAL_ISR(&timerMux);                                     // enabling the interrupts
#else
  uint32_t intr_status = PCNT.int_st.val;
  PCNT.int_clr.val = BIT(intr_status);                                  // Clear Pulse Counter interrupt bit
  #endif
}

//----------------------------------------------------------------------------------

void read_counters()                                                    // Read Pulse Counters
{
  pcnt_counter_pause(PCNT_COUNT_UNIT_0); 
  pcnt_counter_pause(PCNT_COUNT_UNIT_1); 
  const int64_t esp_time_now = esp_timer_get_time();
  int16_t pulses_0 = 0;
  int16_t pulses_1 = 0;
  portENTER_CRITICAL_ISR(&timerMux);                                      // disabling the interrupts
  uint32_t mult_0 = multPulses_0;
  multPulses_0 = 0;                                                       // Clear overflow counter
  pcnt_get_counter_value(PCNT_COUNT_UNIT_0, &pulses_0);                   // Read Pulse Counter value
  uint32_t mult_1 = multPulses_1;
  multPulses_1 = 0;                                                       // Clear overflow counter
  pcnt_get_counter_value(PCNT_COUNT_UNIT_1, &pulses_1);                   // Read Pulse Counter value
  portEXIT_CRITICAL_ISR(&timerMux);                                       // enabling the interrupts
  frequency_0 = (pulses_0 + (mult_0 * overflow)) ;                        // Calculation of frequency.
  pcnt_counter_clear(PCNT_COUNT_UNIT_0);                                  // Clear Pulse Counter
  frequency_1 = (pulses_1 + (mult_1 * overflow)) ;                        // Calculation of frequency.
  pcnt_counter_clear(PCNT_COUNT_UNIT_1);                                  // Clear Pulse Counter
  esp_time_interval = esp_time_now - esp_time;
}

//----------------------------------------------------------------------------------

void start_counters()
{
  esp_time = esp_timer_get_time();
  pcnt_counter_resume(PCNT_COUNT_UNIT_0); 
  pcnt_counter_resume(PCNT_COUNT_UNIT_1); 
}

//----------------------------------------------------------------------------------

void write_data()
{
      // Note: esp_time is the time at the end of the read_PCNT
      // i.e. (approximately) the end of the interval that was read (not the beginning)
#ifdef Serial_OUT
      Serial.print(esp_time);
      Serial.print(", ");
      Serial.print(frequency_0);
      Serial.print(", ");
      //Serial.print(multPulses_0);
      //Serial.print(" ");
      //Serial.print(multPulses_1);
      //Serial.print(" ");
      Serial.println(frequency_1);
#endif
#ifdef SC_CARD_OUT
      output = SD.open(SD_filename,FILE_APPEND);
      output.print(esp_time_interval);
      output.print(" , ");
      output.print(esp_time);
      output.print(" , ");
      output.print(frequency_0);
      output.print(" , ");
      output.println(frequency_1);
      output.close();
#endif
}

//----------------------------------------------------------------------------------

void init_PCNT(pcnt_unit_t unit, int pcnt_input_sig_io, int pcnt_input_ctrl_io)                                                      // Initialize and run PCNT unit
{
  pcnt_config_t pcnt_config = { };                                        // PCNT unit instance

  pcnt_config.pulse_gpio_num = pcnt_input_sig_io;                         // Pulse input GPIO - Freq Meter Input
  pcnt_config.ctrl_gpio_num = pcnt_input_ctrl_io;                         // Control signal input GPIO
  pcnt_config.unit = unit;                                                // Counting unit PCNT - 0
  pcnt_config.channel = PCNT_COUNT_CHANNEL;                               // PCNT unit number - 0
  pcnt_config.counter_h_lim = PCNT_H_LIM_VAL;                             // Maximum counter value
  pcnt_config.pos_mode = PCNT_COUNT_INC;                                  // PCNT positive edge count mode - inc
  //pcnt_config.neg_mode = PCNT_COUNT_INC;                                // PCNT negative edge count mode - inc
  pcnt_config.lctrl_mode = PCNT_MODE_DISABLE;                             // PCNT low control mode - disable
  pcnt_config.hctrl_mode = PCNT_MODE_KEEP;                                // PCNT high control mode - won't change counter mode
  pcnt_unit_config(&pcnt_config);                                         // Initialize PCNT unit

  pcnt_counter_pause(unit);                                               // Pause PCNT unit
  pcnt_counter_clear(unit);                                               // Clear PCNT unit

  pcnt_event_enable(unit, PCNT_EVT_H_LIM);                                // Enable event to watch - max count
  pcnt_intr_enable(unit);                                                 // Enable interrupts for PCNT unit

  //pcnt_filter_enable(unit);
  //pcnt_set_filter_value(unit, 5);
  //pcnt_counter_resume(unit);                                              // Resume PCNT unit - starts count
}

//----------------------------------------------------------------------------------

void read_PCNT(void *)                                                    // Read Pulse Counter callback
{
  read_counters();
  // write_data();
  start_counters();
  flag = true;                                                            // Change flag to enable print
}

//----------------------------------------------------------------------------------

void init_frequencyMeter ()
{
  // Initialize and run PCNT units
  init_PCNT(PCNT_COUNT_UNIT_0, PCNT_INPUT_SIG_IO_0, PCNT_INPUT_CTRL_IO_0);
  init_PCNT(PCNT_COUNT_UNIT_1, PCNT_INPUT_SIG_IO_1, PCNT_INPUT_CTRL_IO_1);
  pcnt_isr_register(pcnt_intr_handler, NULL, 0, NULL);                    // Setup Register ISR handler

  // create periodic timer for reading-out PCNTs
  esp_timer_create_args_t create_args;
  esp_timer_handle_t timer_handle;
  create_args.callback = read_PCNT;                                       // Set esp-timer argument
  esp_timer_create(&create_args, &timer_handle);                          // Create esp-timer instance
  esp_timer_start_periodic(timer_handle, sample_time);                    // Initialize High resolution timer (dependent on sampling time.
  start_counters();
}

//----------------------------------------------------------------------------------

void setup()
{
  Serial.begin(115200);                                                   // Serial Console Arduino 115200 Bps
#ifdef SC_CARD_OUT
   spi.begin();                                                            // setting up SPI pins for ESP32S2 - for SD card interactions
  
  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }

  output = SD.open(SD_filename, FILE_WRITE);                            // clear output.csv file on SD card reader
  if(!output){
    Serial.println("Failed to open file for writing");
    return;
  }
  output.close();
#endif

  init_frequencyMeter();                                                 // Initialize Frequency Meter
}

//---------------------------------------------------------------------------------

void loop()
{
  if (flag == true)                                                     // If count has ended
  {
    flag = false;                                                       // change flag to disable print
    print_counter++;
    if (print_counter>=print_every)
    {
      print_counter = 0;
      write_data();
    }
  }
  
  delay(1); // units are millisecs
}

//---------------------------------------------------------------------------------