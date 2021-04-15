/*
MSF Slider Copyright (C) 2020  Kamil Janko
Website: https://github.com/KirxSheppard
         https://megaspacefighter.com
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
#include "includes/lcd_driver.h"

SetupTmc5160 spiDriver(8, 0.075f, 11, 12, 13); //cs pin
Msf_driver msf;
Lcd_driver lcd(0x27, 2, 16);

bool if_msf_ctrl = false;
#define SERIAL_DEVICE Serial

/*
 * setup() gets called once, at the start of the program.
 */
void setup()
{
    // setup serial connection
    Serial.begin(57600);

    // setupPanDriver();
    spiDriver.setup_driver(4, 24, 1900, 32, true);

    lcd.begin_display();
    lcd.waiting_info(); //waits for the connection from mobile app or Dragonframe
    delay(1000);

    //initial message
    sendMessage(MSG_HI, 0);

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
                lcd.calib_info();
                msf.init();
                msf_init_calib();
                lcd.init_pos_set();
                if_msf_ctrl = true;
                calibrated = true;
            }
            else if (sign == /*CMD_HI*/ 10) // dragonframe connected, start from its mode
            {
                lcd.df_mode_info(DFMOCO_VERSION_STRING);
                setup_dfmoco();               
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
        lcd.mode_display(msf.msf_ctrl_loop());
    }
    else
    {
        dfmoco_loop();
    }
}

void msf_init_calib()
{
    bool slideCalib = false, panCalib = false, tiltCalib = false;

    while(!(slideCalib && panCalib && tiltCalib))
    {
        if(!slideCalib)
        {
            slideCalib = lcd.slide_calib_ok(msf.calib_slide());
        }
        if(!panCalib)
        {
            panCalib = lcd.pan_calib_ok(msf.calib_pan());
        }
        if(!tiltCalib)
        {
            tiltCalib = lcd.tilt_calib_ok(msf.calib_tilt());
        }
        msf.run_steppers();
    }
    msf.sendMotorRanges();
}