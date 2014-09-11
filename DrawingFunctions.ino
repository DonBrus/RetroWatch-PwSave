///////////////////////////////////
//----- Drawing methods
///////////////////////////////////

// Main drawing routine.
// Every drawing starts here.
void onDraw(unsigned long currentTime,boolean wokeUp) {
  
  if(!isDisplayTime(currentTime) && !wokeUp)    // Do not re-draw at every tick
    return;
  
  if(displayMode == DISPLAY_MODE_START_UP) {
    drawStartUp();
  }
  else if(displayMode == DISPLAY_MODE_CLOCK) {
    if(isClicked == LOW) {    // User input received
      startEmergencyMode();
      setPageChangeTime(0);    // Change mode with no page-delay
      setNextDisplayTime(currentTime, 0);    // Do not wait next re-draw time
    }
    else {
      drawClock();

      if(isPageChangeTime(currentTime)) {  // It's time to go into idle mode
        startIdleMode();
        setPageChangeTime(currentTime);  // Set a short delay
      }
      setNextDisplayTime(currentTime, CLOCK_DISP_INTERVAL);
    }
  }
  else if(displayMode == DISPLAY_MODE_EMERGENCY_MSG) {
    if(findNextEmerMessage()) {
      drawEmergency();
      emgCurDisp++;
      if(emgCurDisp >= EMG_COUNT_MAX) {
        emgCurDisp = 0;
        startMessageMode();
      }
      setNextDisplayTime(currentTime, EMERGENCY_DISP_INTERVAL);
    }
    // There's no message left to display. Go to normal message mode.
    else {
      startMessageMode();
      //setPageChangeTime(0);
      setNextDisplayTime(currentTime, 0);  // with no re-draw interval
    }
  }
  else if(displayMode == DISPLAY_MODE_NORMAL_MSG) {
    if(findNextNormalMessage()) {
      drawMessage();
      msgCurDisp++;
      if(msgCurDisp >= MSG_COUNT_MAX) {
        msgCurDisp = 0;
        startClockMode();
      }
      setNextDisplayTime(currentTime, MESSAGE_DISP_INTERVAL);
    }
    // There's no message left to display. Go to clock mode.
    else {
      startClockMode();
      setPageChangeTime(currentTime);
      setNextDisplayTime(currentTime, 0);  // with no re-draw interval
    }
  }
  else if(displayMode == DISPLAY_MODE_IDLE) {
    if(isClicked == LOW) {    // Wake up watch if there's an user input
      startClockMode();
      setPageChangeTime(currentTime);
      setNextDisplayTime(currentTime, 0);
    }
    else {
      drawIdleClock();
      setNextDisplayTime(currentTime, IDLE_DISP_INTERVAL);
    }
  }
  else {
    startClockMode();    // This means there's an error
  }

  isClicked = HIGH;
}  // End of onDraw()


// To avoid re-draw on every drawing time
// wait for time interval according to current mode
// But user input(button) breaks this sleep
boolean isDisplayTime(unsigned long currentTime) {
  if(currentTime - prevDisplayTime > next_display_interval) {
    return true;
  }
  if(isClicked == LOW) {
    delay(500);
    return true;
  }
  return false;
}

// Set next re-draw time
void setNextDisplayTime(unsigned long currentTime, unsigned long nextUpdateTime) {
  next_display_interval = nextUpdateTime;
  prevDisplayTime = currentTime;
}

// Decide if it's the time to change page(mode)
boolean isPageChangeTime(unsigned long currentTime) {
  if(displayMode == DISPLAY_MODE_CLOCK) {
    if(currentTime - mode_change_timer > CLOCK_DISPLAY_TIME)
      return true;
  }
  return false;
}

// Set time interval to next page(mode)
void setPageChangeTime(unsigned long currentTime) {
  mode_change_timer = currentTime;
}

// Check if available emergency message exists or not
boolean findNextEmerMessage() {
  if(emgCurDisp < 0 || emgCurDisp >= EMG_COUNT_MAX) emgCurDisp = 0;
  while(true) {
    if(emgBuffer[emgCurDisp][0] == 0x00) {  // 0x00 means disabled
      emgCurDisp++;
      if(emgCurDisp >= EMG_COUNT_MAX) {
        emgCurDisp = 0;
        return false;
      }
    }
    else {
      break;
    }
  }  // End of while()
  return true;
}

