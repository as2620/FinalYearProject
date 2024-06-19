//-------------------------------------------------------------------------------------------------

// Readout Code for Sending Two Simulataneous Sensors Connected to an ESP32.
// Sensors consit of a GSR, PPG and two K-RIP coils.

//-------------------------------------------------------------------------------------------------

#include "../include/main.hpp"

//----------------------------------------------------------------------------------

void IRAM_ATTR timerIsr()
{
  // Read the K-RIP sensors when the timer interrupt is triggered
  top_coil.read();
  bottom_coil.read();
}

//----------------------------------------------------------------------------------

void writeDataToSerial()
{
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
}

//----------------------------------------------------------------------------------

void ppgTask(void *pvParameters)
{
  Serial.print("LOG: Task 1 running on core ");
  Serial.println(xPortGetCoreID());

  while (1)
  {
    ppg_sensor.read();
  }
}

//----------------------------------------------------------------------------------

void gsrTask(void *pvParameters)
{
  Serial.print("LOG: Task 2 running on core ");
  Serial.println(xPortGetCoreID());

  while (1)
  {
    if (GSR_LOOP_COUNTER < GSR_LOOP_COUNTER_LIMIT)
    {
      gsr_sensor.read();
      GSR_LOOP_COUNTER++;
    }
    else
    {
      gsr_sensor.calculateAverageValue();
      GSR_LOOP_COUNTER = 0;
    }

    vTaskDelay(pdMS_TO_TICKS(GSR_LOOP_DELAY));
  }
}

//----------------------------------------------------------------------------------

void serialTask(void *pvParameters)
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
  timerAlarmWrite(k_rip_timer, 10000, true); /
  timerAlarmEnable(k_rip_timer);

  xTaskCreate(
    serialTask,                                    // Task function
    "serialTask",                                  // Task name
    10000,                                         // Stack size
    NULL,                                          // Parameters
    0,                                             // Priority
    &Serial_Task_Handle                            // Task handle
  );

  xTaskCreate(
    gsrTask,                                       // Task function
    "gsrTask",                                     // Task name
    10000,                                         // Stack size
    NULL,                                          // Parameters
    1,                                             // Priority
    &GSR_Task_Handle                               // Task handle
  );

  xTaskCreate(
    ppgTask,                                       // Task function
    "ppgTask",                                     // Task name
    10000,                                         // Stack size
    NULL,                                          // Parameters
    2,                                             // Priority
    &PPG_Task_Handle                               // Task handle
  );

  vTaskStartScheduler();
}

//----------------------------------------------------------------------------------

void loop(){}

//----------------------------------------------------------------------------------
