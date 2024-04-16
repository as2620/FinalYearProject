#include "../include/gsr_sensor.hpp"

//----------------------------------------------------------------------------------

GsrSensor::GsrSensor(uint8_t gsr_pin)
{
    this->gsr_pin = gsr_pin;

}

//----------------------------------------------------------------------------------

void GsrSensor::read()
{
    this->sensorValue = analogRead(this->gsr_pin);
    this->sum += this->sensorValue;
    this->read_count++;
}

//----------------------------------------------------------------------------------

void GsrSensor::calculateAverageValue()
{
    this->averaged_gsr_value = this->sum/this->read_count;

    this->sum = 0;
    this->read_count = 0;
}

//----------------------------------------------------------------------------------
