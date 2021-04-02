#include "includes/msf_app_ctrl.h"

//Possible commends from the application:
// ' ' - string ending with space sign marks the end of sequence mode
// * - string ending with this sign marks the live mode (controlled from joystick)
// ? - question about current situation, returns '!' when motors are not moving or '~' during operation
// @ - returns ranges for each axis
// # - returns current position of all motors


Msf_driver::Msf_driver() : stepperSlide(AccelStepper::DRIVER, 2, 3), stepperPan(AccelStepper::DRIVER, 9, 10), stepperTilt(AccelStepper::DRIVER, 5, 6)
{
    StepperControl.addStepper(stepperSlide);
    StepperControl.addStepper(stepperPan);
    StepperControl.addStepper(stepperTilt);

    calibSlide = false, calibSlideRight = false, calibPan = false, calibPanRight = false, calibTilt = false, calibTiltRight = false, calibTiltLeft = false;

    //Hardcoded initial possible speed values for live mode
    fastSlide = 2000;
    slowSlide = 600;

    fastPan = 500;
    slowPan = 100;

    fastTilt = 200;
    slowTilt = 60;

    simultaneousMove = true;
    isLiveMode = true;

    slideRange = 0;
    panRange = 0;
    tiltRange = 0;
}

void Msf_driver::run_steppers()
{
    //Need to be called as often as possible while moving
    stepperPan.run();
    stepperSlide.run();
    stepperTilt.run();
}

bool Msf_driver::calib_slide()
{
    //Slide axis
    if (!calibSlide)
    {
        if (digitalRead(endStopRight) != HIGH && !calibSlideRight)
        {
            stepperSlide.setCurrentPosition(0);
            calibSlideRight = true;

            stepperSlide.moveTo(-100000);
        }
        else if (digitalRead(endStopLeft) != HIGH && calibSlideRight)
        {
            slideRange = abs(stepperSlide.currentPosition());
            stepperSlide.setCurrentPosition(0);
            calibSlide = true;
        }
    }
    return calibSlide;
}

bool Msf_driver::calib_pan()
{
    //Pan axis
    if (!calibPan)
    {
        if (digitalRead(endStopPanRight) != HIGH && !calibPanRight)
        {
            stepperPan.setCurrentPosition(0);
            calibPanRight = true;

            stepperPan.moveTo(-100000);
        }
        else if (digitalRead(endStopPanLeft) != HIGH && calibPanRight)
        {
            panRange = abs(stepperPan.currentPosition());
            stepperPan.setCurrentPosition(0);
            calibPan = true;
        }
    }
    return calibPan;
}

bool Msf_driver::calib_tilt()
{
    //Tilt axis
    if (!calibTilt)
    {
        if (digitalRead(endStopTiltRight) != HIGH && !calibTiltRight)
        {
            stepperTilt.setCurrentPosition(0);
            calibTiltRight = true;

            stepperTilt.moveTo(-100000);
        }
        if (digitalRead(endStopTiltLeft) != HIGH && calibTiltRight && !calibTiltLeft)
        {
            tiltRange = abs(stepperTilt.currentPosition());
            stepperTilt.setCurrentPosition(0);
            calibTiltLeft = true;
            stepperTilt.moveTo(tiltRange / 2);
        }
        if (calibTiltLeft && stepperTilt.currentPosition() == tiltRange / 2)
        {
            calibTilt = true;
        }
    }
    return calibTilt;
}

char Msf_driver::msf_ctrl_loop()
{
    char sign;
    if (Serial.available())
    {
        sign = Serial.read();

        if (sign == '*')
        {
            isLiveMode = true;
        }
        else if (sign == '^')
        {
            isLiveMode = false;
        }
        else if (sign == '%') //maxspeed
        {
            setMaxMotorSpeed();
        }
        else if (sign == '&') //minspeed
        {
            setMinMotorSpeed();
        }
        if (isLiveMode)
        {
            liveModeSwitch(sign);
        }
        else
        {
            sequenceModeSwitch(sign);
        }
    }
    //Need to be called as often as possible
    run_steppers();

    return sign;
}

//Parses the key instructions for sequence mode
void Msf_driver::parseCommands(String dataFromCOM)
{
    if (dataFromCOM.indexOf("s") > 0)
        slideCom = dataFromCOM.substring(dataFromCOM.indexOf("s") + 1, dataFromCOM.indexOf("p")).toInt();
    else
        slideCom = 0;
    if (dataFromCOM.indexOf("p") > 0)
        panCom = dataFromCOM.substring(dataFromCOM.indexOf("p") + 1, dataFromCOM.indexOf("t")).toInt();
    else
        panCom = 0;
    if (dataFromCOM.indexOf("t") > 0)
        tiltCom = dataFromCOM.substring(dataFromCOM.indexOf("t") + 1, dataFromCOM.indexOf(" ")).toInt();
    else
        tiltCom = 0;
}

