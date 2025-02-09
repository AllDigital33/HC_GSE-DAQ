
/* Half Cat GSE Controller New Board Diagnostics
 *  
 *  Used to test all sensors, inputs and outputs on a new GSE Controller board
 *  Current hardware version is 1.0
 *  
 *   
 *  
 */ 

 // Pin configuration
  #define RXD1 0
  #define TXD1 1
  #define RXD2 7
  #define TXD2 8
  #define pin_rocketDetect 16 // PU 
  #define pin_igniterContinuity 33 
  #define pin_continuitySwitch 31 // PU
  #define pin_led1 28 //
  #define pin_led2 29 //
  #define pin_buzzer 30 //
  #define pin_igniterFIRE 34 // 
  #define pin_servoFill 22 
  #define pin_servoPurge 23
  #define pin_servoFuel 15
  #define pin_servoOx 14
  #define pin_servoSpare 36
  #define pin_hazVolts A11
  #define pin_radioSet 27
  #define pin_internalLED 13
  #define pin_ext5v 35
  #define pin_ext6v 21
  #define pin_fuseMain 20
  #define pin_fan 26
  #define pin_analogSpare A14
  #define pin_digSpare 39



  #include "Arduino.h"
  #include "ADS1115-Driver.h"
    extern "C" uint32_t set_arm_clock(uint32_t frequency); // required to set MCU clock speed
  ADS1115 ads1115a = ADS1115(0x49);


  #include <Wire.h>
  #include <Adafruit_INA219.h>
  Adafruit_INA219 ina219;

 //Servos
  #include <Servo.h> 
  Servo fuelServo;  
  Servo oxServo;
  Servo fillServo;
  Servo purgeServo;

 //Radio
 char theWord[100];
 int newWord = 0;

// For the flash memory
  //Little FS flash
  #include <LittleFS.h>
  char buf[512] = "";
  char fname1[32] = "/testfile.txt"; //testfile.txt
  //LittleFS_QSPIFlash myfs; // RT5.1
  LittleFS_QPINAND myfs; // used for fully loaded T4.1 W25N02KVZEIR
 
  File file, file1, file3;
//  #include <LittleFS_NAND.h>
  #include <stdarg.h>
  uint64_t fTot, totSize1;
  
  #include <TimeLib.h>  // for logging time


// Temperature from CPU
  extern float tempmonGetTemp(void);  

// just for SD
  #include "SD.h"
  #include <SPI.h> 
  File tempFile;

// enabling ADC channels

  bool aEnabled[8] = {1,1,1,1,1,1,1,1};
  

//************************************************************************************************************************************************************  SETUP
//************************************************************************************************************************************************************  SETUP

 
void setup() {

// Pin setup
  pinMode(pin_led1, OUTPUT);  digitalWrite(pin_led1, LOW);
  pinMode(pin_led2, OUTPUT);  digitalWrite(pin_led2, LOW);
  pinMode(pin_buzzer, OUTPUT);  digitalWrite(pin_buzzer, LOW);
  pinMode(pin_igniterFIRE, OUTPUT);  digitalWrite(pin_igniterFIRE, LOW);
  pinMode(pin_hazVolts, INPUT);

  pinMode(pin_igniterContinuity, INPUT_PULLUP);   
  pinMode(pin_rocketDetect, INPUT_PULLUP);  
  pinMode(pin_continuitySwitch, INPUT_PULLUP);  
  pinMode(pin_fuseMain, INPUT_PULLUP);

  pinMode(pin_radioSet, OUTPUT);digitalWrite(pin_radioSet, LOW);
  pinMode(pin_internalLED, OUTPUT); digitalWrite(pin_internalLED, LOW);
  pinMode(pin_ext5v, OUTPUT);digitalWrite(pin_ext5v, LOW);
  pinMode(pin_ext6v, OUTPUT);digitalWrite(pin_ext6v, LOW);
  pinMode(pin_fan, OUTPUT);digitalWrite(pin_fan, LOW);
  pinMode(pin_digSpare, OUTPUT);digitalWrite(pin_digSpare, LOW);

  fuelServo.attach(pin_servoFuel); 
  oxServo.attach(pin_servoOx);
  purgeServo.attach(pin_servoPurge);
  fillServo.attach(pin_servoFill);
  
  delay(4000);
  Serial.begin(115200);
  delay(100);
  Serial.println("Half Cat GSE New Board Diagnostics and Health Check");
  Serial.println(" ");Serial.println(" ");

  printCommands();

}

//************************************************************************************************************************************************************  MAIN LOOP
//************************************************************************************************************************************************************  MAIN LOOP

