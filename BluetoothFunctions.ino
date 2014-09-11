///////////////////////////////////
//----- BT, Data parsing functions
///////////////////////////////////

// Parsing packet according to current mode
boolean receiveBluetoothData() {
  int isTransactionEnded = false;
  while(!isTransactionEnded) {
    if(BTSerial.available()) {
      
      anFlash(bled,50,100);
      lastSync=current_time;
      timeAwake=30000;
      elapsedTime=millis();

      byte c = BTSerial.read();

      if(c == 0xFF && TRANSACTION_POINTER != TR_MODE_WAIT_MESSAGE) return false;

      if(TRANSACTION_POINTER == TR_MODE_IDLE) {
        parseStartSignal(c);
      }
      else if(TRANSACTION_POINTER == TR_MODE_WAIT_CMD) {
        parseCommand(c);
      }
      else if(TRANSACTION_POINTER == TR_MODE_WAIT_MESSAGE) {
        parseMessage(c);
      }
      else if(TRANSACTION_POINTER == TR_MODE_WAIT_TIME) {
        parseTime(c);
      }
      else if(TRANSACTION_POINTER == TR_MODE_WAIT_ID) {
        parseId(c);
      }
      else if(TRANSACTION_POINTER == TR_MODE_WAIT_COMPLETE) {
        isTransactionEnded = parseEndSignal(c);

      }

    }  // End of if(BTSerial.available())
    else {
      isTransactionEnded = true;

    }
  }  // End of while()

  return true;
}  // End of receiveBluetoothData()

void parseStartSignal(byte c) {
  //drawLogChar(c);
  if(c == TRANSACTION_START_BYTE) {
    TRANSACTION_POINTER = TR_MODE_WAIT_CMD;
    TR_COMMAND = CMD_TYPE_NONE;
  }
}

void parseCommand(byte c) {
  if(c == CMD_TYPE_RESET_EMERGENCY_OBJ || c == CMD_TYPE_RESET_NORMAL_OBJ || c == CMD_TYPE_RESET_USER_MESSAGE) {
    TRANSACTION_POINTER = TR_MODE_WAIT_COMPLETE;
    TR_COMMAND = c;
    processTransaction();
  }
  else if(c == CMD_TYPE_ADD_EMERGENCY_OBJ || c == CMD_TYPE_ADD_NORMAL_OBJ || c == CMD_TYPE_ADD_USER_MESSAGE) {
    TRANSACTION_POINTER = TR_MODE_WAIT_MESSAGE;
    TR_COMMAND = c;
    if(c == CMD_TYPE_ADD_EMERGENCY_OBJ) {
      emgParsingChar = 0;
      if(emgParsingLine >= MSG_COUNT_MAX || emgParsingLine < 0)
        emgParsingLine = 0;
    }
    else if(c == CMD_TYPE_ADD_NORMAL_OBJ) {
      msgParsingChar = 0;
      if(msgParsingLine >= MSG_COUNT_MAX || msgParsingLine < 0)
        msgParsingLine = 0;
    }
  }
  else if(c == CMD_TYPE_DELETE_EMERGENCY_OBJ || c == CMD_TYPE_DELETE_NORMAL_OBJ || c == CMD_TYPE_DELETE_USER_MESSAGE) {
    TRANSACTION_POINTER = TR_MODE_WAIT_COMPLETE;
    TR_COMMAND = c;
  }
  else if(c == CMD_TYPE_SET_TIME) {
    TRANSACTION_POINTER = TR_MODE_WAIT_TIME;
    TR_COMMAND = c;
  }
  else if(c == CMD_TYPE_SET_CLOCK_STYLE || c == CMD_TYPE_SET_INDICATOR) {
    TRANSACTION_POINTER = TR_MODE_WAIT_ID;
    TR_COMMAND = c;
  }
  else {
    TRANSACTION_POINTER = TR_MODE_IDLE;
    TR_COMMAND = CMD_TYPE_NONE;
  }
}

