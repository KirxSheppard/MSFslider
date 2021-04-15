#include "includes/lcd_driver.h"

Lcd_driver::Lcd_driver(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows) : lcd(lcd_addr, lcd_cols, lcd_rows)
{  
}

void Lcd_driver::clear_lcd()
{
    lcd.clear();
    lcd.home();
}

void Lcd_driver::calib_info()
{
    clear_lcd();
    lcd.print(F("Calibrating..."));
    lcd.setCursor(0, 1);
    lcd.print(F("S:x P:x T:x"));
}

void Lcd_driver::df_mode_info(String dfVersion)
{
    clear_lcd();
    lcd.print(F("Dragonframe Mode"));
    lcd.setCursor(0, 1);
    lcd.print(F("DFMoco "));
    lcd.print(dfVersion);
}

void Lcd_driver::begin_display()
{
    lcd.begin();
    delay(100);
    lcd.createChar(0, okSign);
}

void Lcd_driver::waiting_info()
{
    clear_lcd();
    lcd.print(F("Waiting..."));
}

void Lcd_driver::live_mode_info()
{
    clear_lcd();
    lcd.print(F("---Live Mode---"));
}

void Lcd_driver::sequence_mode_info()
{
    clear_lcd();
    lcd.print(F("-Sequence Mode-"));
}

void Lcd_driver::mode_display(char mode)
{
    if (mode == '*')
    {
        live_mode_info();
    }
    else if (mode == '^')
    {
        sequence_mode_info();
    }
}

void Lcd_driver::init_pos_set()
{
    clear_lcd();
    lcd.print(F("Initial position"));
    lcd.setCursor(0, 1);
    lcd.print(F("set!"));
    delay(3000);
    clear_lcd();
    lcd.print(F("---Live Mode---")); //live mode is default state
}

bool Lcd_driver::slide_calib_ok(bool calibrated)
{
    if (calibrated)
    {
        lcd.setCursor(2, 1);
        lcd.write(byte(0));
    }
    return calibrated;
}

bool Lcd_driver::pan_calib_ok(bool calibrated)
{
    if (calibrated)
    {
        lcd.setCursor(6, 1);
        lcd.write(byte(0));
    }
    return calibrated;
}

bool Lcd_driver::tilt_calib_ok(bool calibrated)
{
    if (calibrated)
    {
        lcd.setCursor(10, 1);
        lcd.write(byte(0));
    }
    return calibrated;
}