void loop() {

  char chIn = 255;
  if ( Serial.available() ) {
    do {
     if ( chIn != '0' && chIn != '1' && chIn != '2' && chIn != 'F' && chIn != '$' && chIn != 'C' && chIn != '#' && chIn != '3' && chIn != '4' && chIn != '5' && chIn != '6' && chIn != '7' && chIn != '8' && chIn != '9' && chIn != 'a' && chIn != 'b' && chIn != 'c' && chIn != 'd' && chIn != 'e' && chIn != 'f' && chIn != 'g' && chIn != 'h' && chIn != 'i' && chIn != 'j' && chIn != 'k' && chIn != 'l' && chIn != 'm' && chIn != 'n' && chIn != 'o' && chIn != 'p' && chIn != 'q' && chIn != 'r' && chIn != 's' && chIn != 't'&& chIn != 'u'&& chIn != 'v'&& chIn != 'w'&& chIn != 'x'&& chIn != 'y'&& chIn != 'z' && chIn != '?' && chIn != '!' && chIn != '&' && chIn != '*' && chIn != '+'  )
        chIn = Serial.read();
      else
        Serial.read();
    }
    while ( Serial.available() );
  }

  switch(chIn) {
    case '1':
      toggleLed1(); printCommands(); break;
    case '2':
      toggleLed2(); printCommands(); break;
    case '3':
      //toggleLed3(); printCommands(); break;                
    case '4':
      toggleLB(); printCommands(); break;
    case '5':
      toggle5v();printCommands(); break;
    case '6':
      toggle6v();printCommands(); break;
    case '7':
      checkCont();printCommands(); break;
    case '8':
      toggleIgniter(); printCommands(); break;
    case '9':
      RadioTXtest();printCommands(); break;
    case '0':
      toggleRset(); printCommands(); break;

      
    case 'a':
      servoFuelOpen();  printCommands(); break;
    case 'b':
      servoFuelClose(); printCommands(); break;
    case 'c':
      servoOxOpen(); printCommands(); break;
    case 'd':
      servoOxClose(); printCommands(); break;
    case 'e':
      servoFillOpen(); printCommands(); break;
    case 'f':
      servoFillClose(); printCommands(); break;
    case 'g':
      servoPurgeOpen(); printCommands(); break;
    case 'h':
      servoPurgeClose(); printCommands(); break;
    case 'i':
      rocketConnect(); printCommands(); break;
    case 'j':
      continuitySwitch(); printCommands(); break;
    case 'k':
      bothValves(); printCommands(); break;
    case 'l':
      //toggleV3(); printCommands(); break;
    case 'm':
     // toggleV4(); printCommands(); break;
    case 'n':
      showCurrent(); printCommands(); break;
    case 'o':
      //toggleV6(); printCommands(); break;
    case 'p':
      formatFlash(); printCommands(); break;
    case 'q':
      eraseFlash(); printCommands(); break;
    case 'r':
      printDirectory(); printCommands(); break;
    case 's':
      copy2SD(); printCommands(); break;
    case 't':
      flashLogging(); printCommands(); break;
    case 'u':
      //toggleValve("VPP"); printCommands(); break;
    case 'v':
      checkTemp(); printCommands(); break;
    case 'w':
      //toggleValve("VBO"); printCommands(); break;
    case 'x':
      I2Cscan(); printCommands(); break;
    case 'y':
      PTloop(); printCommands(); break;
    case 'z':
      //radioRead();printCommands(); break; //Configuration report
      radioTest();printCommands(); break;
    case '$':
      readConfig();printCommands(); break;
    case '#':
      readFile();printCommands(); break;
    case '?':
      toggleBuzzer(); printCommands(); break;
    case '&':
      toggleFan();printCommands(); break;
    case '*':
      fuseTest();printCommands(); break;
    case '+':
      checkVoltage();printCommands(); break;
    case 'C':
      DAQtest();printCommands(); break;
    case 'F':
      formatUnused();printCommands(); break;
  }
  
}

void printCommands() {
  Serial.println("  ");
  Serial.println("----------------------------------------------------------------------------------------------------------------------------------------------------------");
  Serial.println("COMMANDS:");
  Serial.println("      Local:  1-LED1, 2-LED2, 4-LED BLTIN, ?-Buzzer, 5-5vExt, 6-6v Ext, &-Fan ");
  Serial.println("    Sensing:  v-CPU Temp, j-Cont Switch, *-Fuses, +-Voltage n-Current");
  Serial.println("     Servos:  a-Fuel Open, b-Fuel Close, c-Ox Open, d-Ox Close, e-Fill Open, f-Fill Close, g-Purge Open, h-Purge close, k-both open");
  Serial.println("     Ignite:  7-Ign Continuity, 8-Igniter Fire"); 
  Serial.println("        PTs:  x-I2C Scan, y-PT values");
  Serial.println("      Radio:  z-Radio Test, 0-radio set, 9-Radio TX");
  Serial.println("         SD:  q-Quick Erase, r-Directory, t-Flash Logging Test, $-read config file, #-read file, C-DAQTest");  
  Serial.println("-----------------------------------------------------------------------------------------------------------------------------------------------------------");  
  Serial.println("  ");
}

