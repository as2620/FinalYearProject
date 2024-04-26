
#pragma once

//----------------------------------------------------------------------------------

#include "k_rip_sensor.hpp"
#include "gsr_sensor.hpp"
#include "ppg_sensor.hpp"

//----------------------------------------------------------------------------------

pcnt_unit_t PCNT_COUNT_UNIT_0 = PCNT_UNIT_0;             // Set Pulse Counter Unit
pcnt_unit_t PCNT_COUNT_UNIT_1 = PCNT_UNIT_1;             // Set Pulse Counter Unit
pcnt_channel_t PCNT_COUNT_CHANNEL = PCNT_CHANNEL_0;          // Set Pulse Counter channel

uint8_t PCNT_INPUT_SIG_IO_0 = GPIO_NUM_34;            // Set Pulse Counter input - Freq Meter Input
uint8_t PCNT_INPUT_CTRL_IO_0 = GPIO_NUM_32;             // Set Pulse Counter Control GPIO pin - HIGH = count up, LOW = count down  
uint8_t PCNT_INPUT_SIG_IO_1 = GPIO_NUM_35;            // Set Pulse Counter input - Freq Meter Input
uint8_t PCNT_INPUT_CTRL_IO_1 = GPIO_NUM_33;             // Set Pulse Counter Control GPIO pin - HIGH = count up, LOW = count down  
uint16_t PCNT_H_LIM_VAL = 32000;                   // Overflow of Pulse Counter 

//---------------------------------------------------------------------------------

uint8_t GSR_PIN = GPIO_NUM_39;

//---------------------------------------------------------------------------------

byte PPG_LED_BRIGHTNESS = 50;
byte PPG_SAMPLE_AVERAGE = 1;
byte PPG_LED_MODE = 2;
byte PPG_SAMPLE_RATE = 100;
int PPG_PULSE_WIDTH = 69;
int PPG_ADC_RANGE = 4096;

//---------------------------------------------------------------------------------

uint8_t SERIAL_LOOP_DELAY = 10;
uint8_t GSR_LOOP_DELAY = 5;

uint32_t GSR_LOOP_COUNTER = 0;
uint32_t GSR_LOOP_COUNTER_LIMIT = 50 / GSR_LOOP_DELAY;

//---------------------------------------------------------------------------------

KRipSensor top_coil(PCNT_COUNT_UNIT_0, PCNT_COUNT_CHANNEL, PCNT_INPUT_SIG_IO_0, PCNT_INPUT_CTRL_IO_0);
KRipSensor bottom_coil(PCNT_COUNT_UNIT_1, PCNT_COUNT_CHANNEL, PCNT_INPUT_SIG_IO_1, PCNT_INPUT_CTRL_IO_1);

GsrSensor gsr_sensor(GSR_PIN);

PpgSensor ppg_sensor(PPG_LED_BRIGHTNESS, PPG_SAMPLE_AVERAGE, PPG_LED_MODE, PPG_SAMPLE_RATE, PPG_PULSE_WIDTH, PPG_ADC_RANGE);

//---------------------------------------------------------------------------------

hw_timer_t *k_rip_timer = NULL;

TaskHandle_t PPG_Task_Handle = NULL;
TaskHandle_t GSR_Task_Handle = NULL;

TaskHandle_t Database_Task_Handle = NULL;

//---------------------------------------------------------------------------------

// SemaphoreHandle_t k_rip_mutex = NULL;
// SemaphoreHandle_t gsr_mutex = NULL;
// SemaphoreHandle_t ppg_mutex = NULL;

//---------------------------------------------------------------------------------

#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include "time.h"

// Token Generation Process Info
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Network Credentials
#define WIFI_SSID "arthika_laptop"
#define WIFI_PASSWORD "12345678"

// Firebase Project API Key
#define API_KEY "AIzaSyD-stiJVk3e745eKPPHMfOQZZMozTBe-XQ"

// Authorized Email and Corresponding Password
#define USER_EMAIL "as2620@ic.ac.uk"
#define USER_PASSWORD "fyp_esp_32"

// RTDB URLefine the RTDB URL
#define DATABASE_URL "https://fyp-sensor-data-9b8bc-default-rtdb.europe-west1.firebasedatabase.app/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String chestCoilPath = "/chest_coil";
String abdomenCoilPath = "/abdomen_coil";
String gsrPath = "/gsr";
String ppgIrPath = "/ppg_ir";
String ppgRedPath = "/ppg_red";
String timePath = "/timestamp";

// Parent Node (to be updated in every loop)
String parentPath;

FirebaseJson json;

const char* ntpServer = "pool.ntp.org";

// Timer variables
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 1; // in milliseconds

float timestamp = 0;
float timestamp_millis = 0;
float prev_timestamp_millis = 0;   

//----------------------------------------------------------------------------------