// Check if available normal message exists or not
boolean findNextNormalMessage() {
  if(msgCurDisp < 0 || msgCurDisp >= MSG_COUNT_MAX) msgCurDisp = 0;
  while(true) {
    if(msgBuffer[msgCurDisp][0] == 0x00) {
      msgCurDisp++;
      if(msgCurDisp >= MSG_COUNT_MAX) {
        msgCurDisp = 0;
        return false;
      }
    }
    else {
      break;
    }
  }  // End of while()
  return true;
}

// Count all available emergency messages
int countEmergency() {
  int count = 0;
  for(int i=0; i<EMG_COUNT_MAX; i++) {
    if(emgBuffer[i][0] != 0x00)
      count++;
  }
  return count;
}

// Count all available normal messages
int countMessage() {
  int count = 0;
  for(int i=0; i<MSG_COUNT_MAX; i++) {
    if(msgBuffer[i][0] != 0x00)
      count++;
  }
  return count;
}

void startClockMode() {
  displayMode = DISPLAY_MODE_CLOCK;
}

void startEmergencyMode() {
  displayMode = DISPLAY_MODE_EMERGENCY_MSG;
  emgCurDisp = 0;
}

void startMessageMode() {
  displayMode = DISPLAY_MODE_NORMAL_MSG;
  msgCurDisp = 0;
}

void startIdleMode() {
  displayMode = DISPLAY_MODE_IDLE;
}

// Draw indicator. Indicator shows count of emergency and normal message
void drawIndicator() {
  if(updateIndicator) {
    int msgCount = countMessage();
    int emgCount = countEmergency();
    int drawCount = 1;

    if(msgCount > 0) {
      display.drawBitmap(127 - 8, 1, IMG_indicator_msg, 8, 8, WHITE);
      display.setTextColor(WHITE);
      display.setTextSize(1);
      display.setCursor(127 - 15, 1);
      display.print(msgCount);
      drawCount++;
    }

    if(emgCount > 0) {
      display.drawBitmap(127 - 8*drawCount - 7*(drawCount-1), 1, IMG_indicator_emg, 8, 8, WHITE);
      display.setTextColor(WHITE);
      display.setTextSize(1);
      display.setCursor(127 - 8*drawCount - 7*drawCount, 1);
      display.print(emgCount);
    }

  }
}

// RetroWatch splash screen
void drawStartUp() {
  display.clearDisplay();

  display.drawBitmap(10, 15, IMG_logo_24x24, 24, 24, WHITE);

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(45,12);
  display.println("Retro");
  display.setCursor(45,28);
  display.println("Watch");
  display.setTextSize(1);
  display.setCursor(45,45);
  display.setTextColor(WHITE);
  display.println("Arduino v1.0");
  display.display();
  delay(2000);

  startClockMode();
}

// Draw emergency message page
void drawEmergency() {
  int icon_num = 60;
  display.clearDisplay();

  if(updateIndicator)
    drawIndicator();

  if(emgBuffer[emgCurDisp][2] > -1 && emgBuffer[emgCurDisp][2] < ICON_ARRAY_SIZE)
    icon_num = (int)(emgBuffer[emgCurDisp][2]);

  drawIcon(centerX - 8, centerY - 20, icon_num);

  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(getCenterAlignedXOfEmg(emgCurDisp), centerY + 10);
  for(int i=3; i<EMG_BUFFER_MAX; i++) {
    char curChar = emgBuffer[emgCurDisp][i];
    if(curChar == 0x00) break;
    if(curChar >= 0xf0) continue;
    display.write(curChar);
  }

  display.display();
}

// Draw normal message page
void drawMessage() {
  int icon_num = 0;
  display.clearDisplay();

  if(updateIndicator)
    drawIndicator();

  if(msgBuffer[msgCurDisp][2] > -1 && msgBuffer[msgCurDisp][2] < ICON_ARRAY_SIZE)
    icon_num = (int)(msgBuffer[msgCurDisp][2]);

  drawIcon(centerX - 8, centerY - 20, icon_num);

  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(getCenterAlignedXOfMsg(msgCurDisp), centerY + 10);
  //  display.print(msgCurDisp);  // For debug
  for(int i=3; i<MSG_BUFFER_MAX; i++) {
    char curChar = msgBuffer[msgCurDisp][i];
    if(curChar == 0x00) break;
    if(curChar >= 0xf0) continue;
    display.write(curChar);
  }

  display.display();
}

