#include "Arduino.h"
#include "stdio.h"                                                        
#include "driver/pcnt.h"                                                  
#include "soc/pcnt_struct.h"

//----------------------------------------------------------------------------------

#define PCNT_COUNT_UNIT_0 PCNT_UNIT_0             
#define PCNT_COUNT_CHANNEL PCNT_CHANNEL_0          

//----------------------------------------------------------------------------------

#define PCNT_INPUT_SIG_IO_0 GPIO_NUM_34  
#define PCNT_INPUT_CTRL_IO_0 GPIO_NUM_35

//----------------------------------------------------------------------------------

// Note that the PCB multiplexes between the two coils, so we can only record one 
// coil at a time. The select line to the multiplexer is connected to GPIO PIN 27.
#define SELECT_LINE_PIN GPIO_NUM_27
bool select_line_state = false;

//----------------------------------------------------------------------------------

#define ON_BOARD_LED GPIO_NUM_2          

//----------------------------------------------------------------------------------

void intialisePulseCounter(void)
{

}

//----------------------------------------------------------------------------------

float readPulseCounter(void)
{
    return 0.0;
}

//----------------------------------------------------------------------------------

void printPulseCounter(void)
{
    float coil_value = readPulseCounter(); 

    if (select_line_state == false)
    {
        Serial.print("Top Coil Frequency: ");
        Serial.print(coil_value);
        Serial.print("Bottom Coil Frequency: ");
        Serial.print("-");
    }
    else
    {
        Serial.print("Top Coil Frequency: ");
        Serial.print("-");
        Serial.print("Bottom Coil Frequency: ");
        Serial.print(coil_value);
    }
}

//----------------------------------------------------------------------------------

void setup(void)
{
    Serial.begin(115200);

    pinMode(ON_BOARD_LED, OUTPUT);
    pinMode(SELECT_LINE_PIN, OUTPUT);

    digitalWrite(SELECT_LINE_PIN, select_line_state);

    intialisePulseCounter();
}

//----------------------------------------------------------------------------------

void loop(void)
{
    if (select_line_state)
    {
        digitalWrite(ON_BOARD_LED, HIGH);
    }
    else
    {
        digitalWrite(ON_BOARD_LED, LOW);
    }

    printPulseCounter();

    select_line_state = !select_line_state;
    digitalWrite(SELECT_LINE_PIN, select_line_state);

    delay(100); // in milliseconds 
}   

//----------------------------------------------------------------------------------