void showCurrent() {

  Serial.println("INA219 Current Sensing");
  Serial.println(" ");
  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    return;
  }
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  
  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");
  Serial.println("");



}



void servoFuelOpen() {

  Serial.println("Fuel Valve Open");
  Serial.println(" ");
  fuelServo.writeMicroseconds(500); // zero is open

}

void servoFuelClose() {

  Serial.println("Fuel Valve Close");
  Serial.println(" ");
  fuelServo.writeMicroseconds(1200); // 1200 is closed

}

void servoOxOpen() {

  Serial.println("Ox Valve Open");
  Serial.println(" ");
  oxServo.writeMicroseconds(500); // zero is open

}

void servoOxClose() {

  Serial.println("Ox Valve Close");
  Serial.println(" ");
  oxServo.writeMicroseconds(1200); // 1200 is closed

}


void bothValves() {

  Serial.println("Fill and Purge Valve Open Same time");
  Serial.println(" ");
  fillServo.writeMicroseconds(500); // zero is open
  purgeServo.writeMicroseconds(600); // zero is open





}




void servoFillOpen() {

  Serial.println("Fill Valve Open");
  Serial.println(" ");
  fillServo.writeMicroseconds(500); // zero is open

}

void servoFillClose() {

  Serial.println("Fill Valve Close");
  Serial.println(" ");
  fillServo.writeMicroseconds(1200); // 1200 is closed

}

void servoPurgeOpen() {

  Serial.println("Purge Valve Open");
  Serial.println(" ");
  purgeServo.writeMicroseconds(600); // zero is open

}

void servoPurgeClose() {

  Serial.println("Purge Valve Close");
  Serial.println(" ");
  purgeServo.writeMicroseconds(1200); // 1200 is closed

}

void checkVoltage() {

  Serial.println("Checking voltage using the ADC... 10s read 10s");
  Serial.println(" ");
  // ADS1115 
  Wire.begin();
  delay(1);
  ads1115a.reset();
  delay(100);

  ads1115a.setDeviceMode(ADS1115_MODE_SINGLE);
  ads1115a.setDataRate(ADS1115_DR_860_SPS);
  ads1115a.setPga(ADS1115_PGA_6_144);
  delay(100);

  Wire.setClock(400000); 
  
  delay(10000);

// ----------------------------------  Check Main Voltage  ------------------------
      elapsedMicros adcTimer = 0;
      float valueADC = 0.0;
      ads1115a.setMultiplexer(ADS1115_MUX_AIN0_GND);
      ads1115a.startSingleConvertion();
      adcTimer = 0;
      while(adcTimer < 1000) {}; //wait 1000 micros for read - Q ver of ADS1115
      valueADC = ads1115a.readConvertedValue();
      float mainVolts = valueADC / 100; // opAmp voltage divider x10 (actual voltage)
      //8.32 = 100%
      //6.8 = 0%


    // ----------------------------------  Check Haz Voltage  ------------------------
    
      int h = analogRead(pin_hazVolts);
      //.   >150 is on
      
     // float hazVolts = valueADC / 100; // opAmp voltage divider x10

      delay(10000);

      Serial.print("Main Voltage = ");
      Serial.println(mainVolts);
      Serial.print("Raw Read = ");
      Serial.println(valueADC);
      Serial.print("Haz Raw/Voltage = ");
      Serial.println(h);


}


void toggle5v() {
  Serial.print("5v External Toggle is now ");
  if(digitalRead(pin_ext5v)) {
    Serial.println("OFF");
    digitalWrite(pin_ext5v, LOW);
  } else {
    Serial.println("ON");
    digitalWrite(pin_ext5v, HIGH);
  }
 
}

void toggle6v() {
  Serial.print("6v External Toggle is now ");
  if(digitalRead(pin_ext6v)) {
    Serial.println("OFF");
    digitalWrite(pin_ext6v, LOW);
  } else {
    Serial.println("ON");
    digitalWrite(pin_ext6v, HIGH);
  }
 
}



void toggleFan() {
  Serial.print("Fan Toggle is now ");
  if(digitalRead(pin_fan)) {
    Serial.println("OFF");
    digitalWrite(pin_fan, LOW);
  } else {
    Serial.println("ON");
    digitalWrite(pin_fan, HIGH);
  }
 
}

