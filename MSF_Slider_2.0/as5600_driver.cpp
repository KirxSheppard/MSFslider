#include "includes/as5600_driver.h"

bool As5600_driver::encoder_loop()
{

    enc = encoder.getPosition();

    return true;
}

int As5600_driver::rangeCounter(int microsteps)
{
    float exsteps = enc * (200 * microsteps) / 4096;
    int steps = round(exsteps);

    return steps;
}
