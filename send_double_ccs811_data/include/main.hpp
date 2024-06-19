//------------------------------------------------------------------------------------------------

// Header File for Sending Two CCS811 Sensors Connected to an ESP32.
// Each sensor is dedicated its own core and take alternating readings. This results in a total
// sampling rate of 8Hz. Data is then sent to a Firebase Realtime Database.

// Please fill in the TODOs in the code below

//------------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------------

#include <Arduino.h>

#include <SPI.h>
#include "Adafruit_CCS811.h"
#include <Wire.h>

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "time.h"

#include <sys/time.h>
#include <ctime>

//------------------------------------------------------------------------------------------------

// Token generation process info
#include "addons/TokenHelper.h"

// RTDB payload printing info and other helper functions
#include "addons/RTDBHelper.h"

//------------------------------------------------------------------------------------------------

// Network credentials
#define WIFI_SSID "TODO: Fill in your WiFi SSID here"
#define WIFI_PASSWORD "TODO: Fill in your WiFi Password here"

//------------------------------------------------------------------------------------------------

// Firebase project API Key
#define API_KEY "TODO: Fill in your Firebase API Key here"

// Authorized Email and Corresponding Password
#define USER_EMAIL "TODO: Fill in your Firebase User Email here"
#define USER_PASSWORD "TODO: Fill in your Firebase User Password here"

// RTDB URLefine the RTDB URL
#define DATABASE_URL "TODO: Fill in your Firebase RTDB URL here"

//------------------------------------------------------------------------------------------------

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;

// Database child nodes
String co2Path                                      = "/CO2";
String vocPath                                      = "/VOC";
String timePath                                     = "/timestamp";

// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
FirebaseJson json;

const char* ntpServer                               = "pool.ntp.org";

// Timer variables 
unsigned long sendDataPrevMillis                    = 0;
unsigned long timerDelay                            = 10; // Send readings every x milliseconds

//------------------------------------------------------------------------------------------------

// CCS811 objects
Adafruit_CCS811 ccs_1;
Adafruit_CCS811 ccs_2;

// ESP32 I2C pins
// First CSS Sensor
const uint8_t I2C_SDA_1                             = 21;
const uint8_t I2C_SCL_1                             = 22;

// Second CSS Sensor
const uint8_t I2C_SDA_2                             = 33;
const uint8_t I2C_SCL_2                             = 32;

// I2C Buses
TwoWire I2C_CCS_2                                   = TwoWire(0);

// Task Handles 
TaskHandle_t CCS_1_Task_Handle;
TaskHandle_t CCS_2_Task_Handle;
TaskHandle_t Database_Task_Handle;

// Variables 
uint32_t co2_value;
uint32_t voc_value;

//------------------------------------------------------------------------------------------------
