/*
    RetroWatch Arduino is a part of open source smart watch project.
 Copyright (C) 2014  Suh Young Bae

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see [http://www.gnu.org/licenses/].
 */
/*
Retro Watch Arduino v1.0

 Get the latest version, android host app at
 ------> https://github.com/godstale/retrowatch
 ------> or http://www.hardcopyworld.com

 Written by Suh Young Bae (godstale@hotmail.com)
 All text above, and the first splash screen(Adafruit) must be included in any redistribution
 */

/*--used pins--
 buttons : 3 (PULLUP) ,7
 leds: 6 Blue ,9 Green
 bt: RX 11,TX 10
 display: A4 (SDA),A5 (SCL)
 */

//#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/io.h>
#include <avr/wdt.h>

#include <Wire.h>
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include "bitmap.h"

///////////////////////////////////////////////////////////////////
//----- OLED instance
#define OLED_RESET 8
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//----- BT instance
SoftwareSerial BTSerial(11, 10); //Connect HC-06, RX, TX
const int BTspeed = 9600;
///////////////////////////////////////////////////////////////////

//----- Bluetooth transaction parsing
#define TR_MODE_IDLE 1
#define TR_MODE_WAIT_CMD 11
#define TR_MODE_WAIT_MESSAGE 101
#define TR_MODE_WAIT_TIME 111
#define TR_MODE_WAIT_ID 121
#define TR_MODE_WAIT_COMPLETE 201

#define TRANSACTION_START_BYTE 0xfc
#define TRANSACTION_END_BYTE 0xfd

#define CMD_TYPE_NONE 0x00
#define CMD_TYPE_RESET_EMERGENCY_OBJ 0x05
#define CMD_TYPE_RESET_NORMAL_OBJ 0x02
#define CMD_TYPE_RESET_USER_MESSAGE 0x03

#define CMD_TYPE_ADD_EMERGENCY_OBJ 0x11
#define CMD_TYPE_ADD_NORMAL_OBJ 0x12
#define CMD_TYPE_ADD_USER_MESSAGE 0x13

#define CMD_TYPE_DELETE_EMERGENCY_OBJ 0x21
#define CMD_TYPE_DELETE_NORMAL_OBJ 0x22
#define CMD_TYPE_DELETE_USER_MESSAGE 0x23

#define CMD_TYPE_SET_TIME 0x31
#define CMD_TYPE_REQUEST_MOVEMENT_HISTORY 0x32
#define CMD_TYPE_SET_CLOCK_STYLE 0x33
#define CMD_TYPE_SET_INDICATOR 0x34

#define CMD_TYPE_PING 0x51
#define CMD_TYPE_AWAKE 0x52
#define CMD_TYPE_SLEEP 0x53
#define CMD_TYPE_REBOOT 0x54

byte TRANSACTION_POINTER = TR_MODE_IDLE;
byte TR_COMMAND = CMD_TYPE_NONE;

//----- Message item buffer
#define MSG_COUNT_MAX 7
#define MSG_BUFFER_MAX 19
unsigned char msgBuffer[MSG_COUNT_MAX][MSG_BUFFER_MAX];
char msgParsingLine = 0;
char msgParsingChar = 0;
char msgCurDisp = 0;

//----- Emergency item buffer
#define EMG_COUNT_MAX 3
#define EMG_BUFFER_MAX 19
char emgBuffer[EMG_COUNT_MAX][EMG_BUFFER_MAX];
char emgParsingLine = 0;
char emgParsingChar = 0;
char emgCurDisp = 0;

//----- Time
#define UPDATE_TIME_INTERVAL 60000
byte iMonth = 1;
byte iDay = 1;
byte iWeek = 1;    // 1: SUN, MON, TUE, WED, THU, FRI,SAT
byte iAmPm = 0;    // 0:AM, 1:PM
byte iHour = 0;
byte iMinutes = 0;
byte iSecond = 0;

#define TIME_BUFFER_MAX 6
char timeParsingIndex = 0;
char timeBuffer[6] = {
  -1, -1, -1, -1, -1, -1};
