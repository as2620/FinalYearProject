
#include "../include/main.hpp"

//----------------------------------------------------------------------------------

void CCS_1_Task(void *pvParameters)
{
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  if(!ccs_1.begin())
  {
    Serial.println("Failed to start sensor! Please check your wiring.");
    while(1);
  }

  ccs_1.setDriveMode(CCS811_DRIVE_MODE_250MS);
  while(!ccs_1.available());

  while(1)
  {
    if(ccs_1.available())
    {
      if(!ccs_1.readData())
      {
        // Print timestamp
        // Serial.print("Sensor 1");
        // Serial.print(" ");
        // Serial.print(esp_timer_get_time());
        // Serial.print(" ");
        // // Print CO2 value
        // Serial.print(ccs_1.geteCO2());
        // Serial.print(" ");  
        // // Print TVOC value
        // Serial.println(ccs_1.getTVOC());

        co2_value = ccs_1.geteCO2();
        voc_value = ccs_1.getTVOC();
      }
    }
  }
}

//----------------------------------------------------------------------------------

void CCS_2_Task(void *pvParameters)
{
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  
  // (2) Intialise the second CCS sensor (using the second I2C bus
  if(!ccs_2.begin(0x5A, &I2C_CCS_2))
  {
    Serial.println("Failed to start sensor! Please check your wiring.");
    while(1);
  }

  // (3) Set the measurement mode
  ccs_2.setDriveMode(CCS811_DRIVE_MODE_250MS);
  
  // (4) Wait for the sensor to be ready
  while(!ccs_2.available());

  while(1)
  {
    if(ccs_2.available())
    {
      if (!ccs_2.readData())
      {
        // Print timestamp
        // Serial.print("Sensor 2");
        // Serial.print(" ");
        // Serial.print(esp_timer_get_time());
        // Serial.print(" ");
        // // Print CO2 value
        // Serial.print(ccs_2.geteCO2());
        // Serial.print(" ");  
        // // Print TVOC value
        // Serial.println(ccs_2.getTVOC());

        co2_value = ccs_2.geteCO2();
        voc_value = ccs_2.getTVOC();
      }
    }
  }
}

//----------------------------------------------------------------------------------

void Init_Wifi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

//----------------------------------------------------------------------------------

String getTimestamp() {
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

//----------------------------------------------------------------------------------

void Database_Task(void *pvParameters)
{
  Serial.print("Task 3 running on core ");
  Serial.println(xPortGetCoreID());

  while (1)
  {    
    // Send new readings to database
    if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
      sendDataPrevMillis = millis();

      //Get current timestamp
      String timestamp_string = getTimestamp();
      Serial.print ("time: ");
      Serial.println (timestamp);

      parentPath= databasePath + "/" + timestamp_string;

      json.set(co2Path.c_str(), String(co2_value));
      json.set(vocPath.c_str(), String(voc_value));
      json.set(timePath, timestamp_string);
      Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
    }
  }
}

//----------------------------------------------------------------------------------

void setup() 
{
  Serial.begin(115200);
  Serial.println("CCS811 Double Test");

  //-------------------------------- Setup I2C Communication --------------------------------

  // First CSS Sensor:
  // (1) Enable I2C
  Wire.begin(); 

  // Second CSS Sensor:
  // (1) Enable I2C since the two sensors have the same address
  I2C_CCS_2.begin(I2C_SDA_2, I2C_SCL_2, 400);

  // -------------------------------- WiFi and Database --------------------------------

  Init_Wifi();
  configTime(0, 0, ntpServer);

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/mask_data";

  // -------------------------------- Tasks --------------------------------
  // Create a task for the first sensor
  xTaskCreatePinnedToCore(
    CCS_1_Task, // Task function
    "CCS_1_Task", // Task name
    10000, // Stack size
    NULL, // Parameters
    2, // Priority
    &CCS_1_Task_Handle, // Task handle
    0 // Core
  );

  // Create a task for the second sensor
  xTaskCreatePinnedToCore(
    CCS_2_Task, // Task function
    "CCS_2_Task", // Task name
    10000, // Stack size
    NULL, // Parameters
    2, // Priority
    &CCS_2_Task_Handle, // Task handle
    1 // Core
  );

  // Create a task for the database
  xTaskCreate(
    Database_Task, // Task function
    "Database_Task", // Task name
    10000, // Stack size
    NULL, // Parameters
    3, // Priority
    &Database_Task_Handle // Task handle
  );
}

//----------------------------------------------------------------------------------

void loop() {}

//----------------------------------------------------------------------------------
