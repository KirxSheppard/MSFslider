#ifndef MSF_APP_CTRL_H
#define MSF_APP_CTRL_H

/*
Library - AccelStepper by Mike McCauley:
http://www.airspayce.com/mikem/arduino/AccelStepper/index.html
*/
#include <Arduino.h>
#include <AccelStepper.h>
#include <MultiStepper.h>

#define NUM_OF_STEPPERS  3

//defines modes
#define APP_MODE 94 // has to be send to access classic app mode
#define DRAGON_MODE 95
#define CMD_CALIB 64 //@

/*
 * Command codes from user for Live and Sequence Mode
 */
#define CMD_SLIDE_R 97
#define CMD_SLIDE_B 98
#define CMD_PAN_R 99
#define CMD_PAN_B 100
#define CMD_TILT_R 101
#define CMD_TILT_B 102

//pins for endstops
#define endStopRight 4      //dolly right
#define endStopLeft 7       //dolly left
#define endStopPanLeft 15   //pan axis (Arduino A1)
#define endStopPanRight 14  //(Arduino A0)
#define endStopTiltLeft 17  //tilt axis (Arduino A3)
#define endStopTiltRight 16 //(Arduino A2)

class Msf_driver
{
public:
    //mode, step, dir
    Msf_driver(); 
    bool calib_slide();
    bool calib_pan();
    bool calib_tilt();
    char msf_ctrl_loop();
    void parseCommands(String dataFromCOM);
    void sequenceMode(String dataFromCom);
    void liveModeSwitch(char sign);
    void sequenceModeSwitch(char sign);
    void setMinMotorSpeed();
    void setMaxMotorSpeed();
    void sendMotorRanges();
    void sendCurrentPositions();
    void clearLCD();
    void init();
    void simultaneous_steppers(String dataFromCom);
    void run_steppers();
    
    void acceleration_sequence();

private:
    AccelStepper stepperSlide;
    AccelStepper stepperPan;
    AccelStepper stepperTilt;

    MultiStepper SteppersControl;

    // Msf_Multi SteppersControl;

    long stepperPositions[3];

    bool calibSlide, calibSlideRight, calibPan, calibPanRight, calibTilt, calibTiltRight, calibTiltLeft;

    int fastSlide, slowSlide, fastPan, slowPan, fastTilt, slowTilt;

    String fullCommand;
    long slideCom, panCom, tiltCom;

    bool simultaneousMove;
    bool isLiveMode;

    int slideRange, panRange, tiltRange;
};

#endif