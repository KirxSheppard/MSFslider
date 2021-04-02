#include "includes/tmc5160_driver.h"

void SetupTmc5160::setup(uint8_t toff, uint8_t blank_time, uint16_t rms_current, uint16_t microsteps, bool en_pwm_mode)
{
    // SPI.begin();

  driver.begin(); //  SPI: Init CS pins and possible SW SPI pins
  delay(10);
  driver.toff(toff); // Enables driver in software
  driver.blank_time(blank_time);
  driver.rms_current(rms_current); // Set motor RMS current
  driver.microsteps(microsteps);    // Set microsteps to 1/16th
  driver.en_pwm_mode(en_pwm_mode);
}