void parseMessage(byte c) {
  if(c == TRANSACTION_END_BYTE) {
    processTransaction();
    TRANSACTION_POINTER = TR_MODE_IDLE;
  }

  if(TR_COMMAND == CMD_TYPE_ADD_EMERGENCY_OBJ) {
    if(emgParsingChar < EMG_BUFFER_MAX) {
      if(emgParsingChar > 1) {
        emgBuffer[emgParsingLine][emgParsingChar] = c;
      }
      emgParsingChar++;
    }
    else {
      TRANSACTION_POINTER = TR_MODE_WAIT_COMPLETE;
    }
  }
  else if(TR_COMMAND == CMD_TYPE_ADD_NORMAL_OBJ) {
    if(msgParsingChar < MSG_BUFFER_MAX) {
      if(msgParsingChar > 1) {
        msgBuffer[msgParsingLine][msgParsingChar] = c;
      }
      msgParsingChar++;
    }
    else {
      TRANSACTION_POINTER = TR_MODE_WAIT_COMPLETE;
    }
  }
  else if(TR_COMMAND == CMD_TYPE_ADD_USER_MESSAGE) {
    // Not available yet.
    TRANSACTION_POINTER = TR_MODE_WAIT_COMPLETE;
  }
}

void parseTime(byte c) {
  if(TR_COMMAND == CMD_TYPE_SET_TIME) {
    if(timeParsingIndex >= 0 && timeParsingIndex < TIME_BUFFER_MAX) {
      timeBuffer[timeParsingIndex] = (int)c;
      timeParsingIndex++;
    }
    else {
      processTransaction();
      TRANSACTION_POINTER = TR_MODE_WAIT_COMPLETE;
    }
  }
}

void parseId(byte c) {
  if(TR_COMMAND == CMD_TYPE_SET_CLOCK_STYLE) {
    clockStyle = c;
    processTransaction();
  }
  else if(TR_COMMAND == CMD_TYPE_SET_INDICATOR) {
    if(c == INDICATOR_ENABLE)
      updateIndicator = true;
    else
      updateIndicator = false;
    processTransaction();
  }
  TRANSACTION_POINTER = TR_MODE_WAIT_COMPLETE;
}

boolean parseEndSignal(byte c) {
  if(c == TRANSACTION_END_BYTE) {
    TRANSACTION_POINTER = TR_MODE_IDLE;
    return true;
  }
  return false;
}

void processTransaction() {
  if(TR_COMMAND == CMD_TYPE_RESET_EMERGENCY_OBJ) {
    init_emg_array();//init_msg_array();
  }
  else if(TR_COMMAND == CMD_TYPE_RESET_NORMAL_OBJ) {
    init_msg_array();//init_emg_array();
  }
  else if(TR_COMMAND == CMD_TYPE_RESET_USER_MESSAGE) {
    // Not available yet.
  }
  else if(TR_COMMAND == CMD_TYPE_ADD_NORMAL_OBJ) {
    msgBuffer[msgParsingLine][0] = 0x01;
    msgParsingChar = 0;
    msgParsingLine++;
    if(msgParsingLine >= MSG_COUNT_MAX)
      msgParsingLine = 0;
    setNextDisplayTime(millis(), 0);  // update screen immediately
  }
  else if(TR_COMMAND == CMD_TYPE_ADD_EMERGENCY_OBJ) {
    emgBuffer[emgParsingLine][0] = 0x01;
    emgParsingChar = 0;
    emgParsingLine++;
    if(emgParsingLine >= EMG_COUNT_MAX)
      emgParsingLine = 0;
    startEmergencyMode();
    setNextDisplayTime(millis(), 2000);
  }
  else if(TR_COMMAND == CMD_TYPE_ADD_USER_MESSAGE) {
  }
  else if(TR_COMMAND == CMD_TYPE_DELETE_EMERGENCY_OBJ || TR_COMMAND == CMD_TYPE_DELETE_NORMAL_OBJ || TR_COMMAND == CMD_TYPE_DELETE_USER_MESSAGE) {
    // Not available yet.
  }
  else if(TR_COMMAND == CMD_TYPE_SET_TIME) {
    setTimeValue();
    timeParsingIndex = 0;
    setNextDisplayTime(millis(), 0);  // update screen immediately
  }
  if(TR_COMMAND == CMD_TYPE_SET_CLOCK_STYLE || CMD_TYPE_SET_INDICATOR) {
    setNextDisplayTime(millis(), 0);  // update screen immediately
  }
}