void fuseTest() {
      Serial.print("Fuse Detect Status:  ");
      // If voltage is not on then this will report incorrectly


      if(digitalRead(pin_fuseMain) == LOW) {
        Serial.println("Main Fuse Good/Closed");
      } else {
        Serial.println("Main Fuse Open/Bad");
        //zzz check the voltage 
      }


      
}

void checkCont() {
      Serial.print("Igniter Continuity = ");
      if(digitalRead(pin_igniterContinuity) == LOW) {
        Serial.println("Closed");
      } else {
        Serial.println("Open");
      }
}

void checkTemp(){
      float CPUtemp = 0.0;
      Serial.println("Checking CPU Temperature...");Serial.println(" ");
      Serial.print("CPU Temp (f) = ");
      CPUtemp  = (tempmonGetTemp() * 9.0f / 5.0f) + 32.0f;
      Serial.println(CPUtemp);
      
}


void toggleIgniter(){
  Serial.print("Igniter Toggle is now ");
  if(digitalRead(pin_igniterFIRE)) {
    Serial.println("OFF");
    digitalWrite(pin_igniterFIRE, LOW);
  } else {
    Serial.println("ON");
    digitalWrite(pin_igniterFIRE, HIGH);
  }
}





void I2Cscan() {

 byte error, address;
  int nDevices;
  Serial.println("Scanning I2C Bus...");
  Wire.begin();
  delay(1000);
  
  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
 
    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
 
      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
  Wire.end(); 

  
}

//*****************************************************************************  ADC STUFF ******************************

void PTloop() {

  // ADS1115 
  Wire.begin();
  delay(1);
  ads1115a.reset();
  delay(100);
  ads1115a.setDeviceMode(ADS1115_MODE_SINGLE);
  ads1115a.setDataRate(ADS1115_DR_860_SPS);
  ads1115a.setPga(ADS1115_PGA_6_144);
  delay(100); 
  Wire.setClock(400000); 
  delay(100);  

  uint16_t valueA = 0;
  uint16_t valueB = 0;

  float value0f = 0.0;
  float value1f = 0.0;
  float value2f = 0.0;
  float value3f = 0.0;



  unsigned long aMicros = 900;
  //int aMicros = 100000;
  unsigned long  bMicros = 900;
  int aRotate = 0;

  int aCounter[5] = {0,0,0,0};

  aRotate = nextA(aRotate);


  // Do the first one A
  if(aRotate == 1) ads1115a.setMultiplexer(ADS1115_MUX_AIN0_GND);
  if(aRotate == 2) ads1115a.setMultiplexer(ADS1115_MUX_AIN1_GND);
  if(aRotate == 3) ads1115a.setMultiplexer(ADS1115_MUX_AIN2_GND);
  if(aRotate == 4) ads1115a.setMultiplexer(ADS1115_MUX_AIN3_GND);
  ads1115a.startSingleConvertion();



  elapsedMillis timeIt = 0;
  elapsedMicros aTimer = 0;


  Serial.println("Starting Five Second Sampling Test...");
  Serial.println("    ");
  
  
  while(timeIt < 5000) {

    if(aTimer >= aMicros) {
      //Serial.print("Starting    ");
      valueA = ads1115a.readConvertedValue();
      //Serial.print(valueA);
      //Serial.print("   ");
      //Serial.print(aRotate);
      //Serial.print("   ");      
      if(aRotate == 1) value0f += (float) valueA;
      if(aRotate == 2) value1f += (float) valueA;
      if(aRotate == 3) value2f += (float) valueA;
      if(aRotate == 4) value3f += (float) valueA;
      aCounter[aRotate]++;
      //Serial.print(aCounter[aRotate]);
      //Serial.print("    Next:");
      aRotate = nextA(aRotate);
      //Serial.print(aRotate);
      //Serial.print("   ");
      if(aRotate == 1) ads1115a.setMultiplexer(ADS1115_MUX_AIN0_GND);
      if(aRotate == 2) ads1115a.setMultiplexer(ADS1115_MUX_AIN1_GND);
      if(aRotate == 3) ads1115a.setMultiplexer(ADS1115_MUX_AIN2_GND);
      if(aRotate == 4) ads1115a.setMultiplexer(ADS1115_MUX_AIN3_GND);
      ads1115a.startSingleConvertion();
      aTimer = 0;
      //Serial.println("DONE!   ");
    }

    
  } // end five second while





  Serial.println("ACD1 Values:     ");
  if(aEnabled[0] == 1) value0f = value0f / (float) aCounter[1];
  if(aEnabled[1] == 1) value1f = value1f / (float) aCounter[2];
  if(aEnabled[2] == 1) value2f = value2f / (float) aCounter[3];
  if(aEnabled[3] == 1) value3f = value3f / (float) aCounter[4];
  
  Serial.print(" (VOL) IN 0: ");
  Serial.print(value0f,0);
  Serial.print("     count: ");
  Serial.println(aCounter[1]);
  Serial.print(" (HAZ) IN 1: ");
  Serial.print(value1f,0);
  Serial.print("     count: ");
  Serial.println(aCounter[2]);
  Serial.print(" (N/A) IN 2: ");
  Serial.print(value2f,0);
  Serial.print("     count: ");
  Serial.println(aCounter[3]);
  Serial.print(" (PSP) IN 3: ");
  Serial.print(value3f,0);
  Serial.print("     count: ");
  Serial.println(aCounter[4]);
  Serial.println("  ");

 

}