void Msf_driver::simultaneous_steppers(String dataFromCom)
{
    parseCommands(dataFromCom);

    stepperPositions[0] = slideCom;
    stepperPositions[1] = panCom;
    stepperPositions[2] = tiltCom;

    StepperControl.moveTo(stepperPositions);
    StepperControl.runSpeedToPosition();
}

//Method for sequence mode
void Msf_driver::sequenceMode(String dataFromCom)
{
    parseCommands(dataFromCom);

    //Protection from the values extending possible ranges
    if (slideCom > slideRange)
        slideCom = slideRange;
    if (panCom > panRange)
        panCom = panRange;
    if (tiltCom > tiltRange)
        tiltCom = tiltRange;

    if (slideCom < 0)
        slideCom = 0;
    if (panCom < 0)
        panCom = 0;
    if (tiltCom < 0)
        tiltCom = 0;

    // stepperSlide.setMaxSpeed(600);
    // stepperPan.setMaxSpeed(300);
    // stepperTilt.setMaxSpeed(300);

    if (simultaneousMove)
    {
        stepperPositions[0] = slideCom;
        stepperPositions[1] = panCom;
        stepperPositions[2] = tiltCom;

        StepperControl.moveTo(stepperPositions);
        StepperControl.runSpeedToPosition();
    }
    else
    {
        stepperSlide.moveTo(slideCom);
        stepperPan.moveTo(panCom);
        stepperTilt.moveTo(tiltCom);
    }
}

void Msf_driver::liveModeSwitch(char sign)
{
    switch (sign)
    {
    case 'a':
        stepperSlide.moveTo(slideRange);
        fullCommand = "";
        break;
    case 'b':
        stepperSlide.moveTo(0);
        fullCommand = "";
        break;
    case 'c':
        stepperPan.moveTo(panRange);
        fullCommand = "";
        break;
    case 'd':
        stepperPan.moveTo(0);
        fullCommand = "";
        break;
    case 'e':
        stepperTilt.moveTo(tiltRange);
        fullCommand = "";
        break;
    case 'f':
        stepperTilt.moveTo(0);
        fullCommand = "";
        break;
    case '0':
        stepperSlide.stop();
        fullCommand = "";
        break;
    case '1':
        stepperSlide.setMaxSpeed(slowSlide);
        fullCommand = "";
        break;
    case '2':
        stepperSlide.setMaxSpeed(slowSlide + (fastSlide - slowSlide) * 0.25); // 2/5 of the max speed
        fullCommand = "";
        break;
    case '3':
        stepperSlide.setMaxSpeed(slowSlide + (fastSlide - slowSlide) * 0.5); //medium speed
        fullCommand = "";
        break;
    case '4':
        stepperSlide.setMaxSpeed(slowSlide + (fastSlide - slowSlide) * 0.75); // 4/5 of the max speed
        fullCommand = "";
        break;
    case '5':
        stepperSlide.setMaxSpeed(fastSlide);
        fullCommand = "";
        break;
    case '<':
        stepperPan.stop();
        fullCommand = "";
        break;
    case '6':
        stepperPan.setMaxSpeed(slowPan);
        fullCommand = "";
        break;
    case '7':
        stepperPan.setMaxSpeed(slowPan + (fastPan - slowPan) * 0.25);
        fullCommand = "";
        break;
    case '8':
        stepperPan.setMaxSpeed(slowPan + (fastPan - slowPan) * 0.5);
        fullCommand = "";
        break;
    case '9':
        stepperPan.setMaxSpeed(slowPan + (fastPan - slowPan) * 0.75);
        fullCommand = "";
        break;
    case 'A':
        stepperPan.setMaxSpeed(fastPan);
        fullCommand = "";
        break;
    case '>':
        stepperTilt.stop();
        fullCommand = "";
        break;
    case 'B':
        stepperTilt.setMaxSpeed(slowTilt);
        fullCommand = "";
        break;
    case 'C':
        stepperTilt.setMaxSpeed(slowTilt + (fastTilt - slowTilt) * 0.25);
        fullCommand = "";
        break;
    case 'D':
        stepperTilt.setMaxSpeed(slowTilt + (fastTilt - slowTilt) * 0.5);
        fullCommand = "";
        break;
    case 'E':
        stepperTilt.setMaxSpeed(slowTilt + (fastTilt - slowTilt) * 0.75);
        fullCommand = "";
        break;
    case 'F':
        stepperTilt.setMaxSpeed(fastTilt);
        fullCommand = "";
        break;
    case '*':
        Serial.println(F("Live mode set!"));
        break;
    case '^':
        isLiveMode = false;
        break;
    case '?':
        if (!stepperSlide.isRunning() && !stepperPan.isRunning() && !stepperSlide.isRunning())
        {
            Serial.println(F("! ")); //when all stepper motors are in idle
        }
        else
        {
            Serial.println(F("~ ")); //while stepper motors are moving
        }
        fullCommand = "";
        break;
    case '@':
        sendMotorRanges();
        fullCommand = "";
        break;
    case '#':
        sendCurrentPositions();
        fullCommand = "";
        break;
    case '%': //maxspeed
        setMaxMotorSpeed();
        fullCommand = "";
        break;
    case '&': //minspeed
        setMinMotorSpeed();
        fullCommand = "";
        break;
    default:
        Serial.println(F("Invalid command"));
        break;
    }
}

