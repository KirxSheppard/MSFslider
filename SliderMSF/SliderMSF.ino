/*
Copyright (C) 2020  Kamil Janko
Website: https://github.com/KirxSheppard
Contact: kamil.janko@megaspacefighter.com
Version: 1.2

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

//Library - AccelStepper by Mike McCauley:
//http://www.airspayce.com/mikem/arduino/AccelStepper/index.html

#include <AccelStepper.h>
#include <MultiStepper.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <HighPowerStepperDriver.h>
#include <SPI.h>

//Defines checkbox sign on the LCD display
byte okSign[8] = {
    0B00000,
    0B00001,
    0B00001,
    0B00010,
    0B10010,
    0B01100,
    0B00100,
    0B00000,
};

//Hardcoded possible spped values for live mode
int fastSlide = 2000;
int mediumSlide = 1300;
int slowSlide = 600;

int fastPan = 500;
int mediumPan = 300;
int slowPan = 100;

int fastTilt = 200;
int mediumTilt = 120;
int slowTilt = 60;

char temp[30];
String fullCommand;
int slideCom, panCom, tiltCom;

bool calibSlide, calibSlideRight, calibPan, calibPanRight, calibTilt, calibTiltRight, calibTiltLeft = false;
bool steppersInProgress = false;
bool isLiveMode = true;

int slideRange, panRange, tiltRange = 0;
int slidePrevCom, panPrevCom, tiltPrevCom = 0;

//pins for endstops
#define endStopRight 4      //dolly right
#define endStopLeft 7       //dolly left
#define endStopPanLeft 15   //pan axis (Arduino A1)
#define endStopPanRight 14  //(Arduino A0)
#define endStopTiltLeft 17  //tilt axis (Arduino A3)
#define endStopTiltRight 16 //(Arduino A2)

//chip select pin for 36v4 driver prrogrammed with SPI
#define scs 8               

//array for multistepper library simultaneous stepper movement
long stepperPositions[3];

//The first value defines the way to control the motor, in this case, the last two values corespond to the driver, the remaining two for the steps and direction
AccelStepper stepperSlide(AccelStepper::DRIVER, 2, 3); //step, dir
AccelStepper stepperPan(AccelStepper::DRIVER, 9, 10);
AccelStepper stepperTilt(AccelStepper::DRIVER, 5, 6);

LiquidCrystal_I2C lcd(0x27, 16, 2);

HighPowerStepperDriver tiltDriver;
MultiStepper StepperControl;

void setup()
{

  Serial.begin(115200);
  setupPanDriver();

  pinMode(endStopRight, INPUT); //for the right endstop
  pinMode(endStopLeft, INPUT);  //for the left endstop
  pinMode(endStopTiltLeft, INPUT);
  pinMode(endStopTiltRight, INPUT);
  pinMode(endStopPanRight, INPUT);
  pinMode(endStopPanLeft, INPUT);

  lcd.begin();
  lcd.createChar(0, okSign);
  Serial.println("Connected! ");

  //sets all axes to home position
  clearLCD();
  lcd.print("Waiting...");
  char sign;
  while (1)
  {
    delay(100);
    if (Serial.available())
    {
      delay(3);
      sign = Serial.read();
    }
    if (sign == '@')
    {
      clearLCD();
      initCalibration();
      break;
    }
  }

  clearLCD();
  lcd.print("Initial position");
  lcd.setCursor(0, 1);
  lcd.print("set!");
  delay(3000);

  clearLCD();
  lcd.print("Waiting..."); //as it starts waiting for the command
}

void setupPanDriver()
{
  SPI.begin();
  tiltDriver.setChipSelectPin(scs);

  delay(1);

  // Resets the driver to its default settings and clears latched status
  // conditions.
  tiltDriver.resetSettings();
  tiltDriver.clearStatus();

  // Selects auto mixed decay.  TI's DRV8711 documentation recommends this mode
  // for most applications, and the creator of the library finds that it usually works well.
  tiltDriver.setDecayMode(HPSDDecayMode::AutoMixed);

  // Sets the current limit.
  tiltDriver.setCurrentMilliamps36v4(2500);

  // Sets the number of microsteps that correspond to one full step.
  tiltDriver.setStepMode(HPSDStepMode::MicroStep8);

  // Enables the motor outputs.
  tiltDriver.enableDriver();
}

// Method called during setup
// Callibrates each stepper motor to home position
void initCalibration()
{
  lcd.print("Calibrating...");
  lcd.setCursor(0, 1);
  lcd.print("S:x P:x T:x");

  stepperSlide.setMaxSpeed(2200);
  stepperSlide.setAcceleration(600);
  stepperSlide.moveTo(100000);

  stepperPan.setMaxSpeed(400);
  stepperPan.setAcceleration(200);
  stepperPan.moveTo(100000);

  stepperTilt.setMaxSpeed(200);
  stepperTilt.setAcceleration(60);
  stepperTilt.moveTo(100000);

  StepperControl.addStepper(stepperSlide);
  StepperControl.addStepper(stepperPan);
  StepperControl.addStepper(stepperTilt);

  //Loops until all axes are calibrated
  while (!calibSlide || !calibPan || !calibTilt)
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
        lcd.setCursor(2, 1);
        lcd.write(byte(0));
      }
    }

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
        lcd.setCursor(6, 1);
        lcd.write(byte(0));
      }
    }

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
        lcd.setCursor(10, 1);
        lcd.write(byte(0));
      }
    }

    //Need to be called as often as possible
    stepperPan.run();
    stepperSlide.run();
    stepperTilt.run();
  }

  clearLCD();
  sendMotorRanges(); //answers to the application about motors' ranges

  stepperSlide.setMaxSpeed(slowSlide);
  stepperPan.setMaxSpeed(slowPan);
  stepperTilt.setMaxSpeed(slowTilt);
}

//Main loop of the program
void loop()
{
  if (Serial.available())
  {
    char sign = Serial.read();

    if (sign == '*')
    {
      isLiveMode = true;
      clearLCD();
      lcd.print("---Live Mode---");
    }
    else if (sign == '^')
    {
      isLiveMode = false;
    }

    //Possible commends from the application:
    // ' ' - string ending with space sign marks the end of sequence mode
    // * - string ending with this sign marks the live mode (controlled from joystick)
    // ? - question about current situation, returns '!' when motors are not moving or '~' during operation
    // @ - returns ranges for each axis
    // # - returns current position of all motors
    if (isLiveMode)
    {
      liveModeSwitch(sign);
    }
    else
    {
      sequenceModeSwitch(sign);
    }
  }

  //After sequence completion informs the application
  if (!stepperSlide.isRunning() && !stepperPan.isRunning() && !stepperTilt.isRunning() && steppersInProgress && !isLiveMode)
  {
    clearLCD();
    lcd.print("Waiting...");
    Serial.println("!");
    steppersInProgress = false;
  }

  stepperSlide.run();
  stepperPan.run();
  stepperTilt.run();
}

//Parses the key instructions for sequence mode
void parseCommands(String dataFromCOM)
{
  slideCom = dataFromCOM.substring(dataFromCOM.indexOf("s") + 1, dataFromCOM.indexOf("p")).toInt();
  panCom = dataFromCOM.substring(dataFromCOM.indexOf("p") + 1, dataFromCOM.indexOf("t")).toInt();
  tiltCom = dataFromCOM.substring(dataFromCOM.indexOf("t") + 1, dataFromCOM.indexOf(" ")).toInt();
}

//Method for sequence mode
void sequenceMode(String dataFromCom)
{
  clearLCD();
  lcd.print("-Sequence Mode-");

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

  // stepperPositions[0] = slideCom;
  // stepperPositions[1] = panCom;
  // stepperPositions[2] = tiltCom;

  // StepperControl.moveTo(stepperPositions);
  // StepperControl.runSpeedToPosition();

  stepperSlide.moveTo(slideCom);
  stepperPan.moveTo(panCom);
  stepperTilt.moveTo(tiltCom);

  steppersInProgress = true;
}

void liveModeSwitch(char sign)
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
    stepperSlide.setMaxSpeed(mediumSlide);
    fullCommand = "";
    break;
  case '3':
    stepperSlide.setMaxSpeed(fastSlide);
    fullCommand = "";
    break;
  case '<':
    stepperPan.stop();
    fullCommand = "";
    break;
  case '4':
    stepperPan.setMaxSpeed(slowPan);
    fullCommand = "";
    break;
  case '5':
    stepperPan.setMaxSpeed(mediumPan);
    fullCommand = "";
    break;
  case '6':
    stepperPan.setMaxSpeed(fastPan);
    fullCommand = "";
    break;
  case '>':
    stepperTilt.stop();
    fullCommand = "";
    break;
  case '7':
    stepperTilt.setMaxSpeed(slowTilt);
    fullCommand = "";
    break;
  case '8':
    stepperTilt.setMaxSpeed(mediumTilt);
    fullCommand = "";
    break;
  case '9':
    stepperTilt.setMaxSpeed(fastTilt);
    fullCommand = "";
    break;
  case '*':
    Serial.println("Live mode set!");
    break;
  case '^':
    isLiveMode = false;
    break;
  case '?':
    if (!stepperSlide.isRunning() && !stepperPan.isRunning() && !stepperSlide.isRunning())
    {
      Serial.println("! "); //when all stepper motors are in idle
    }
    else
    {
      Serial.println("~ "); //while stepper motors are moving
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
  default:
    Serial.println("Invalid command");
    break;
  }
}

void sequenceModeSwitch(char sign)
{
  switch (sign)
  {
  case ' ':
    stepperSlide.setMaxSpeed(2000);
    stepperPan.setMaxSpeed(400);
    stepperTilt.setMaxSpeed(200);

    //deals with the full command and divides commands to the specific stepper motors
    sequenceMode(fullCommand);
    fullCommand = "";
    break;
  case '?':
    if (!stepperSlide.isRunning() && !stepperPan.isRunning() && !stepperSlide.isRunning())
    {
      Serial.println("! "); //when all stepper motors are in idle
    }
    else
    {
      Serial.println("~ "); //while stepper motors are moving
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
    Serial.println("Sequence mode set!");
    break;
  default:
    fullCommand += sign;
    break;
  }
}

//Sends ranges of each axis to the appliaction
void sendMotorRanges()
{
  snprintf(temp, 30, "@s%dp%dt%d ", slideRange, panRange, tiltRange);
  Serial.println(temp);
}

//Sends current position of motors
void sendCurrentPositions()
{
  int slidePos = stepperSlide.currentPosition();
  int panPos = stepperPan.currentPosition();
  int tiltPos = stepperTilt.currentPosition();
  snprintf(temp, 30, "#s%dp%dt%d ", slidePos, panPos, tiltPos);
  Serial.println(temp);
}

//Clears the LCD and sets the cursor to home position
void clearLCD()
{
  lcd.clear();
  lcd.home();
}