int nextA(int curVal) {
 int iNext = curVal +1;
 if(iNext > 4) iNext = 1;
 int iNext2 = 0;
 for(int i=0;i < 4;i++) {
  iNext2 = iNext++;
  if(iNext2 == 5) iNext2 = 1;
  if(aEnabled[iNext2-1] == 1) return iNext2;
 }
 return 0; // nothing enabled
}








void toggleLB() {
  Serial.print("LED Built-in Toggle is now ");
  if(digitalRead(pin_internalLED)) {
    Serial.println("OFF");
    digitalWrite(pin_internalLED, LOW);
  } else {
    Serial.println("ON");
    digitalWrite(pin_internalLED, HIGH);
  }
 
}

void toggleLed1() {
  Serial.print("LED1 Toggle is now ");
  if(digitalRead(pin_led1)) {
    Serial.println("OFF");
    digitalWrite(pin_led1, LOW);
  } else {
    Serial.println("ON");
    digitalWrite(pin_led1, HIGH);
  }
 
}

void toggleLed2() {
  Serial.print("LED2 Toggle is now ");
  if(digitalRead(pin_led2)) {
    Serial.println("OFF");
    digitalWrite(pin_led2, LOW);
  } else {
    Serial.println("ON");
    digitalWrite(pin_led2, HIGH);
  }
 
}


void toggleBuzzer() {
  Serial.print("Buzzer Toggle is now ");
  if(digitalRead(pin_buzzer)) {
    Serial.println("OFF");
    digitalWrite(pin_buzzer, LOW);
  } else {
    Serial.println("ON");
    digitalWrite(pin_buzzer, HIGH);
  }
}


void continuitySwitch() {
  Serial.print("Continuity switch is ");
  if(digitalRead(pin_continuitySwitch) == HIGH) {
    Serial.println("OPEN  (not pressed)");
  } else {
    Serial.println("CLOSED  (pressed)");
  }
}


void rocketConnect() {
  

  Serial.print("The Rocket umbilical is  ");
  if(digitalRead(pin_rocketDetect) == HIGH) {
    Serial.println("NOT CONNECTED  (open)");
  } else {
    Serial.println("CONNECTED  (closed)");
  }
}


//***************************************************  RADIO STUFF ********************************************


void RadioTXtest(){

    Serial.println("Initiating Radio Transmit Test...");
    Serial.println(" ");
    Serial1.begin(19200);
    Serial.println("Transmitting 10 packets....");
    for(int i=0; i < 10; i++) {
      Serial1.print("@M,33,Hello World hello hello hello hello hello hello hello hello hello hello hello hello hello hello hello ,!");
    }
    Serial.println("Done.");

}


void radioTest() {
  
    Serial.println("Initiating Radio Test...");
    Serial.println(" ");
    digitalWrite(pin_radioSet, HIGH); //turn radio set pin to low
    delay(50);
    Serial1.begin(9600);
    char s[5];
    strcpy(s,"");
    unsigned long testtime = millis() + 3500;
    s[0] = 0xaa;
    s[1] = 0xfa;
    s[2] = 0xaa; // aa = product model number 
    s[3] = 0x0d; //  /r termination
    s[4] = 0x0a; //  /n termination
    Serial1.println(s);
    int x = 0;
    while(x < 1) {
      checkRadio();
      if(newWord == 1) {
        Serial.print("Radio Rx:  ");
        Serial.println(theWord);
        char *ptr = strstr(theWord, "LORA6100");
        if (ptr != NULL) Serial.println("Radio Set Comms Test Successful!"); 
        digitalWrite(pin_radioSet, LOW); //turn radio set pin to low
        delay(2000);
        Serial1.begin(19200);
        Serial1.print("@M,33,Hello World,!");
        Serial1.flush();
        Serial.println(" "); 
        Serial.println("Message Sent"); 
        
        x = 1;
        
      }
      if(millis() > testtime) {
        Serial.println("Radio Timeout");
        digitalWrite(pin_radioSet, LOW); //turn radio set pin to low
        delay(200);
        Serial1.begin(19200);
        x = 2;
      }
       
    }
}

