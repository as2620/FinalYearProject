//------------------------------------------------------------------------------------------------

// Readout Code for Sending Two CCS811 Sensors Connected to an ESP32.
// Each sensor is dedicated its own core and take alternating readings. This results in a total
// sampling rate of 8Hz. Data is then sent to a Firebase Realtime Database.

//------------------------------------------------------------------------------------------------

#include "../include/main.hpp"

//------------------------------------------------------------------------------------------------

void CCS1Task(void *pvParameters)
{
  Serial.print("LOG: Task 1 running on core ");
  Serial.println(xPortGetCoreID());

  if(!ccs_1.begin())
  {
    Serial.println("ERROR: Failed to start sensor! Please check your wiring.");
    while(1);
  }

  // Set the measurement mode to read every 250ms
  ccs_1.setDriveMode(CCS811_DRIVE_MODE_250MS);

  // Wait for the sensor to be ready
  while(!ccs_1.available());

  // Read data from the sensor
  while(1)
  {
    if(ccs_1.available())
    {
      if(!ccs_1.readData())
      {
        co2_value = ccs_1.geteCO2();
        voc_value = ccs_1.getTVOC();
      }
    }
  }
}

//------------------------------------------------------------------------------------------------

void CCS2Task(void *pvParameters)
{
  Serial.print("LOG: Task 2 running on core ");
  Serial.println(xPortGetCoreID());
  
  // Intialise the second CCS sensor (using the second I2C bus
  if(!ccs_2.begin(0x5A, &I2C_CCS_2))
  {
    Serial.println("ERROR: Failed to start sensor! Please check your wiring.");
    while(1);
  }

  // Set the measurement mode to read every 250ms
  ccs_2.setDriveMode(CCS811_DRIVE_MODE_250MS);
  
  // Wait for the sensor to be ready
  while(!ccs_2.available());

  // Read data from the sensor
  while(1)
  {
    if(ccs_2.available())
    {
      if (!ccs_2.readData())
      {
        co2_value = ccs_2.geteCO2();
        voc_value = ccs_2.getTVOC();
      }
    }
  }
}

//------------------------------------------------------------------------------------------------

void InitWifi()
{
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("LOG: Connecting to WiFi ..");

  // Wait until the connection is established
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print('.');
    delay(1000);
  }

  // Print the local IP address
  Serial.println(WiFi.localIP());
  Serial.println();
}

//------------------------------------------------------------------------------------------------

String getTimestamp() 
{
  // Get current time
  struct timeval tv;
  gettimeofday(&tv, NULL);

  // Convert timestamp to datetime
  time_t sec = tv.tv_sec;
  struct tm *tm_info;
  char datetime[26];
  tm_info = localtime(&sec);
  strftime(datetime, 26, "%Y-%m-%d %H:%M:%S", tm_info);

  // Construct the timestamp string with microseconds
  String timestamp = String(datetime) + "." + String(tv.tv_usec);
  return timestamp;
}

//------------------------------------------------------------------------------------------------

void Database_Task(void *pvParameters)
{
  Serial.print("LOG: Task 3 running on core ");
  Serial.println(xPortGetCoreID());

  while (1)
  {    
    // Send new readings to database
    if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0))
    {
      sendDataPrevMillis = millis();

      //Get current timestamp
      String timestamp_string = getTimestamp();

      // Construct the JSON object
      parentPath= databasePath + "/" + timestamp_string;

      json.set(co2Path.c_str(), String(co2_value));
      json.set(vocPath.c_str(), String(voc_value));
      json.set(timePath, timestamp_string);

      // Send the data to the database 
      Firebase.RTDB.setJSONAsync(&fbdo, parentPath.c_str(), &json);
    }
  }
}

//------------------------------------------------------------------------------------------------

void setup() 
{
  Serial.begin(115200);
  Serial.println("LOG: Send CCS811 Double Test");

  // Setup I2C Communication 
  // First CSS Sensor:
  Wire.begin(); 

  // Second CSS Sensor:
  I2C_CCS_2.begin(I2C_SDA_2, I2C_SCL_2, 400);

  // ------------------------------------- WiFi and Database --------------------------------------

  InitWifi();
  configTime(0, 0, ntpServer);

  // Assign the api key 
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task
  config.token_status_callback = tokenStatusCallback; 

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialise the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("LOG: Getting User UID");
  while ((auth.token.uid) == "") 
  {
    Serial.print('.');
    delay(1000);
  }
  
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("INFO: User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/mask_data";

  // ------------------------------------------- Tasks --------------------------------------------

  // Create a task for the first sensor
  xTaskCreatePinnedToCore
  (
    CCS1Task,                       // Task function
    "CCS1Task",                     // Task name
    10000,                          // Stack size
    NULL,                           // Parameters
    2,                              // Priority
    &CCS_1_Task_Handle,             // Task handle
    0                               // Core
  );

  // Create a task for the second sensor
  xTaskCreatePinnedToCore
  (
    CCS2Task,                       // Task function
    "CCS2Task",                     // Task name
    10000,                          // Stack size
    NULL,                           // Parameters
    2,                              // Priority
    &CCS_2_Task_Handle,             // Task handle
    1                               // Core
  );

  // Create a task for the database
  xTaskCreate(
    Database_Task,                  // Task function
    "Database_Task",                // Task name
    10000,                          // Stack size
    NULL,                           // Parameters
    3,                              // Priority
    &Database_Task_Handle           // Task handle
  );
}

//----------------------------------------------------------------------------------

void loop() {}

//----------------------------------------------------------------------------------
