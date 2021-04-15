#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

const byte okSign[8] = {
    0B00000,
    0B00001,
    0B00001,
    0B00010,
    0B10010,
    0B01100,
    0B00100,
    0B00000,
};

class Lcd_driver
{
public:
    Lcd_driver(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows);
    void begin_display();
    void clear_lcd();
    void calib_info();
    void df_mode_info(String dfVersion);
    void waiting_info();
    void live_mode_info();
    void sequence_mode_info();
    void mode_display(char mode);
    void init_pos_set();
    bool slide_calib_ok(bool calibrated);
    bool pan_calib_ok(bool calibrated);
    bool tilt_calib_ok(bool calibrated);


private:
    LiquidCrystal_I2C lcd;
};
#endif