void checkRadio() {  // **********  CHECK RADIO  *****************
   newWord = 0;
   char receivedChar;
   if (Serial1.available() > 0) {
  
    unsigned long testTime = millis() + 100; //100
    
    strcpy(theWord, "");
    //Serial.println("radio inbound detect");

    while (newWord != 1) {
       if(Serial1.available()) {

         receivedChar = Serial1.read();
         //Serial.print(receivedChar);
         append(theWord, receivedChar);
         if(receivedChar == 0x0a) {
            newWord = 1;
            break;   
         }
       }
       if(millis() > testTime) {  // exit and evaluate if it takes too long
          newWord =1;
          Serial.println("RX timeout exit error");
          Serial.println(theWord);
          break;
       }
       delay(1);
     }
     Serial.println(" ");
     //ProcessRadio();
   }
}

void append(char* s, char c) {  //used to concat char to char array
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
}

void radioRead() {
  
    Serial.println("Reading Radio Configuration...");
    Serial.println(" ");
    digitalWrite(pin_radioSet, HIGH); //turn radio set pin to low
    delay(200);
    Serial1.begin(9600);
    char s[5];
    strcpy(s,"");
    unsigned long testtime = millis() + 5000;
    s[0] = 0xaa;
    s[1] = 0xfa;
    s[2] = 0x01; // 01 = read configuration 
    s[3] = 0x0d; //  /r termination
    s[4] = 0x0a; //  /n termination
    Serial.print("Results:  ");
    Serial1.println(s);
    unsigned long testTime = millis() + 5000; //100
    while(1) {
      
      if (Serial1.available() > 0) {
        Serial.print(Serial1.read());
      }
      if(millis() > testTime) break;
    }
    Serial.println(" ");
    Serial.println("Done.");
    digitalWrite(pin_radioSet, LOW); 

}

void toggleRset() {
  Serial.print("RadioSet Toggle is now ");
  if(digitalRead(pin_radioSet)) {
    Serial.println("OFF");
    digitalWrite(pin_radioSet, LOW);
  } else {
    Serial.println("ON");
    digitalWrite(pin_radioSet, HIGH);
  }
 
}


//***************************************************  FLASH STUFF ********************************************



void formatFlash() {

    if (!myfs.begin()) {
      Serial.println("LittleFS Flash:  *** ERROR starting Little FS ***");
      return;
    } else {
      Serial.println("LittleFS Flash:  Successfully started");
    }
    Serial.println(" ");
    Serial.println("Initiated a low-level format of flash...");
    myfs.lowLevelFormat('.');
    Serial.println("Done formating flash memory");

}


void printDirectory() {



  Serial.println("Printing SD Directory: ");
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("Error starting SD");
  }

  
  printSDirectory(SD.open("/"), 0);
  Serial.println();
  freeSpaces();

}

  void freeSpaces() {

  unsigned long bFree = 0;
  bFree = SD.totalSize() - SD.usedSize();
  Serial.printf("Bytes Used: %llu, Bytes Total:%llu", SD.usedSize(), SD.totalSize());
  Serial.print("    Bytes Free: ");
  Serial.println(bFree);
  Serial.println();
}

void printSDirectory(File dir, int numTabs) {
  //dir.whoami();
  uint64_t fSize = 0;
  uint32_t dCnt = 0, fCnt = 0;
  if ( 0 == dir ) {
    Serial.printf( "\t>>>\t>>>>> No Dir\n" );
    return;
  }
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      Serial.printf("\n %u dirs with %u files of Size %u Bytes\n", dCnt, fCnt, fSize);
      fTot += fCnt;
      totSize1 += fSize;
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }

    if (entry.isDirectory()) {
      Serial.print("DIR\t");
      dCnt++;
    } else {
      Serial.print("FILE\t");
      fCnt++;
      fSize += entry.size();
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println(" / ");
      printSDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
    //Serial.flush();
  }
}


