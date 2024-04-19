
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

void Serial_Task(void *pvParameters)
{
  Serial.print("Task 3 running on core ");
  Serial.println(xPortGetCoreID());

  while (1)
  {    
    writeDataToSerial();   
    vTaskDelay(pdMS_TO_TICKS(SERIAL_LOOP_DELAY));
  }
}

//----------------------------------------------------------------------------------

void setup()
{
  Serial.begin(115200);
  Serial.println("Simultanous K-RIP, GSR and PPG Sensor Data Acquisition");

  // PPG Sensor
  if (!ppg_sensor.intialise())
  {
    Serial.println("ERROR: PPG Sensor not initialised");
  }
  else
  {
    Serial.println("LOG: PPG Sensor initialised");
  };

  // K RIP Sensor
  top_coil.intialise();
  bottom_coil.intialise();

  k_rip_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(k_rip_timer, &timerIsr, true);
  timerAlarmWrite(k_rip_timer, 10000, true); // genrerate interrupt every 10ms
  timerAlarmEnable(k_rip_timer);

  // k_rip_mutex = xSemaphoreCreateMutex();
  // ppg_mutex = xSemaphoreCreateMutex();
  // gsr_mutex = xSemaphoreCreateMutex();

  xTaskCreate(
    Serial_Task, // Task function
    "Serial_Task", // Task name
    10000, // Stack size
    NULL, // Parameters
    0, // Priority
    &Serial_Task_Handle // Task handle
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
