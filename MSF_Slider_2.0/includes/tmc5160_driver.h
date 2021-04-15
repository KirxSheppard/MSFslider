#ifndef TMC5160_DRIVER_H
#define TMC5160_DRIVER_H

#include <Arduino.h>
#include <TMCStepper.h>

//SPI config
// #define SW_MOSI 11 // Software Master Out Slave In (MOSI)
// #define SW_MISO 12 // Software Master In Slave Out (MISO)
// #define SW_SCK 13  // Software Slave Clock (SCK)
// #define R_SENSE 0.075f

class SetupTmc5160
{
public:
    SetupTmc5160(uint16_t pinCS, float RS, uint16_t pinMOSI, uint16_t pinMISO, uint16_t pinSCK);
    void setup_driver(uint8_t toff, uint8_t blank_time, uint16_t rms_current, uint16_t microsteps, bool en_pwm_mode);

private:
    TMC5160Stepper driver;
};
#endif