void copy2SD() {
    Serial.println("Copying Flash to SD card... ");
    // Initializing the SD Card
    if(!SD.begin(BUILTIN_SDCARD)){
        Serial.println(F("SD Card Mount Failed"));
        return;
    } else {
        Serial.println(F("Good SD Card mounted"));
    }
  if (!myfs.begin()) {
    Serial.println("Error starting Flash Disk");
    return;
  }
  delay(500);
    
   uint64_t fSize = 0;
  // Cycle through directory
      File mdir;
      mdir = myfs.open("/");
    while (true) {  
      File entry =  mdir.openNextFile();
      if (! entry) {
        // no more files
        Serial.println("No more flash files to copy ");
        totSize1 += fSize;
        break;
      }
      
      if (entry.isDirectory()) {
        Serial.print("Skipping Directory: ");
        Serial.println(entry.name());
      } else {
        Serial.print("Copying File ");
        Serial.print(entry.name());
        Serial.print("   Size = ");
        fSize = entry.size();      
        Serial.println(fSize);
        // Copy it to SD
        // Delete and open from SD
        char sName[30];
        strcpy(sName,"/");  
        strcat(sName,entry.name());
        SD.remove(sName);
        tempFile = SD.open(sName, FILE_WRITE);
        // Open for reading and writing from flash
        char buf2[1];
        file = myfs.open(entry.name(), FILE_READ);
       
        while(file.available()) {         
         file.read(buf2, 1);
         tempFile.print(buf2[0]);
        }
        file.close();
        tempFile.close();        
      }
      entry.close();
    }
    
}


void eraseFlash() {
  Serial.println("Quick formatting flash...");
  myfs.quickFormat();
  //myfs.lowLevelFormat('.');
  Serial.println("Done formating flash memory");
  Serial.println(" ");

}


void flashLogging() {


    if (!myfs.begin()) {
      Serial.println("LittleFS Flash:  *** ERROR starting Little FS ***");
      return;
    } else {
      Serial.println("LittleFS Flash:  Successfully started");
    }
    Serial.println("Process:  Deleting File");
    myfs.remove(fname1);
    Serial.println("Process:  Writing File");
    strcpy(buf,"Testing");
    file = myfs.open(fname1, FILE_WRITE);
    if(file) {
      delay(10);
      file.println(buf);
      file.close();   
      Serial.println("Process:  Reading File");
      char buf2[1];
      char buf3[100];
      file = myfs.open(fname1, FILE_READ);
      bool fLetter = false;
      bool fCorrect = false;
      int bufCount = 0;
      while(file.available()) {
      file.read(buf2, 1);
      buf3[bufCount] = buf2[0];
      bufCount++;
      }
      file.close();
      char *ptr = strstr(buf3, "Test");
      if (ptr != NULL) { /* Substring found */
        Serial.println("Process:  Successful File Read");
      } else {
        Serial.println("Process:  File Read Failed");
        return;
      }
    } else {
      Serial.println("Error opening file");
      return;
    }
    Serial.println("Process:  Conducting performance test...");
    unsigned long theStop;
    int theCounter = 0;
    strcpy(buf,"Line 1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,Line 1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,Line 1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10");
    myfs.remove(fname1);
    delay(200);
    theStop = millis() + 1000;
    while(millis() < theStop) {
     file = myfs.open(fname1, FILE_WRITE);
     file.println(buf);
     file.close(); 
     theCounter++;
    }
    Serial.print("Flash Logging Speed Result = ");
    Serial.print(theCounter);
    Serial.println("  should be at least 13 (98 after low level format)");   
    
}

void readConfig() {


    if (!myfs.begin()) {
      Serial.println("LittleFS Flash:  *** ERROR starting Little FS ***");
      return;
    } else {
      Serial.println("LittleFS Flash:  Successfully started");
    }
    Serial.println("Reading Config File....");
    Serial.println("  ");
    Serial.println("  ");

    char fname[30];
    char chIn = 255;
    strcpy(fname,"/config.txt");

    file = myfs.open(fname, FILE_READ);
    if (file) {
      Serial.println("File open successful. Here are the contents:");
    } else {
      Serial.println("ERROR OPENING FILE");
      return;
    }

    while(file.available()) {
     //file.read(buf2, 1);
     //buf3[bufCount] = buf2[0];
     //bufCount++;
     char buf = file.read();
     Serial.print(buf);
    }
    file.close();

  

}

void readFile() {


    if (!myfs.begin()) {
      Serial.println("LittleFS Flash:  *** ERROR starting Little FS ***");
      return;
    } else {
      Serial.println("LittleFS Flash:  Successfully started");
    }
    Serial.println("Reading File....");
    Serial.println("  ");
    Serial.println("  ");

    char fname[30];
    char chIn = 255;
    strcpy(fname,"DAQ4.txt");

    file = myfs.open(fname, FILE_READ);
    if (file) {
      Serial.println("File open successful. Here are the contents:");
    } else {
      Serial.println("ERROR OPENING FILE");
      return;
    }

    while(file.available()) {
     //file.read(buf2, 1);
     //buf3[bufCount] = buf2[0];
     //bufCount++;
     char buf = file.read();
     Serial.print(buf);
    }
    file.close();


}

