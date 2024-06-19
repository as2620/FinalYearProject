//------------------------------------------------------------------------------------------------

// Readout Code for the MAX30102 Sensor Connected to an ESP32.

//------------------------------------------------------------------------------------------------

#include <Arduino.h>
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

//------------------------------------------------------------------------------------------------

#define MAX_BRIGHTNESS 255

//------------------------------------------------------------------------------------------------

MAX30105 ppg_sensor;

//------------------------------------------------------------------------------------------------

uint32_t ir_buffer[100];                     // Infrared LED sensor data
uint32_t red_buffer[100];                    // Red LED sensor data

//------------------------------------------------------------------------------------------------

int32_t buffer_length;                       // Data length
int32_t spo2;                                // SPO2 value
int8_t valid_spo2;                           // Indicator to show if the SPO2 calculation is valid
int32_t heart_rate;                          // Heart rate value
int8_t valid_heart_rate;                     // Indicator to show if the heart rate calculation is valid

byte pulse_led      = 11;                    // Must be on PWM pin
byte red_led        = 13;                    // Blinks with each data read

//------------------------------------------------------------------------------------------------

void setup()
{
  Serial.begin(115200);
  
  Serial.print("LOG: Initializing Pulse Oximeter");
  
  // Initialise sensor, Use default I2C port, 400kHz speed
  if (!ppg_sensor.begin(Wire, I2C_SPEED_FAST)) 
  {
    Serial.println(F("ERROR: MAX3010x was not found. Please check wiring/power."));
    while (1);
  }

  // Set up the MAX30102 sensor with the following settings
  byte led_brightness        = 50;            // Options: 0=Off to 255=50mA
  byte sample_average        = 1;             // Options: 1, 2, 4, 8, 16, 32
  byte led_mode              = 2;             // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sample_rate           = 100;           // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulse_width            = 69;            // Options: 69, 118, 215, 411
  int adc_range              = 4096;          // Options: 2048, 4096, 8192, 16384
  
  // Configure sensor with these settings
  ppg_sensor.setup(led_brightness, sample_average, led_mode, sample_rate, pulse_width, adc_range); 
}

//------------------------------------------------------------------------------------------------

void loop()
{
  buffer_length = 100; 

  // Read the first 100 samples, and determine the signal range
  for (byte i = 0 ; i < buffer_length ; i++)
  {
    while (ppg_sensor.available() == false) 
    {
      // Check the sensor for new data
      ppg_sensor.check(); 
    }

    red_buffer[i] = ppg_sensor.getRed();
    ir_buffer[i] = ppg_sensor.getIR();

    // Move to next sample
    ppg_sensor.nextSample(); 
  }

  // Calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(ir_buffer, buffer_length, red_buffer, &spo2, &valid_spo2, &heart_rate, &valid_heart_rate);

  // Continuously taking samples from MAX30102.  
  // Heart rate and SpO2 are calculated every 1 second
  while (1)
  {
    // Dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
    for (byte i = 25; i < 100; i++)
    {
      red_buffer[i - 25] = red_buffer[i];
      ir_buffer[i - 25] = ir_buffer[i];
    }

    // Take 25 sets of samples before calculating the heart rate.
    for (byte i = 75; i < 100; i++)
    {
      while (ppg_sensor.available() == false) 
      {
        // Check the sensor for new data
        ppg_sensor.check(); 
      }

      // Blink onboard LED with every data read
      digitalWrite(red_led, !digitalRead(red_led)); 

      red_buffer[i] = ppg_sensor.getRed();
      ir_buffer[i] = ppg_sensor.getIR();

      // Move to next sample
      ppg_sensor.nextSample(); 

      // Print RED
      Serial.print(red_buffer[i], DEC);
      Serial.print(" ");

      // Print IR
      Serial.print(ir_buffer[i], DEC);
      Serial.print(" ");

      // Print HR
      Serial.print(heart_rate, DEC);
      Serial.print(" ");

      // Print SPO2
      Serial.println(spo2, DEC);
    }

    // After gathering 25 new samples recalculate HR and SP02
    maxim_heart_rate_and_oxygen_saturation(ir_buffer, buffer_length, red_buffer, &spo2, &valid_spo2, &heart_rate, &valid_heart_rate);
  }
}

//------------------------------------------------------------------------------------------------
