#include "../include/ppg_sensor.hpp"

//----------------------------------------------------------------------------------

PpgSensor::PpgSensor(byte ledBrightness, byte sampleAverage, byte ledMode, byte sampleRate, int pulseWidth, int adcRange)
{
    this->ledBrightness = ledBrightness;
    this->sampleAverage = sampleAverage;
    this->ledMode = ledMode;
    this->sampleRate = sampleRate;
    this->pulseWidth = pulseWidth;
    this->adcRange = adcRange;
}

//----------------------------------------------------------------------------------

bool PpgSensor::intialise()
{
    // Initialize sensor
    // Use default I2C port, 400kHz speed
    if (!this->particleSensor.begin(Wire, I2C_SPEED_FAST))
    {
        return false;
    }

    // Setup to sense the MAX30105
    this->particleSensor.setup(this->ledBrightness, this->sampleAverage, this->ledMode, this->sampleRate, this->pulseWidth, this->adcRange);

    return true;

}

//----------------------------------------------------------------------------------

void PpgSensor::read()
{
    red_value = particleSensor.getRed();
    ir_value = particleSensor.getIR();
}

//----------------------------------------------------------------------------------
