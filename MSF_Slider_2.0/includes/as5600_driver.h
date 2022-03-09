#ifndef AS5600_DRIVER_H
#define AS5600_DRIVER_H
#include <AS5600.h>
#include <math.h>

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
    int rangeCounter(int microsteps);

private:
    long enc = 0;
    long lenc = 0;

    AS5600 encoder;

};



#endif
