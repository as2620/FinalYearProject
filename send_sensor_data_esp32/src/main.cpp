
#include "../include/main.hpp"

//----------------------------------------------------------------------------------

void IRAM_ATTR timerIsr()
{
    // xSemaphoreTake(k_rip_mutex, portMAX_DELAY);
    top_coil.read();
    bottom_coil.read();
    // xSemaphoreGive(k_rip_mutex);
}

//----------------------------------------------------------------------------------

void writeDataToSerial()
{
  // if (xSemaphoreTake(k_rip_mutex, portMAX_DELAY) == pdTRUE) 
  // {
    // if (xSemaphoreTake(gsr_mutex, portMAX_DELAY) == pdTRUE) 
    // {
    //   if (xSemaphoreTake(ppg_mutex, portMAX_DELAY) == pdTRUE) 
    //   {
        // Write data to serial port
        Serial.print(top_coil.frequency);
        Serial.print(" ");
        Serial.print(bottom_coil.frequency);
        Serial.print(" ");
        Serial.print(gsr_sensor.averaged_gsr_value);
        Serial.print(" ");
        Serial.print(ppg_sensor.red_value, DEC);
        Serial.print(" ");
        Serial.println(ppg_sensor.ir_value, DEC);

    //     xSemaphoreGive(ppg_mutex);
    //   }
    //   xSemaphoreGive(gsr_mutex);
    // }
    // xSemaphoreGive(k_rip_mutex);
  // }
}

//----------------------------------------------------------------------------------

void PPG_Task(void *pvParameters)
{
  Serial.print("Task 1 running on core ");
  Serial.println(xPortGetCoreID());
  while (1)
  {
    // xSemaphoreTake(ppg_mutex, portMAX_DELAY);
    ppg_sensor.read();
    // xSemaphoreGive(ppg_mutex);
  }
}

//----------------------------------------------------------------------------------

void GSR_Task(void *pvParameters)
{
  Serial.print("Task 2 running on core ");
  Serial.println(xPortGetCoreID());

  while (1)
  {
    if (GSR_LOOP_COUNTER < GSR_LOOP_COUNTER_LIMIT)
    {
      // xSemaphoreTake(gsr_mutex, portMAX_DELAY);
      gsr_sensor.read();
      // xSemaphoreGive(gsr_mutex);

      GSR_LOOP_COUNTER++;
    }
    else
    {
      // xSemaphoreTake(gsr_mutex, portMAX_DELAY);
      gsr_sensor.calculateAverageValue();
      // xSemaphoreGive(gsr_mutex);

      GSR_LOOP_COUNTER = 0;
    }

    vTaskDelay(pdMS_TO_TICKS(GSR_LOOP_DELAY));
  }
}

//----------------------------------------------------------------------------------

// Initialize WiFi
void initWiFi() {
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

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
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
      timestamp = getTime();
      Serial.print ("time: ");
      Serial.println (timestamp);

      parentPath= databasePath + "/" + String(timestamp);

      json.set(chestCoilPath.c_str(), String(top_coil.frequency));
      json.set(abdomenCoilPath.c_str(), String(bottom_coil.frequency));
      json.set(gsrPath.c_str(), String((gsr_sensor.averaged_gsr_value));
      json.set(ppgRedPath.c_str(), String(ppg_sensor.red_value));
      json.set(ppgIrPath.c_str(), String(ppg_sensor.ir_value));
      json.set(timePath, String(timestamp));
      Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
    }
  }
}

//----------------------------------------------------------------------------------

void setup()
{
  Serial.begin(115200);
  Serial.println("Simultanous K-RIP, GSR and PPG Sensor Data Acquisition");

  // -------------------------------- PPG Sensor --------------------------------

  if (!ppg_sensor.intialise())
  {
    Serial.println("ERROR: PPG Sensor not initialised");
  }
  else
  {
    Serial.println("LOG: PPG Sensor initialised");
  };

  // -------------------------------- K RIP Sensor --------------------------------

  top_coil.intialise();
  bottom_coil.intialise();

  k_rip_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(k_rip_timer, &timerIsr, true);
  timerAlarmWrite(k_rip_timer, 10000, true); // genrerate interrupt every 10ms
  timerAlarmEnable(k_rip_timer);

  // -------------------------------- Mutexes --------------------------------

  // k_rip_mutex = xSemaphoreCreateMutex();
  // ppg_mutex = xSemaphoreCreateMutex();
  // gsr_mutex = xSemaphoreCreateMutex();

  // -------------------------------- WiFi and Database --------------------------------

  initWiFi();
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
  databasePath = "/UsersData/" + uid + "/readings";

  // -------------------------------- Tasks --------------------------------

  xTaskCreate(
    Database_Task, // Task function
    "Database_Task", // Task name
    10000, // Stack size
    NULL, // Parameters
    0, // Priority
    &Database_Task_Handle // Task handle
  );

  xTaskCreate(
    GSR_Task, // Task function
    "GSR_Task", // Task name
    10000, // Stack size
    NULL, // Parameters
    1, // Priority
    &GSR_Task_Handle // Task handle
  );

  xTaskCreate(
    PPG_Task, // Task function
    "PPG_Task", // Task name
    10000, // Stack size
    NULL, // Parameters
    2, // Priority
    &PPG_Task_Handle // Task handle
  );

  vTaskStartScheduler();
}

//----------------------------------------------------------------------------------

void loop(){}

//----------------------------------------------------------------------------------