void Msf_driver::sequenceModeSwitch(char sign)
{
    switch (sign)
    {
    case ' ':
        stepperSlide.setMaxSpeed(fastSlide);
        stepperPan.setMaxSpeed(fastPan);
        stepperTilt.setMaxSpeed(fastTilt);

        //deals with the full command and divides commands to the specific stepper motors
        sequenceMode(fullCommand);
        fullCommand = "";
        break;
    case '?':
        if (!stepperSlide.isRunning() && !stepperPan.isRunning() && !stepperSlide.isRunning())
        {
            Serial.println(F("! ")); //when all stepper motors are in idle
        }
        else
        {
            Serial.println(F("~ ")); //while stepper motors are moving
        }
        fullCommand = "";
        break;
    case '@':
        sendMotorRanges();
        fullCommand = "";
        break;
    case '#':
        sendCurrentPositions();
        fullCommand = "";
        break;
    case '\r': //ignore CR
        break;
    case '*':
        isLiveMode = true;
        break;
    case '^':
        Serial.println(F("Sequence mode set!"));
        break;
    default:
        fullCommand += sign;
        break;
    }
}

void Msf_driver::setMaxMotorSpeed()
{
    while (Serial.available())
    {
        char sign = Serial.read();
        switch (sign)
        {
        case ' ':
            fullCommand += sign;
            parseCommands(fullCommand);
            fullCommand = "";
            break;
        case '\r': //ignore CR
            break;
        default:
            fullCommand += sign;
            break;
        }
        if (fullCommand == "")
        {
            if (slideCom != 0)
                fastSlide = slideCom;
            if (panCom != 0)
                fastPan = panCom;
            if (tiltCom != 0)
                fastTilt = tiltCom;
        }
        if (Serial.read() == ' ')
            break;
    }
}

void Msf_driver::setMinMotorSpeed()
{
    while (Serial.available())
    {
        char sign = Serial.read();
        switch (sign)
        {
        case ' ':
            fullCommand += sign;
            parseCommands(fullCommand);
            fullCommand = "";
            break;
        case '\r': //ignore CR
            break;
        default:
            fullCommand += sign;
            break;
        }
        if (fullCommand == "")
        {
            if (slowSlide != 0)
                slowSlide = slideCom;
            if (fastPan != 0)
                slowPan = panCom;
            if (fastTilt != 0)
                slowTilt = tiltCom;
        }
        if (Serial.read() == ' ')
            break;
    }
}

//Sends ranges of each axis to the appliaction
void Msf_driver::sendMotorRanges()
{
    char temp[30];
    snprintf(temp, 30, "@s%dp%dt%d ", slideRange, panRange, tiltRange);
    Serial.println(temp);
}

//Sends current position of motors
void Msf_driver::sendCurrentPositions()
{
    char temp[30];
    int slidePos = stepperSlide.currentPosition();
    int panPos = stepperPan.currentPosition();
    int tiltPos = stepperTilt.currentPosition();
    snprintf(temp, 30, "#s%dp%dt%d ", slidePos, panPos, tiltPos);
    Serial.println(temp);
}

void Msf_driver::init()
{
    pinMode(endStopRight, INPUT); //for the right endstop
    pinMode(endStopLeft, INPUT);  //for the left endstop
    pinMode(endStopTiltLeft, INPUT);
    pinMode(endStopTiltRight, INPUT);
    pinMode(endStopPanRight, INPUT);
    pinMode(endStopPanLeft, INPUT);

    stepperSlide.setMaxSpeed(2200);
    stepperSlide.setAcceleration(slowSlide + (fastSlide - slowSlide) * 0.2);
    stepperSlide.moveTo(100000);

    stepperPan.setMaxSpeed(800);
    stepperPan.setAcceleration(slowPan + (fastPan - slowPan) * 0.2);
    stepperPan.moveTo(100000);

    stepperTilt.setMaxSpeed(300);
    stepperTilt.setAcceleration(slowTilt + (fastTilt - slowTilt) * 0.2);
    stepperTilt.moveTo(100000);
}