// Draw main clock screen
// Clock style changes according to user selection
void drawClock() {
  display.clearDisplay();

  if(updateIndicator)
    drawIndicator();

  // CLOCK_STYLE_SIMPLE_DIGIT
  if(clockStyle == CLOCK_STYLE_SIMPLE_DIGIT) {
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(centerX - 34, centerY - 17);
    display.println((const char*)pgm_read_word(&(weekString[iWeek])));
    display.setTextSize(2);
    display.setCursor(centerX + 11, centerY - 17);
    display.println((const char*)pgm_read_word(&(ampmString[iAmPm])));

    display.setTextSize(2);
    display.setCursor(centerX - 29, centerY + 6);
    if(iHour < 10)
      display.print("0");
    display.print(iHour);
    display.print(":");
    if(iMinutes < 10)
      display.print("0");
    display.println(iMinutes);

    display.display();
  }
  // CLOCK_STYLE_SIMPLE_MIX
  else if(clockStyle == CLOCK_STYLE_SIMPLE_MIX) {
    display.drawCircle(centerY, centerY, iRadius - 6, WHITE);
    showTimePin(centerY, centerY, 0.1, 0.4, iHour*5 + (int)(iMinutes*5/60));
    showTimePin(centerY, centerY, 0.1, 0.70, iMinutes);

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(centerY*2 + 3, 23);
    display.println((const char*)pgm_read_word(&(weekString[iWeek])));
    display.setCursor(centerY*2 + 28, 23);
    display.println((const char*)pgm_read_word(&(ampmString[iAmPm])));

    display.setTextSize(2);
    display.setCursor(centerY*2, 37);
    if(iHour < 10)
      display.print("0");
    display.print(iHour);
    display.print(":");
    if(iMinutes < 10)
      display.print("0");
    display.println(iMinutes);
    display.display();
  }
  else {
    // CLOCK_STYLE_SIMPLE_ANALOG.
    display.drawCircle(centerX, centerY, iRadius, WHITE);
    showTimePin(centerX, centerY, 0.1, 0.5, iHour*5 + (int)(iMinutes*5/60));
    showTimePin(centerX, centerY, 0.1, 0.78, iMinutes);
    // showTimePin(centerX, centerY, 0.1, 0.9, iSecond);
    display.display();

    iSecond++;
    if(iSecond > 60) iSecond = 0;
  }
}

// Draw idle page
void drawIdleClock() {
  display.clearDisplay();

  if(updateIndicator)
    drawIndicator();

  display.setTextSize(2);
  display.setCursor(centerX - 29, centerY - 4);
  if(iHour < 10)
    display.print("0");
  display.print(iHour);
  display.print(":");
  if(iMinutes < 10)
    display.print("0");
  display.println(iMinutes);

  display.display();
}

// Returns starting point of normal string to display
int getCenterAlignedXOfMsg(int msgIndex) {
  int pointX = centerX;
  for(int i=3; i<MSG_BUFFER_MAX; i++) {
    char curChar = msgBuffer[msgIndex][i];
    if(curChar == 0x00) break;
    if(curChar >= 0xf0) continue;
    pointX -= 3;
  }
  if(pointX < 0) pointX = 0;
  return pointX;
}

// Returns starting point of emergency string to display
int getCenterAlignedXOfEmg(int emgIndex) {
  int pointX = centerX;
  for(int i=3; i<EMG_BUFFER_MAX; i++) {
    char curChar = emgBuffer[emgIndex][i];
    if(curChar == 0x00) break;
    if(curChar >= 0xf0) continue;
    pointX -= 3;
  }
  if(pointX < 0) pointX = 0;
  return pointX;
}

// Calculate clock pin position
double RAD=3.141592/180;
double LR = 89.99;
void showTimePin(int center_x, int center_y, double pl1, double pl2, double pl3) {
  double x1, x2, y1, y2;
  x1 = center_x + (iRadius * pl1) * cos((6 * pl3 + LR) * RAD);
  y1 = center_y + (iRadius * pl1) * sin((6 * pl3 + LR) * RAD);
  x2 = center_x + (iRadius * pl2) * cos((6 * pl3 - LR) * RAD);
  y2 = center_y + (iRadius * pl2) * sin((6 * pl3 - LR) * RAD);

  display.drawLine((int)x1, (int)y1, (int)x2, (int)y2, WHITE);
}

// Icon drawing tool
void drawIcon(int posx, int posy, int icon_num) {
  if(icon_num < 0 || icon_num >= ICON_ARRAY_SIZE)
    return;

  display.drawBitmap(posx, posy, (const unsigned char*)pgm_read_word(&(bitmap_array[icon_num])), 16, 16, WHITE);
}