PROGMEM const char* weekString[] = {
  "", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
PROGMEM const char* ampmString[] = {
  "AM", "PM"};

//----- Display features
#define DISPLAY_MODE_START_UP 0
#define DISPLAY_MODE_CLOCK 1
#define DISPLAY_MODE_EMERGENCY_MSG 2
#define DISPLAY_MODE_NORMAL_MSG 3
#define DISPLAY_MODE_IDLE 11
byte displayMode = DISPLAY_MODE_START_UP;

#define CLOCK_STYLE_SIMPLE_ANALOG  0x01
#define CLOCK_STYLE_SIMPLE_DIGIT  0x02
#define CLOCK_STYLE_SIMPLE_MIX  0x03
byte clockStyle = CLOCK_STYLE_SIMPLE_MIX;

#define INDICATOR_ENABLE 0x01
boolean updateIndicator = true;

byte centerX = 64;
byte centerY = 32;
byte iRadius = 28;

#define IDLE_DISP_INTERVAL 60000
#define CLOCK_DISP_INTERVAL 60000
#define EMERGENCY_DISP_INTERVAL 5000
#define MESSAGE_DISP_INTERVAL 3000
unsigned long prevClockTime = 0;
unsigned long prevDisplayTime = 0;

unsigned long next_display_interval = 0;
unsigned long mode_change_timer = 0;
#define CLOCK_DISPLAY_TIME 300000
#define EMER_DISPLAY_TIME 10000
#define MSG_DISPLAY_TIME 5000

unsigned long current_time;

//----- Button control
byte buttonPin = 3;
boolean isClicked = false;
byte buttonPin2= 7;

//----- Leds
byte gled=6;
byte bled=9;

//----- Power Saving
unsigned long elapsedTime; //used for sending arduino to sleep
unsigned long sleepingTime; //increased by watchdog timer every 8 seconds,measures time spent sleeping cause millis() timer freezes during sleep
unsigned long lastSync; //used for waking up and forcing arduino awake for a longer interval than usual ; used for syncing during sleep
unsigned long timeAwake; //default time interval before shutting off
boolean isSleeping;
boolean stayAwake; //used for preventing arduino from sleeping while syncing

//------------------------------------------------------------------------------------------------------------

void setup()   {

  //Serial.begin(9600);    // Do not enable serial. This makes serious problem because of shortage of RAM.

  upPuller(); //automatically defines input pins (buttons, too) and enables pullup resistor on each ; used for power saving as unused pins floating can consume power


    init_emg_array();
  init_msg_array();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.display();    // show splashscreen
  delay(1000);
  drawStartUp();    // Show RetroWatch Logo
  centerX = display.width() / 2;
  centerY = display.height() / 2;
  iRadius = centerY - 2;

  BTSerial.begin(BTspeed);  // set the data rate for the BT port

  pinMode(gled,OUTPUT);
  pinMode(bled,OUTPUT);

  //these are just for startup 'beauty'.
  for(int i=0;i<255;i++){
    analogWrite(gled,i);
    analogWrite(bled,i);
    delay(2);
  }
  for(int i=255;i>=0;i--){
    analogWrite(gled,i);
    analogWrite(bled,i);
    delay(2);
  }

  analogWrite(gled,5);

  elapsedTime=millis(); //starts up sleeping timer
  sleepingTime=0;
  lastSync=0;
  timeAwake=30000; //default period before powering down
  stayAwake=false;
}

void loop() {
  boolean isReceived = false;

  // Get button input
  if(digitalRead(buttonPin2) == LOW){
    isClicked = LOW;
    elapsedTime = millis();
  }

  // Receive data from remote and parse
  isReceived = receiveBluetoothData();

  // Update clock time
  current_time = millis()+sleepingTime;
  updateTime(current_time);

  // Display routine
  onDraw(current_time,false);

  /* // If data doesn't arrive, wait for a while to save battery
   if(!isReceived){
   delay(300);
   } */

  if(((millis()-elapsedTime)>timeAwake) && !stayAwake){
    beRightBack();
  }

}

///////////////////////////////////
//----- Time functions
///////////////////////////////////
void setTimeValue() {
  iMonth = timeBuffer[0];
  iDay = timeBuffer[1];
  iWeek = timeBuffer[2];    // 1: SUN, MON, TUE, WED, THU, FRI,SAT
  iAmPm = timeBuffer[3];    // 0:AM, 1:PM
  iHour = timeBuffer[4];
  if(iAmPm==1){
    iHour+=12;
  }
  iMinutes = timeBuffer[5];
}

void updateTime(unsigned long current_time) {
  if(iMinutes >= 0) {
    if(current_time - prevClockTime > UPDATE_TIME_INTERVAL) {
      // Increase time
      iMinutes+=((current_time - prevClockTime)/60000); //had to do all of this mess about minutes,otherwise it would just increase it by 1 even if it had been sleeping for more


      if(iMinutes >= 60) {

        sleepingTime=0;
        iHour+=iMinutes/60;
        iMinutes = iMinutes%60;

        (iHour>12) ? iAmPm=1 : iAmPm=0;

          if(iHour > 24) {
          iHour = 0;
          iAmPm=0;

          iWeek++;

          if(iWeek > 7)
            iWeek = 1;
          iDay++;
          if(iDay > 30)  // Yes. day is not exact.
            iDay = 1;
        }
      }
      prevClockTime = millis() + sleepingTime;
    }

  }
  else {
    displayMode = DISPLAY_MODE_START_UP;
  }
}













