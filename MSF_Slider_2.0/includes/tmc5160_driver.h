#ifndef TMC5160_DRIVER_H
#define TMC5160_DRIVER_H

#include <Arduino.h>
#include <TMCStepper.h>

class SetupTmc5160
{
public:
    SetupTmc5160(uint16_t pinCS, float RS, uint16_t pinMOSI, uint16_t pinMISO, uint16_t pinSCK);
    void setup_driver(uint8_t toff, uint8_t blank_time, uint16_t rms_current, uint16_t microsteps, bool en_pwm_mode);

private:
    TMC5160Stepper driver;
};
#endif