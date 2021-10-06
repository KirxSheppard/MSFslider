#ifndef AS5600_DRIVER_H
#define AS5600_DRIVER_H
#include <AS5600.h>
#include <Wire.h>

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
  #define SERIAL SerialUSB
  #define SYS_VOL   3.3
#else
  #define SERIAL Serial
  #define SYS_VOL   5
#endif

class As5600_driver
{
public:
    bool encoder_loop();
    float convertRawAngleToDegrees(word newAngle);
    int rangeCounter(int microsteps);

private:
    int angle;
    int lang;
    int slideStepsPrev = 0;
    int slideStepsCurr = 0;

    AMS_5600 ams5600;

};



#endif
