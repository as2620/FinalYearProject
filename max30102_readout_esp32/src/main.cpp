#include <Arduino.h>
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

MAX30105 particleSensor;

#define MAX_BRIGHTNESS 255

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
//Arduino Uno doesn't have enough SRAM to store 100 samples of IR led data and red led data in 32-bit format
//To solve this problem, 16-bit MSB of the sampled data will be truncated. Samples become 16-bit data.
uint16_t irBuffer[100]; //infrared LED sensor data
uint16_t redBuffer[100];  //red LED sensor data
#else
uint32_t irBuffer[100];   // Infrared LED sensor data
uint32_t redBuffer[100];  // Red LED sensor data
#endif

int32_t bufferLength;   // Data length
int32_t spo2;           // SPO2 value
int8_t validSPO2;       // Indicator to show if the SPO2 calculation is valid
int32_t heartRate;      // Heart rate value
int8_t validHeartRate;  // Indicator to show if the heart rate calculation is valid

byte pulseLED = 11;     // Must be on PWM pin
byte readLED = 13;      // Blinks with each data read

void setup()
{
  Serial.begin(115200);
  
  Serial.print("Initializing Pulse Oximeter..");
  
  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) // Use default I2C port, 400kHz speed
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }


  /* The following parameters should be tuned to get the best readings for IR and RED LED. 
   * The perfect values varies depending on your power consumption required, accuracy, ambient light, sensor mounting, etc. 
   * Refer Maxim App Notes to understand how to change these values
   * I got the best readings with these values for my setup. Change after going through the app notes.
  */
  byte ledBrightness = 50;  // Options: 0=Off to 255=50mA
  byte sampleAverage = 1;   // Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2;         // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 100;    // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 69;      // Options: 69, 118, 215, 411
  int adcRange = 4096;      // Options: 2048, 4096, 8192, 16384
  
  // Configure sensor with these settings
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); 
}


void loop()
{
  bufferLength = 100; // Buffer length of 100 stores 4 seconds of samples running at 25sps

  // Read the first 100 samples, and determine the signal range
  for (byte i = 0 ; i < bufferLength ; i++)
  {
    // Do we have new data?
    while (particleSensor.available() == false) 
      //Check the sensor for new data
      particleSensor.check(); 

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();

    // We're finished with this sample so move to next sample
    particleSensor.nextSample(); 
  }

  // Calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  // Continuously taking samples from MAX30102.  
  // Heart rate and SpO2 are calculated every 1 second
  while (1)
  {
    // Dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    // Take 25 sets of samples before calculating the heart rate.
    for (byte i = 75; i < 100; i++)
    {
      // Do we have new data?
      while (particleSensor.available() == false) 
      // Check the sensor for new data
        particleSensor.check(); 

      // Blink onboard LED with every data read
      digitalWrite(readLED, !digitalRead(readLED)); 

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();

      // We're finished with this sample so move to next sample
      particleSensor.nextSample(); 

      // Print RED
      Serial.print(redBuffer[i], DEC);
      Serial.print(" ");
      // Print IR
      Serial.print(irBuffer[i], DEC);
      Serial.print(" ");
      // Print HR
      Serial.print(heartRate, DEC);
      Serial.print(" ");
      // Print SPO2
      Serial.println(spo2, DEC);

    }

    // After gathering 25 new samples recalculate HR and SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  }
}
