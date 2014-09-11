void flash(byte led,int time){ //this function is mainly used for debugging purposes
  digitalWrite(led,HIGH); //green led
  delay(time);
  digitalWrite(led,LOW);
}

void anFlash(byte led,int time,byte bright){ //this function is mainly used for debugging purposes (too)
  analogWrite(led,HIGH); //green led
  delay(time);
  analogWrite(led,LOW);
}

void fade(byte led,int time){
  for(int i=0;i<255;i++){
    analogWrite(led,i);
    delay(time);
  }
  for(int i=255;i>=0;i--){
    analogWrite(led,i);
    delay(time);
  }
}

///////////////////////////////////
//----- Utils
///////////////////////////////////
void init_msg_array() {
  for(int i=0; i<MSG_COUNT_MAX; i++) {
    for(int j=0; j<MSG_BUFFER_MAX; j++) {
      msgBuffer[i][j] = 0x00;
    }
  }
  msgParsingLine = 0;
  msgParsingChar = 0;    // First 2 byte is management byte
  msgCurDisp = 0;
}

void init_emg_array() {
  for(int i=0; i<EMG_COUNT_MAX; i++) {
    for(int j=0; j<EMG_BUFFER_MAX; j++) {
      emgBuffer[i][j] = 0x00;
    }
  }
  emgParsingLine = 0;
  emgParsingChar = 0;    // First 2 byte is management byte
  emgCurDisp = 0;
}





