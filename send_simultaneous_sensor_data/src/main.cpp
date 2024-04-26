
#include "../include/main.hpp"
#include <sys/time.h>
#include <ctime>

//----------------------------------------------------------------------------------

void IRAM_ATTR timerIsr()
{
    top_coil.read();
    bottom_coil.read();
}

//----------------------------------------------------------------------------------

void PPG_Task(void *pvParameters)
{
  while (1)
  {
    ppg_sensor.read();
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

//----------------------------------------------------------------------------------

void GSR_Task(void *pvParameters)
{
  while (1)
  {
    if (GSR_LOOP_COUNTER < GSR_LOOP_COUNTER_LIMIT)
    {
      gsr_sensor.read();

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

void initWiFi() 
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

// Function to generate timestamp with microsecond resolution
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
  while (1)
  {   
      String timestamp_string = getTimestamp();
      
      parentPath = databasePath + "/" + timestamp_string;
      
      json.set(chestCoilPath.c_str(), String(top_coil.frequency));
      json.set(abdomenCoilPath.c_str(), String(bottom_coil.frequency));
      json.set(gsrPath.c_str(), String(gsr_sensor.averaged_gsr_value));
      json.set(ppgRedPath.c_str(), String(ppg_sensor.red_value));
      json.set(ppgIrPath.c_str(), String(ppg_sensor.ir_value));
      json.set(timePath, timestamp_string);

      // Send data to database
      Firebase.RTDB.setJSONAsync(&fbdo, parentPath.c_str(), &json);

      vTaskDelay(pdMS_TO_TICKS(10));
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
  timerAlarmWrite(k_rip_timer, (200 * 1000), true); // genrerate interrupt every 200ms
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
  databasePath = "/UsersData/" + uid + "/shirt_data";

  // -------------------------------- Tasks --------------------------------

  xTaskCreate(
    PPG_Task, // Task function
    "PPG_Task", // Task name
    10000, // Stack size
    NULL, // Parameters
    1, // Priority
    &PPG_Task_Handle // Task handle
  );

  xTaskCreate(
    GSR_Task, // Task function
    "GSR_Task", // Task name
    10000, // Stack size
    NULL, // Parameters
    2, // Priority
    &GSR_Task_Handle // Task handle
  );
  
  xTaskCreate(
    Database_Task, // Task function
    "Database_Task", // Task name
    10000, // Stack size
    NULL, // Parameters
    3, // Priority
    &Database_Task_Handle // Task handle
  );

  vTaskStartScheduler();
}

//----------------------------------------------------------------------------------

void loop(){}

//----------------------------------------------------------------------------------
