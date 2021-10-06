#include "includes/as5600_driver.h"

bool As5600_driver::encoder_loop()
{
    Wire.begin();

    if (ams5600.detectMagnet() == 0)
    {
        while (1)
        {
            if (ams5600.detectMagnet() == 1)
            {
                // SERIAL.print("Current Magnitude: ");
                // SERIAL.println(ams5600.getMagnitude());
                break;
            }
            else
            {
                return false;
                // SERIAL.println("Can not detect magnet");
            }
            delay(1000);
        }
    }
    return true;
}

float As5600_driver::convertRawAngleToDegrees(word newAngle)
{
    /* Raw data reports 0 - 4095 segments, which is 0.087 of a degree */
    float retVal = newAngle * 0.087;
    int ang = retVal;
    return ang;
}

int As5600_driver::rangeCounter(int microsteps)
{
    //0-4095 encoder range, 200steps * microsteps number (8, 16, 32, 64, 128, 256)
    int stepperMoved = ((200 * microsteps) / 360) * convertRawAngleToDegrees(ams5600.getRawAngle());
    

}