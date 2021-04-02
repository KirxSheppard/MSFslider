/*
MSF Slider Copyright (C) 2020  Kamil Janko
Website: https://github.com/KirxSheppard
Contact: kamil.janko@megaspacefighter.com
Version: 2.0

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "includes/dfmoco.h"
#include "includes/tmc5160_driver.h"
#include "includes/msf_app_ctrl.h"

SetupTmc5160 spiDriver(8); //cs pin
Msf_driver msf;

/*
 * setup() gets called once, at the start of the program.
 */
void setup()
{
    // lcd.begin();
    // delay(100);

    // setup serial connection
    Serial.begin(57600);

    // setupPanDriver();
    spiDriver.setup(4, 24, 1900, 32, true);

    //initial message
    sendMessage(MSG_HI, 0);

    //waits for the connection from mobile app or Dragonframe
    //   clearLCD();
    // lcd.print(F("Waiting..."));
    delay(1000);

    char sign;
    bool calibrated = false;
    while (!calibrated)
    {
        delay(100);
        if (SERIAL_DEVICE.available())
        {
            delay(3);
            sign = SERIAL_DEVICE.read();

            if (sign == CMD_CALIB) // sent from mobile app - starts from classic app mode
            {
                msf.gpio_init();
                msf.msf_init_calib();
                if_msf_ctrl = true;
                calibrated = true;
            }
            else if (sign == CMD_HI) // dragonframe connected, start from its mode
            {
                setup_dfmoco();
                // clearLCD();
                // lcd.print(F("Dragonframe Mode"));
                // lcd.setCursor(0, 1);
                // lcd.print(F("DFMoco "));
                // lcd.print(DFMOCO_VERSION_STRING);
                if_msf_ctrl = false;
                calibrated = true;
            }
        }
    }
}

void loop()
{
    if (if_msf_ctrl)
    {
        msf.msf_ctrl_loop();
    }
    else
    {
        dfmoco_loop();
    }
}
