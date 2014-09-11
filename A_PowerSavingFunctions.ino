///////////////////////////////////
//----- Power Saving
///////////////////////////////////

void upPuller(void){

  pinMode(buttonPin2, INPUT);  // Defines button pin
  pinMode(buttonPin,INPUT);
  digitalWrite(buttonPin2, HIGH); //enables internal pullup resistor
  digitalWrite(buttonPin, HIGH);

  for(int i=2;i<=13;i++){
    switch(i){
    case 3: //button

      break;
    case 6: //blue led
      break;
    case 7: //button
      break;
    case 9: //green led
      break;
    case 10: //bt tx
      break;
    case 11: //bt rx
      break;

    default:
      pinMode(i,INPUT);
      digitalWrite(i,HIGH);
    }
  }

}

void beRightBack(){
  
  BTSerial.end(); //shuts off bt
  screenOff();
  analogWrite(gled,0); //shut off power led
  timeAwake=30000;
  isSleeping=true;

  sleep();

  onDraw(current_time,true);
  flash(bled,100);

  analogWrite(gled,5); //lights up power led
  BTSerial.begin(BTspeed); //reboot bluetooth

  elapsedTime=millis(); //resets counter

}

void sleep(void) {

  while(isSleeping){
    // Set pin 3 as interrupt and attach handler:
    attachInterrupt(1, pinInterrupt, LOW);
    //
    // Choose our preferred sleep mode:
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    //
    MCUSR &= ~(1 << WDRF);
    WDTCSR |= (1 << WDCE) | (1 << WDE);              // enable configuration change s
    WDTCSR = (1<< WDP0) | (0 << WDP1) | (0 << WDP2) | (1 << WDP3); // set the prescalar = 9
    WDTCSR |= (1 << WDIE);                           // enable interrupt mode
    //Set sleep enable (SE) bit:
    sleep_enable();
    //Put the device to sleep:
    sleep_mode();
    //
    // Upon waking up,sketch continues from this point
    sleep_disable();
  }


  detachInterrupt(1);
}

void pinInterrupt(void) {
  isSleeping=false;
}

ISR( WDT_vect ) {

  sleepingTime+=8000;
  //flash(bled,250);
//  if((millis()+sleepingTime)-lastSync>1680000){
//    isSleeping=false;
//    timeAwake=240000;
//  }

}

void screenOff(){
  display.clearDisplay(); //blacks down display
  display.display();
}