void flashCrashTest() {


    if (!myfs.begin()) {
      Serial.println("LittleFS Flash:  *** ERROR starting Little FS ***");
      return;
    } else {
      Serial.println("LittleFS Flash:  Successfully started");
    }

    unsigned long bFree = 0;
    bFree = myfs.totalSize() - myfs.usedSize();
    Serial.printf("Bytes Used: %llu, Bytes Total:%llu", myfs.usedSize(), myfs.totalSize());
    Serial.print("    Bytes Free: ");
    Serial.println(bFree);
    Serial.println();

    Serial.println("Writing till I crash....");
    Serial.println("  ");
    Serial.println("  ");


    //strcpy(buf,"Line 1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,Line 1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,Line 1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10");
    strcpy(buf,"1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,101,2,3,4,5,6,7,8,9,101,");
    
    myfs.remove(fname1);
    delay(200);
    int theCount = 0;
    elapsedMillis reportIt;

    file = myfs.open(fname1, FILE_WRITE);
    
    if(file) {
      while(1) {
      
        int testIt = file.println(buf);
        delay(1);
        if(testIt == 0) {
          file.close(); 
          Serial.println("Stopping out of disk space - TestIt 0");
          Serial.println(theCount);
          return;
        } 
        if(!file) {
          file.close(); 
          Serial.println("Stopping out of disk space - file is false");
          Serial.println(theCount);
          return;
        }  
  
        theCount++;
        if(reportIt > 30000) {
          Serial.print("Status used % is: ");
          file.close();
          delay(100);
          float used = (float) myfs.usedSize();
            float total = (float) myfs.totalSize();
            float sz = used / total;
            sz = sz * 100;
          Serial.println(sz,2);
          reportIt = 0;
          if(total == used) {
           file.close(); 
           Serial.println("Stopping total equals used");
           Serial.println(theCount);
           return;
        } 
          file = myfs.open(fname1, FILE_WRITE);
        }
    /*    if(theCount > 20000) {
            Serial.println("Exit 20k");
            file.close();
            delay(200);
            float used = (float) myfs.usedSize();
            float total = (float) myfs.totalSize();
            float sz = used / total;
            Serial.println(sz,2);
            Serial.println(myfs.usedSize());
            Serial.println(myfs.totalSize());
            return;
          }
      */    
        }
    } else {
      Serial.println("Error opening file for write");
    }
    

}

void formatUnused() {
  Serial.println("Formatting unused flash...");
  myfs.formatUnused(0,0);
  Serial.println("Done formating flash memory");
  Serial.println(" ");

}

void DAQtest() { //simulate a DAQ write 60 seconds

  Serial.println(F("DAQ Test running 60 seconds"));
  if (!myfs.begin()) {
        Serial.println(F("Flash Mount Failed"));
  }

  set_arm_clock(384'000'000); //increase MCU clock for DAQ operations - temporary to manage MCU temperature
  delay(100);
  myfs.formatUnused(0,0); // clears NAND 
  File logFile;
  char theFilename[40];
  char fileLine[100];
  strcpy(theFilename,"/DAQ8.txt");
  myfs.remove(theFilename);
  delay(3);
  logFile = myfs.open(theFilename, FILE_WRITE);
  if(logFile) {
      strcpy(fileLine, "Milliseconds,");
      // determine headers based on enabled PTs

        strcat(fileLine, "Ox");
        strcat(fileLine, ",");
        strcat(fileLine, "Fuel");
        strcat(fileLine, ",");
     logFile.println(fileLine); // write the header file
    delay(1);
  } else {
    Serial.println("Error Opening Flash file");
    return;
  }
  elapsedMillis cycleTimer = 0;
  elapsedMillis fileClock = 0;
  elapsedMillis hardSave = 0;
  elapsedMillis nextTime = 0;
  while(cycleTimer < 60000) {
   if(nextTime >= 10) { // log every ten ms
      logFile.print(fileClock);logFile.print(",");
      logFile.print(123);logFile.print(",");
      logFile.print(456);logFile.print(",");
      logFile.print(789);logFile.print(",");
      logFile.println(" ");
      if(hardSave > 10000) {
        hardSave = 0;
        logFile.close();
        delay(1);
        logFile= myfs.open(theFilename, FILE_WRITE);
        delay(1);
      }
      nextTime = 0;
   }
  }
  logFile.close();
  Serial.println("All Done Testing DAQ file creation");
}
