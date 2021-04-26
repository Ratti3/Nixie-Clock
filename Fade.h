/*
  // Fade Out
  for(uint8_t i = 255; i != 0; i -= 1){
    analogWrite(PIN_HV_BL,i);
    analogWrite(PIN_DOT_THREE, i);
    analogWrite(PIN_DOT_FOUR, i);
      analogWrite(PIN_COLON, i);
    delay(40);
  }

  // Fade In
  for(uint8_t i = 0; i != 255; i +=1 ){
    analogWrite(PIN_HV_BL, i);
    analogWrite(PIN_DOT_THREE, i);
    analogWrite(PIN_DOT_FOUR, i);
    analogWrite(PIN_COLON, i);
    delay(40);
  }
  
  digitalWrite(PIN_HV_EN, LOW);
  delay(1000);

*/
