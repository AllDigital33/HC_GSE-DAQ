

/*   ***********  HALF CAT OPEN GSE Controller - 2024-2025 by Mike and Preston Brinker  ***********
 * 
    Pressure Transducer CODES:
    • PTK = Pressure Tank
    • PCH = Pressure Chamber
    • PLD = Pressure Load

    Valves:
    • SOX = Servo OX
    • SFU = Servo Fuel
    • SFL = Servo Fill
    • SPG = Servo Purge
 *     
 *   
 */


  bool serialDebug = false;  // set to true to get Serial output for debugging (compile 396Mhz!)
  bool debugMode = false;
  bool mute = true;  // silence the buzzer for testing

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
  #define pin_SFL 22 
  #define pin_SPG 23
  #define pin_SFU 15
  #define pin_SOX 14
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

//Teensy
  #include "Arduino.h"
  extern "C" uint32_t set_arm_clock(uint32_t frequency); // required to set MCU clock speed


//INA219 -- current and voltage sensing add
  #include <Wire.h>
  #include <Adafruit_INA219.h>
  Adafruit_INA219 ina219;

 //Servos
  #include <Servo.h> 
  Servo SFU;  
  Servo SOX;
  Servo SFL;
  Servo SPG;

  struct configStruct {  // master configuration
     // PT ranges - assume all are .5v-4.5v 
     byte PTKenabled = 1;
     int PTKrange = 500;
     float PTKzero = .5;
     int PTKalarm = 425;
     byte PCHenabled = 1;
     int PCHrange = 500;
     float PCHzero = .50;
     byte PLDenabled = 1;
     // Servos
     int SFUopen = 500;
     int SFUclose = 1200;
     int SOXopen = 500;
     int SOXclose = 1200;
     int SFLopen = 500;
     int SFLclose = 1200;
     int SPGopen = 500;
     int SPGclose = 1200;

     int pressureWaitTime = 2000; //only send changes every 2 seconds
     int pressureMinChange = 4;
      
     int CPUtempAlarm = 165;
     int DAQauto = 1;
     int DAQtimeout = 50000;
     
             
  };
  configStruct configuration;

  struct localStruct {
    // used for local code configuration / non-configurable
    int battlow = 15;
    int logRate = 10; // 100 hz (10ms pause)
    int DAQsuperTimeout = 180000; // three minutes super timeout
    int DAQflush = 10000; // millis to safety close/open/flush file
    int FTPsize = 100; // FTP packet size - increase sendChunk below
    bool radioLog = true; // local config to enable radio logging
    int ADCint = 6; // interval for reading ADC units in millis 6=115sps ZZZ
    unsigned long mvInt = 500; // interval to read main volts in ADC
    unsigned long hzInt = 500; // interval to read haz volts from ADC
    elapsedMillis hzTimer = 0;
    elapsedMillis mvTimer = 0;
    unsigned long ADCwait = 2000; // microseconds to wait for ADC to sample
    int servo6vhold = 2000; //turn on servo power for 2 seconds when moving them
   
  };
  localStruct local;


  struct timerStruct {  // timer variables
    unsigned long radioRXTimer = 0;
    unsigned long radioTXTimer = 0;
    unsigned long radioSendCallSignTimer = 90000;
    unsigned long healthTimer = 0;
    unsigned long healthResend = 0;
    int radioRXInterval = 300;
    int radioTXInterval = 1000;
    int radioSendCallSignInterval = 90000;
    unsigned long housekeepingInput = 0;
    int housekeepingInputInterval = 500;  // set to 500 ms for now
    unsigned long fullStatusTimer = 0;
    int fullStatusInterval = 10000; // send full status every 10 seconds (and when an event)
    unsigned long pressureTimer = 0;
    unsigned long igniterTimer = 0;
    unsigned long LineTimer = 0;
    unsigned long QDoff = 0;
    unsigned long oxLineTimer = 0;
    unsigned long mainOff = 0;
    unsigned long mainOn = 0;
    unsigned long health = 0; // error check
    elapsedMillis errorBlink = 0;
    unsigned long timer6v = 0; 
    elapsedMillis processPF60 = 0;
    elapsedMillis processPurge = 0;
    elapsedMillis servoTimer = 0;
  };
  timerStruct timers;

  struct workingStruct {  // working variables

    char statusSentenceString[200]; // full status to send
    char configString[200]; // full config send
    int lastPTK = 0;
    int lastPCH = 0;
    int lastPLD = 0;
    bool PTKzero = false;
    bool PCHzero = false;
    bool PLDzero = false;

    // DAQ, Logging, and File TX and Flash
    byte DAQstart = 0; // 1 = start, 2 = stop
    byte DAQrecording = 0;
    byte DAQstudent = 1;
    char DAQfilename[20];
    char rLogFilename[20];
    int flashFormat = 0;
    char code[6];
    int confvalue;
    int radioLog = 0;
    char radioLogString[200];
    //FTP send controls
    volatile bool requestDir = false;
    int dirFileCount = 0;
    int totalPackets = 0;
    volatile bool sendFlag = false;
    char sendFile[20];
    char sendChunk[110];
    int currentPacket = 0;
    bool ftpMode = false;
    bool getFile = false;
    elapsedMillis superTimeout = 0;  
    bool beepErrors = false; // for first health check beeping
    int logCache = 0; //cache radio inbound during DAQ 
    char logCacheStr[25][200];
    bool writeConfig = false;
    bool SDcopy = false;
    bool SD = false; // is there a valid SD card?
    unsigned long fillTime = 60;
    unsigned long purgeTime = 10;
    bool radioOff = false;
    bool servoTest = false;

  };
  workingStruct working;

  struct errorsStruct {

    int mainBatt = 2;
    int MCUtemp = 2;
    int PT = 2;
    int radio = 2;
    int flashMem = 2;
    int SD = 2;
    int ADC = 2;
    int MCUcrash = 2;
    int errorCount = 0;
    int fuse5v = 2;
    int fuseMain = 2;
  };
  errorsStruct errors;

  struct padStatusStruct {

    //new PT values 
    int PTK = 0;  // Tank
    int PLD = 0;  // Load
    int PCH = 0;  // Chamber
    byte rocketConnected = 0;
    byte padHot = 0; //pad safety plug in/out (haz 24v) 
    byte armedState = 0;
    byte igniterContinuity = 0;
    int mainVolts = 0;
    byte SOX = 0; //closed = 0, open = 1
    byte SFU = 0;
    byte SFL = 0;
    byte SPG = 0;
    byte processFill60 = 0; // can be 0- not run, 1 - complete, 2 - running, 3-stopped
    byte processPurge = 0;
    byte processLaunch = 0;
    byte processAbort = 0;
    int CPUtemp = 0;


  };
  padStatusStruct padStatus;


// *** Radio Setup
  char radioHeader[7];
  char radioMessage[150];
  char radioMessageS[200];
  int newWord = 0;
  char theWord[320];

//added for send queue
  char rSend[15][300];
  byte rSendCount = 0;
  byte rSendLast = 0;
  byte rSendPos = 0;
  char vFullMessageR[300];

  //*************  ADC Sampling setup  ********************
  //ADS1115 ADC
  #include "ADS1115-Driver.h"
  ADS1115 ads1115a = ADS1115(0x49);
  unsigned long mainVoltageTimer = 0;
  unsigned long PTKtimer = 0;
  unsigned long PCHtimer = 0;
  unsigned long PLDtimer = 0;
  float valueADC = 0.0;
  int rotateA = 1;
  elapsedMicros rotateAwait = 0;

  //*************  DAQ and File handling ********************
  // For the flash memory and logging
  //Little FS flash
  //#include <LittleFS.h>
  //LittleFS_QPINAND myfs; // used for fully loaded T4.1

  #include <stdarg.h>
  #include <TimeLib.h>  // for logging time
  // just for SD
  #include "SD.h"
  #include <SPI.h>   
  File configFile;
  File logFile;
  File rLogFile;
  File cfile;
  char theFilename[40];
  char fileLine[100];
  unsigned long sTimeout = 0;
  unsigned long DAQnextTime = 0;
  unsigned long hardSave = 0;
  elapsedMillis fileClock = 0;

// Temperature from CPU
  extern float tempmonGetTemp(void);  



//***********************************************************     SETUP    ************************************************************
//***********************************************************     SETUP    ************************************************************


void setup() {

  Serial.begin(115200);
  if(CrashReport) { // capture if the MCU core dumps
    if(serialDebug) Serial.print(CrashReport);
    errors.MCUcrash = 1;
  } else {
    errors.MCUcrash = 0;
  }
  // Set MCU clock to 150Mhz for thermal management. Set higher when DAQ on.
  set_arm_clock(150'000'000);
  delay(50);

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
  pinMode(pin_ext5v, OUTPUT);digitalWrite(pin_ext5v, HIGH); //Start off the 5v high - only for protection
  pinMode(pin_ext6v, OUTPUT);digitalWrite(pin_ext6v, LOW);
  pinMode(pin_fan, OUTPUT);digitalWrite(pin_fan, LOW);
  pinMode(pin_digSpare, OUTPUT);digitalWrite(pin_digSpare, LOW);

  SFU.attach(pin_SFU); 
  SOX.attach(pin_SOX);
  SPG.attach(pin_SPG);
  SFL.attach(pin_SFL);
  delay(500);

  // Power on override using continuity switch for copy Flash to SD card ========
  //if(digitalRead(pin_continuitySwitch) == LOW) SDcopy(); // call SD copy then infinate loop


  //*************  ADC Sampling setup  ********************
  // ADS1115 
  Wire.begin();
  delay(1);
  ads1115a.reset();
  delay(100);
  
  ads1115a.setDeviceMode(ADS1115_MODE_SINGLE);
  ads1115a.setDataRate(ADS1115_DR_860_SPS);
  ads1115a.setPga(ADS1115_PGA_6_144);
  uint8_t tmpHealth = 0;
  tmpHealth = ads1115a.healthTest(); // reads config
  if(tmpHealth != 227) errors.ADC = 1; 
  delay(100);

  Wire.setClock(1000000); //speedy gonzales
  delay(100);  
  for (int i = 0; i < 10; i++) {
    sampleLoop();delay(2);  // Do first ADC sampling function of all ADC ports
  }

  // *************** DAQ and File Management ********************
  // SD Card setup
  if(!SD.begin(BUILTIN_SDCARD)){
      if(serialDebug) Serial.println(F("SD Card Mount Failed - ignoring SD conf for now"));
      working.SD = false;
      errors.SD = 1;
  } else {
      if(serialDebug) Serial.println(F("Good SD Card mounted"));
      working.SD = true;
      errors.SD = 0;
  }
  if(working.SD) readSDconfig(); 


  // Setup the radioLog file (rotating sequence of ten logs)
  if(local.radioLog && working.SD){
    getRlog();
  }
  if(serialDebug) printConfig();
  delay(5000); // give radio time to power up


  //pre-run some checks
  CPUtemp();
  checkRocketConnect(); 
  checkVoltage(); 
  fuseCheck();
  checkPressure(); 

  // Initialize Radio 
  radioTest();
  Serial1.begin(19200);

  //Close all valves on startup
  valve("SFL",0); 
  valve("SOX",0); 
  valve("SPG",0); 
  valve("SFU",0); 

  if(serialDebug) Serial.println("Starting up....");
  checkHealth();
  timers.healthTimer = millis() + 30000;

}

//***********************************************************        MAIN LOOP    ************************************************************
//***********************************************************        MAIN LOOP    ************************************************************

void loop() {

  if(millis() > timers.radioRXTimer) {
    checkRadio();
    timers.radioRXTimer = millis() + timers.radioRXInterval;
    
  }

  if(millis() > timers.radioTXTimer && rSendCount > 0) {
    RadioSendQueue();
    timers.radioTXTimer = millis() + timers.radioTXInterval; //1 sec default spacing
  }

  sampleLoop();  // ADC sampling functions
  DAQlogging();

  if(millis() > timers.housekeepingInput) { // every 500ms
    checkPressure();                // Check Pressures
    fuseCheck();                    // Check fuses on PCB
    checkRocketConnect();           // Check Rocket Detect
    checkIgniterContinuity();       // Check igniter continuity
    checkVoltage();                 // Check voltages and pad hot 
    checkContinuitySwitch();        // Check Continuity switch
    igniterCheck();                 // Check igniter to turn off
    processCheck();                 // Check the active processes
    servoTestCheck();               // For testing servo positions
    timers.housekeepingInput = millis() + timers.housekeepingInputInterval;
  }

  if(millis() > timers.health) {
     checkHealth();                  // check for new errors
     timers.health = millis() + 5000;
  }

  if(millis() > timers.fullStatusTimer) {
    sendFullStatus();
  }

  if(errors.errorCount > 0 && timers.errorBlink > 250) {blinkError();}


}

//***********************************************************        MAIN LOOP END       ************************************************************
//***********************************************************        MAIN LOOP END       ************************************************************

void servoTestCheck() {

  if(working.servoTest) {
    if(timers.servoTimer > 1000) { // wait one second
        working.servoTest = false;
        float endC = 0.0;
        endC = ina219.getCurrent_mA();
        if(serialDebug) {Serial.print(F("Servo ending Current: "));Serial.println(endC,2);}
        strcpy(radioHeader, "#M");
        if(endC > 1000) {
            strcpy(radioMessage, "Servo Test FAILED with ");
        } else {
            strcpy(radioMessage, "Servo Test SUCCEEDED with ");
        }
            char floatStr[20];
            dtostrf(endC, 6, 2, floatStr);
            strcat(radioMessage,floatStr);
            RadioSend(); 
    }
  }

}




void processCheck() {

  if(padStatus.processFill60 == 2) {  // check Fill 60 process
    if(timers.processPF60 >= (working.fillTime * 1000)) {
        valve("SFL",0); //close fill valve
        padStatus.processFill60 = 1;
        sendFullStatus(); 
        strcpy(radioHeader, "#M");
        strcpy(radioMessage, "Fill COMPLETE");
        RadioSend(); 
    }
  }   

  if(padStatus.processPurge == 2) {  // check Purge 10 process
    if(timers.processPurge >= (working.purgeTime * 1000)) {
        valve("SPG",0); //close purge valve
        padStatus.processPurge = 1;
        sendFullStatus(); 
        strcpy(radioHeader, "#M");
        strcpy(radioMessage, "Purge Complete");
        RadioSend(); 
    }
  }   

  if(timers.timer6v > 0) {
    if(millis() > timers.timer6v) {
      SFU.detach(); 
      SOX.detach();
      SPG.detach();
      SFL.detach();
      timers.timer6v = 0;
      digitalWrite(pin_ext6v, LOW); //turn off servo power
    }
  }
}

void fuseCheck() {
   if(digitalRead(pin_fuseMain) == LOW) {
     errors.fuseMain = 0;
   } else {
     errors.fuseMain = 1;
   }  
}

void checkContinuitySwitch() {   //physical pad switch

  int tempRead = 0;
  tempRead = digitalRead(pin_continuitySwitch);
  if(tempRead == 0) {  // button pressed down
      checkIgniterContinuity(); 
      if(padStatus.igniterContinuity == 1) {
        if(serialDebug) Serial.println(F("Good Continuity"));
        digitalWrite(pin_led2, HIGH);
        if(!mute) digitalWrite(pin_buzzer, HIGH);
        delay(2000);
        digitalWrite(pin_led2, LOW);
        digitalWrite(pin_buzzer, LOW);
      } else {
        if(serialDebug) Serial.println(F("Bad Continuity"));
        digitalWrite(pin_led1, HIGH);
        delay(250);
        digitalWrite(pin_led1, LOW);
        delay(250);
        digitalWrite(pin_led1, HIGH);
        delay(250);
        digitalWrite(pin_led1, LOW);
        delay(250);
        digitalWrite(pin_led1, HIGH);
        delay(500);
        digitalWrite(pin_led1, LOW);
      }
  }
}
void igniterCheck() { // turns off the igniter when it is on
  if(timers.igniterTimer > 0) {
    if(millis() > timers.igniterTimer) {
      digitalWrite(pin_igniterFIRE, LOW);
      timers.igniterTimer = 0;
      checkIgniterContinuity();
    }
  }
}

void blinkError() {
  if(digitalRead(pin_led1) == LOW) {
    digitalWrite(pin_led1, HIGH);
  } else {
    digitalWrite(pin_led1, LOW);
  }
  timers.errorBlink = 0;
}

void checkVoltage() {   //also checks pad hot from haz voltage

  int haz = analogRead(pin_hazVolts);
      //.   >150 is on

  if(padStatus.padHot == 0 && haz > 150) {  // the pad just got hot
      padStatus.padHot = 1;
    //send a full status
    if(serialDebug) Serial.println(F("Pad is now HOT"));
    sendFullStatus();  
    //send a message
    strcpy(radioHeader, "#M");
    strcpy(radioMessage, "The Pad is now HOT");
    RadioSend();       
  }
  
  if(padStatus.padHot == 1 && haz <= 150) {  // the pad just got cold
      padStatus.padHot = 0;
      padStatus.armedState = 0;
      valve("SFU",0);
      valve("SOX",0);
      valve("SFL",0);
      valve("SPG",0);
      if (padStatus.processFill60 == 2) padStatus.processFill60 = 0; // stop  processes if running
      if (padStatus.processPurge == 2) padStatus.processPurge = 0; // stop  processes if running
      if (padStatus.processLaunch == 2) padStatus.processLaunch = 0; // stop  processes if running
      delay(50);
      checkIgniterContinuity();  
      //send a full status
      sendFullStatus();
      statusSentence(); // set the status sentence
      if(serialDebug) Serial.println(F("Pad is now COLD"));   
      //send a message
      strcpy(radioHeader, "#M");
      strcpy(radioMessage, "The Pad is now COLD");
      RadioSend();       
  }


  // health checking batteries
  
    if(padStatus.mainVolts < local.battlow) {
      errors.mainBatt = 1;
    } else {
      errors.mainBatt = 0;
    }

// zzz add current checking?
  
}

void checkIgniterContinuity() { //new

  int tempRead = 0;
  tempRead = digitalRead(pin_igniterContinuity);
  
  if(padStatus.igniterContinuity == 0 && tempRead == 0) {  // the igniter just got continuity
    padStatus.igniterContinuity = 1;
    //send a full status
    sendFullStatus();
    if(serialDebug) {
      Serial.print(F("Igniter now has continuity - reading: "));
      Serial.println(tempRead);
    }
    //send a message
    strcpy(radioHeader, "#M");
    strcpy(radioMessage, "The Igniter now has continuity");
    RadioSend();        
  }

  if(padStatus.igniterContinuity == 1 && tempRead == 1) {  // the igniter just lost continuity
    padStatus.igniterContinuity = 0;
    //send a full status
    sendFullStatus();
    if(serialDebug) {
      Serial.print(F("Igniter has lost continuity - reading: "));
      Serial.println(tempRead);
    }
    //send a message
    strcpy(radioHeader, "#M");
    strcpy(radioMessage, "The Igniter has lost continuity");
    RadioSend();       
  }
}

void CPUtemp() {

      float CPUtemp = 0.0;
      CPUtemp  = (tempmonGetTemp() * 9.0f / 5.0f) + 32.0f;
      padStatus.CPUtemp = (int)CPUtemp;
      if(padStatus.CPUtemp > configuration.CPUtempAlarm) {
        errors.MCUtemp = 1;
      } else {
        errors.MCUtemp = 0;
      }
}

void checkRocketConnect() {

  byte tempDetect;

  tempDetect = digitalRead(pin_rocketDetect);

  if(tempDetect == 0 && padStatus.rocketConnected == 0) {  //we just got connected
    padStatus.rocketConnected = 1;
    //send a full status
    sendFullStatus();
    if(serialDebug) Serial.println(F("Rocket now connected"));
    //send a message
    strcpy(radioHeader, "#M");
    strcpy(radioMessage, "Rocket is now connected");
    RadioSend();  
  }

  if(tempDetect == 1 && padStatus.rocketConnected == 1) {  //we just got disconnected
    padStatus.rocketConnected = 0;
    //send a full status
    sendFullStatus();
    if(serialDebug) Serial.println(F("Rocket Disconnected"));
    //send a message
    strcpy(radioHeader, "#M");
    strcpy(radioMessage, "Rocket is now disconnected");
    RadioSend();  
  }
}


void checkPressure() {

/*  
 *  New Structure
      Values are no in: 
        padStatus.POX
        padStatus.PFL
        padStatus.PPS
        padStatus.PRS
       
 */
 
  bool sendUpdate = false;

  if((padStatus.PTK > (working.lastPTK + configuration.pressureMinChange) || padStatus.PTK < (working.lastPTK - configuration.pressureMinChange))  && configuration.PTKenabled == 1) sendUpdate = true;


  // only send updates every two seconds and if > 2 
  if(millis() > timers.pressureTimer && sendUpdate == true) {

      char pressureString[100];
      char comma[5] = ",";
      char str_int[16];
    
      strcpy(pressureString, "");  //zero out the string
      //------------ START ------------------
      strcpy(str_int, ""); sprintf (str_int, "%d" , padStatus.PTK);
      strcat(pressureString, str_int);strcat(pressureString, comma);
      //-------------
      strcpy(str_int, ""); sprintf (str_int, "%d" , padStatus.PCH);
      strcat(pressureString, str_int);strcat(pressureString, comma);
      //-------------
      strcpy(str_int, ""); sprintf (str_int, "%d" , padStatus.PLD);
      strcat(pressureString, str_int);
      //-------------            

     if(serialDebug) {
      Serial.print(F("Sending new pressure data:  ")); Serial.println(pressureString);
     }
     strcpy(radioHeader, "#PRS");   
     strcpy(radioMessage, pressureString);
     RadioSend(); 
     timers.pressureTimer = millis() + configuration.pressureWaitTime;
     
     working.lastPTK = padStatus.PTK;
     working.lastPCH = padStatus.PCH;
     working.lastPLD = padStatus.PLD;


  }  
}

void sendFullStatus() {
      // Send full status
    statusSentence(); // set the status sentence
    if(serialDebug) Serial.println(F("Sending STATUS"));
    strcpy(radioHeader, "#FS");
    strcpy(radioMessage, working.statusSentenceString);
    RadioSend();  
    timers.fullStatusTimer = millis() + timers.fullStatusInterval;
}


void statusSentence() {

  char comma[5] = ",";
  char str_int[16];

  strcpy(working.statusSentenceString, "");  //zero out the string
  //------------ START ------------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , padStatus.rocketConnected);
  strcat(working.statusSentenceString, str_int);strcat(working.statusSentenceString, comma);
  //-------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , padStatus.padHot);
  strcat(working.statusSentenceString, str_int);strcat(working.statusSentenceString, comma);
  //-------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , padStatus.armedState);
  strcat(working.statusSentenceString, str_int);strcat(working.statusSentenceString, comma);
  //-------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , padStatus.igniterContinuity);
  strcat(working.statusSentenceString, str_int);strcat(working.statusSentenceString, comma);
  //-------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , padStatus.mainVolts);
  strcat(working.statusSentenceString, str_int);strcat(working.statusSentenceString, comma);
  //-------------
    strcpy(str_int, ""); sprintf (str_int, "%d" , padStatus.PTK);
  strcat(working.statusSentenceString, str_int);strcat(working.statusSentenceString, comma);
  //-------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , padStatus.PCH);
  strcat(working.statusSentenceString, str_int);strcat(working.statusSentenceString, comma);
  //-------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , padStatus.PLD);
  strcat(working.statusSentenceString, str_int);strcat(working.statusSentenceString, comma);
  //-------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , padStatus.SFU);
  strcat(working.statusSentenceString, str_int);strcat(working.statusSentenceString, comma);
  //-------------    
  strcpy(str_int, ""); sprintf (str_int, "%d" , padStatus.SOX);
  strcat(working.statusSentenceString, str_int);strcat(working.statusSentenceString, comma);
  //-------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , padStatus.SFL);
  strcat(working.statusSentenceString, str_int);strcat(working.statusSentenceString, comma);
  //-------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , padStatus.SPG);
  strcat(working.statusSentenceString, str_int);strcat(working.statusSentenceString, comma);
  //-------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , padStatus.processFill60);
  strcat(working.statusSentenceString, str_int);strcat(working.statusSentenceString, comma);
  //-------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , padStatus.processLaunch);
  strcat(working.statusSentenceString, str_int);strcat(working.statusSentenceString, comma);
  //-------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , padStatus.processPurge);
  strcat(working.statusSentenceString, str_int);strcat(working.statusSentenceString, comma);
  //-------------   
  strcpy(str_int, ""); sprintf (str_int, "%d" , padStatus.CPUtemp);
  strcat(working.statusSentenceString, str_int);
  //-------------  

  //-------------  END 

     working.lastPTK = padStatus.PTK;
     working.lastPCH = padStatus.PCH;
     working.lastPLD = padStatus.PLD;


}

void retrySD() {

  if(!SD.begin(BUILTIN_SDCARD)){
      if(serialDebug) Serial.println(F("SD Card Retry Mount Failed"));
      working.SD = false;
      errors.SD = 1;
  } else {
      if(serialDebug) Serial.println(F("Good SD Card mounted"));
      working.SD = true;
      errors.SD = 0;
  }

}

void checkHealth() {

    CPUtemp();
  
    if(padStatus.PTK == 99999 && configuration.PTKenabled == 1) {
      errors.PT = 1;
    } else {
      errors.PT = 0;
    }
  
    int theCount = 0;
    bool sendNow = false;
  
    if(errors.mainBatt == 1) theCount++;
    if(errors.MCUtemp == 1) theCount++;
    if(errors.PT == 1) theCount++;
    if(errors.radio == 1) theCount++;
    if(errors.SD == 1) theCount++;
    if(errors.ADC == 1) theCount++;
    if(errors.MCUcrash == 1) theCount++;
    if(errors.fuseMain == 1) theCount++;

    if(errors.errorCount != theCount) sendNow = true;
    errors.errorCount = theCount;
    if(errors.errorCount > 0) { // resend to confirm every 30 seconds
      if(millis() > timers.healthResend) {
        sendNow = true;
        timers.healthResend = millis() + 30000;
        if(errors.SD == 1) retrySD();
      }
    }
    if(errors.errorCount == 0) digitalWrite(pin_led1, HIGH);

    if(sendNow || millis() > timers.healthTimer) {



      //------------ Startup Beeps -------------
      if(!working.beepErrors) {  // Three beeps is A-OK
       if(errors.errorCount == 0) {
         digitalWrite(pin_led2, HIGH);
         beepCount(3);
         digitalWrite(pin_led2, LOW);
       } else {
          digitalWrite(pin_led1, HIGH);
          if(!mute) digitalWrite(pin_buzzer, HIGH);
          delay(3000);
          digitalWrite(pin_buzzer, LOW);
          delay(1000);
          if(errors.mainBatt == 1) {beepCount(2); delay(1000);}
          if(errors.SD == 1) {beepCount(3); delay(1000);}
          if(errors.MCUtemp == 1) {beepCount(5); delay(1000);}
          if(errors.PT == 1) {beepCount(1); delay(1000);}
          if(errors.radio == 1) {beepCount(6); delay(1000);}
          if(errors.ADC == 1) {beepCount(8); delay(1000);}
          if(errors.MCUcrash == 1) {beepCount(9); delay(1000);}
          if(errors.fuseMain == 1) {beepCount(10); delay(1000);}
          digitalWrite(pin_led1, LOW);
       }
        working.beepErrors = true;
      }
      
    
      if(serialDebug) Serial.print(F("Health Check Error Count is: "));
      if(serialDebug) Serial.println(errors.errorCount);
  
      char healthString[100];
      char comma[5] = ",";
      char str_int[16];
    
      strcpy(healthString, "");  //zero out the string
      //------------ START ------------------
      strcpy(str_int, ""); sprintf (str_int, "%d" , errors.errorCount);
      strcat(healthString, str_int);strcat(healthString, comma);
      //-------------
      strcpy(str_int, ""); sprintf (str_int, "%d" , errors.mainBatt);
      strcat(healthString, str_int);strcat(healthString, comma);
      //-------------
      strcpy(str_int, ""); sprintf (str_int, "%d" , errors.MCUtemp);
      strcat(healthString, str_int);strcat(healthString, comma);
      //-------------
      strcpy(str_int, ""); sprintf (str_int, "%d" , errors.PT);
      strcat(healthString, str_int);strcat(healthString, comma);
      //-------------
      strcpy(str_int, ""); sprintf (str_int, "%d" , errors.radio);
      strcat(healthString, str_int);strcat(healthString, comma);
      //-------------                         
      strcpy(str_int, ""); sprintf (str_int, "%d" , errors.SD);
      strcat(healthString, str_int);strcat(healthString, comma);
      //-------------
      strcpy(str_int, ""); sprintf (str_int, "%d" , errors.ADC);
      strcat(healthString, str_int);strcat(healthString, comma);
      //-------------
      strcpy(str_int, ""); sprintf (str_int, "%d" , errors.MCUcrash);
      strcat(healthString, str_int);strcat(healthString, comma);
      //-------------
      strcpy(str_int, ""); sprintf (str_int, "%d" , errors.fuseMain);
      strcat(healthString, str_int);
      //-------------

     if(serialDebug) {
      Serial.print(F("Sending health data:  ")); Serial.println(healthString);
     }
     strcpy(radioHeader, "#HER");   
     strcpy(radioMessage, healthString);
     RadioSend(); 
     timers.healthTimer = millis() + 60000;
    }
  
}

void beepCount(int num) {
  for (int i = 0; i <= (num-1); i++) {
        if(!mute) digitalWrite(pin_buzzer, HIGH);
        delay(500);
        digitalWrite(pin_buzzer, LOW);
        delay(500);
  }  
}



void parseit(char *record, char arr[20][20]) {
  
  if(serialDebug) Serial.println(F("Starting to parse inbound data"));
  const byte segmentSize = 19 ;  // Segments longer than this will be skipped
  char scratchPad[segmentSize + 1]; // A buffer to pull the progmem bytes into for printing/processing
  int i = 0;
  // declare three pointers to the progmem string
  char * nextSegmentPtr = record; // points to first character of segment
  char * delimiterPtr = record;   // points to detected delimiter character, or eos NULL
  char * endOfData = record + strlen(record); // points to last character in string
  byte len; // number of characters in current segment
  while (1) {
    delimiterPtr = strchr_P(nextSegmentPtr, ','); // Locate target character in progmem string.
    len = delimiterPtr - nextSegmentPtr;
    if (delimiterPtr == nullptr) { // Hit end of string
      len = endOfData - nextSegmentPtr;
    }
    if (len <= segmentSize) {
      memcpy_P(scratchPad, nextSegmentPtr, len);
      scratchPad[len] = '\0'; // Append terminator to extracted characters.
      strcpy(arr[i],scratchPad); 
    }
    else {
      strcpy(arr[i],"overflow");       
    }
    if (delimiterPtr == nullptr) { // ----- Exit while loop here -----
      break;
    }
    i++;
    nextSegmentPtr = nextSegmentPtr + len + 1;
  } // end while  
}



//***********************************************************        RADIO LOGIC    ************************************************************
//***********************************************************        RADIO LOGIC     ************************************************************

void radioTest() {
  
    digitalWrite(pin_radioSet, HIGH); //turn radio set pin to low
    delay(200);
    Serial1.begin(9600);
    char s[5];
    strcpy(s,"");
    s[0] = 0xaa;
    s[1] = 0xfa;
    s[2] = 0xaa; // aa = product model number 
    s[3] = 0x0d; //  /r termination
    s[4] = 0x0a; //  /n termination
    Serial1.println(s);

    radioTestRead();
    if(serialDebug) Serial.println(theWord);
    char *ptr = strstr(theWord, "LORA6100");
    char *ptr2 = strstr(theWord, "LORA611");
    if (ptr == NULL && ptr2 == NULL) { // test for 100mw or 1w versions
      errors.radio = 1; 
      if(serialDebug) Serial.println("ERROR:  No Radio Found");
    } else {
      errors.radio = 0;
    }
    digitalWrite(pin_radioSet, LOW);
    delay(200);
      
}

void radioTestRead() {  
  
   newWord = 0;
   char receivedChar;
   unsigned long testTime = millis() + 500;    
   strcpy(theWord, "");
   while (newWord != 1) {
      if(Serial1.available()) {
        receivedChar = Serial1.read();
        append(theWord, receivedChar);
      if(receivedChar == 0x0a) {
           newWord = 1;
           break;   
         }
       }
       if(millis() > testTime) {  // exit and evaluate if it takes too long
          newWord = 1;
          break;
       }
       delay(1);
     }
}

void append(char* s, char c) {  //used to concat char to char array
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
}

void RadioSend() {  //New and improved
  //populate global radioHeader (e.g. #M) and radioMessage first
    strcpy(radioMessageS, "");
    strcat(radioMessageS, radioHeader);
    strcat(radioMessageS, ",");    
    strcat(radioMessageS, radioMessage);      
    strcat(radioMessageS, ",!");
    //done creating string
    //add it to the send queue n times
    rSendCount ++;
    rSendPos ++;
    if (rSendPos == 11) rSendPos = 1;
    strcpy(rSend[rSendPos], radioMessageS);  
    //now log the message to FLASH
    logIt(radioMessageS,0);
}

void RadioSendQueue () {

  // this is designed to cut down on send congestion. Buffer packets 1000ms apart so they don't mangle together
  // Globals:  String rSend[10];  byte rSendCount = 0;  rSendLast = 0; rSendPos = 0; RadioSendQueueRate = 1000
   if(rSendCount > 0) {
     digitalWrite(pin_led2, HIGH);
     if (Serial1.available() > 0) { checkRadio(); } // don't send if inbound packets
     //digitalWrite(pin_led2, HIGH); //blink onboard light
     rSendLast ++;
     if (rSendLast == 11) rSendLast = 1;
     if(!working.radioOff) { //used to silence radio transmission but not lose logging
       Serial1.print(rSend[rSendLast]);
       Serial1.flush(); //waits for outgoing transmission to complete
     }
     rSendCount = rSendCount - 1; 
     //digitalWrite(pin_led2, LOW);
     if(serialDebug) Serial.print("TX:  ");
     if(serialDebug) Serial.println(rSend[rSendLast]);
     digitalWrite(pin_led2, LOW);
   }
}



void checkRadio() {  // **********  CHECK RADIO  *****************

   newWord = 0;
   char receivedChar;
   byte vtimeout = 0;
   int wCount = 0;
   byte rangeError = 0;
   byte lengthError = 0;
   byte ignoreError = 0;
      
   if (Serial1.available() > 0) {
  //  Serial.println("Rx...");
    unsigned long testTime = millis() + 2000;
    strcpy(theWord, "");
    if(serialDebug) Serial.println("radio inbound detect");
    digitalWrite(pin_led2, HIGH);

    while (newWord != 1) {
       if(Serial1.available()) {

         receivedChar = Serial1.read();
         if(receivedChar == 33) {  // look for ! to end
          newWord = 1;
          append(theWord, receivedChar);
          //Serial1.read();  // read the extra end char ?
         } else {
           append(theWord, receivedChar);
           wCount++;
         }
         // --------- Error checking -------------------------
         if(receivedChar == 64) { // Ignore all sentences from RocketTalk flight computer
           ignoreError = 1;
           newWord = 1;
           if(serialDebug) {Serial.print("Ignoring RocketTalk radio traffic");}
         }
         if(wCount > 145) { // too long without exit
            lengthError = 1;
            newWord = 1; //abort
            if(serialDebug) {Serial.print("Radio BAD RX: ");Serial.println(theWord);}
         }
         if(receivedChar < 32 || receivedChar > 126) {  //noise or garbage on the radio - empty and abort to not waste cycles
            rangeError = 1;
            newWord = 1;
            if(serialDebug) {Serial.print("Radio BAD RX: ");Serial.println(theWord);}
         }
       }
       if(millis() > testTime) {  // exit and evaluate if it takes too long
          newWord =1; //force exit
          vtimeout = 1;
       }
       //delay(1);
     } //end while 
     
     digitalWrite(pin_led2, LOW);
     // Error processing
     if(rangeError == 1) {
       if(serialDebug) Serial.print("**** Radio RX out of range - noise ignored ****");
       Serial1.clear(); //clear the entire buffer
       while(Serial1.available()) {
              receivedChar = Serial1.read();
              if(millis() > testTime) break;
            }
     }
     if(lengthError == 1) {
       if(serialDebug) Serial.print("**** Radio RX length issue - purging ****");
       Serial1.clear(); //clear the entire buffer
       while(Serial1.available()) {
              receivedChar = Serial1.read();
              if(millis() > testTime) break;
            }
     }  
     if(ignoreError == 1) {
       Serial1.clear(); //clear the entire buffer
       while(Serial1.available()) {
              receivedChar = Serial1.read();
              if(millis() > testTime) break;
            }
     }         
             
     if(vtimeout == 1 || ignoreError == 1) {
       if(serialDebug) Serial.println("timeout or ignore exit error - aborting...");
       if(serialDebug) Serial.println(theWord);  
     } else {
       if(serialDebug) {Serial.print("Radio RX: ");Serial.println(theWord);}
       ProcessRadio();
     }
   } 
}


void ProcessRadio() {   // **********  PROCESS RADIO COMMAND  *****************

      int chkNew = 0;
      if(theWord[0] == 35 && theWord[strlen(theWord)-1] == 33) {   // proper sentence recieved from radio # !          
          chkNew = 1;
          

      } else {  // error
              if(serialDebug) {Serial.print("Bad RX Sentence: ");Serial.println(theWord);}
              char wordLog[340];
              strcpy(wordLog,"BAD RX:  ");
              strcat(wordLog, theWord);
              logIt(wordLog,1);
              strcpy(theWord, "");
              return;
      }
      // **************  SUMMARY OF INBOUND RADIO COMMANDS  ********************
     /*
        #DAQSON	  DAQ on from simple remote
        #DAQOFF	  DAQ off - stop recording
        #S	      Request Status
        #FCFG1    Set full configuration values first half
        #FCFG2.   second half (64b limit with red cable)
        #RSET	    Reset the pad controller
        #DEFAULT	Default all configuration
        #OSFU	    Open SFU
        #OSOX	    Open SOX
        #OSFL	    Open SFL
        #OSPG	    Open SPG
        #OFAO     Open SFU and SOX
        #CSFU	    Close SFU
        #CSOX	    Close SOX
        #CSFL	    Close SFL
        #CSPG	    Close SPG
        #CLOSE    Close all valves
        #ARM1	    Command to ARM
        #ARM0	    Command to Disarm
        #FIRE	    Fire the Igniter
        #PFILL    Automatic Process - Fill for N seconds (#PFILL0 to stop)
        #PPRG     Automatic Process - Purge 10  (#PPRG0 to stop)
        #PLCH     Automatic Process - Launch (close valve, igniter, wait, valves)
        #ABORT    Automatic Process Close All and Safe - Abort
        #ZPTK	    Zero Tank PT
        #ZPCH	    Zero Chamber PT
        #TSERV    Test servo positioning (#TSERV,XXX,VALUE,!) XXX=the valve
      */
      
      if (chkNew == 1) {  // Real processing goes here            ********************************  RADIO PROCESSING  ******************************

          //---------------- LOG THE MESSAGE FIRST -----------------
          logIt(theWord,1);
          //-------------- STATUS  --------------------------------
          if (strncmp("#S,",theWord,3) == 0) { 
            if(serialDebug) Serial.println(F("Got radio action STATUS"));
            RadioSend();  
            sendFullStatus(); 
            sendConfig();
            checkHealth();
            return;
          }

          //-------------- DAQ Start from Student Remote (basic)  --------------------------------
          if (strncmp("#DAQSON",theWord,7) == 0) { 
            if(working.DAQrecording == 1) {
              if(serialDebug) Serial.println(F("Got radio action DAQ Start Basic (student) but DAQ already recording"));
            } else {
              working.DAQstudent = 1;
              working.DAQstart = 1;
              if(serialDebug) Serial.println(F("Got radio action DAQ Start Basic (student)"));
              strcpy(radioHeader, "#DON");
              strcpy(radioMessage, "DAQ Student Remote Start");  
              RadioSend();      
              return;
            }
          }

          //-------------- DAQ Stop  --------------------------------
          if (strncmp("#DAQOFF",theWord,7) == 0) { 
            if(working.DAQrecording == 0) {
              if(serialDebug) Serial.println(F("Got radio action DAQ Stop but DAQ already stopped"));
            } else {
              if(serialDebug) Serial.println(F("Got radio action DAQ Stop"));
              strcpy(radioHeader, "#DOFF");
              strcpy(radioMessage, "DAQ Stop");  
              working.DAQstart = 2; // triggers close and reset in other thread
              RadioSend();  
              return;     
            }
          }


          //-------------- NEW FULL CONFIG 1 --------------------------------
          if (strncmp("#FCFG1,",theWord,7) == 0) { 
            if(serialDebug) Serial.println(F("Got radio action FULL Config 1"));
            char tempFill[35][20]={0x0};
            parseit(theWord,tempFill);
            // they are all int but only a few tests needed

            if(configTest1(tempFill)) {
              setConfig1(tempFill);
            } else {
              strcpy(radioHeader, "#M");
              strcpy(radioMessage, "ERROR Bad Configuration Data from Pad");
              RadioSend();
            }      
            return;   
          }


          //-------------- NEW FULL CONFIG 2 --------------------------------
          if (strncmp("#FCFG2,",theWord,7) == 0) { 
            if(serialDebug) Serial.println(F("Got radio action FULL Config 2"));
            char tempFill[35][20]={0x0};
            parseit(theWord,tempFill);
            // they are all int but only a few tests needed

            if(configTest2(tempFill)) {
              setConfig2(tempFill);
              writeConfig();
              
              strcpy(radioHeader, "#M");
              strcpy(radioMessage, "Configuration Update Confirmed");
              RadioSend();  
              sendConfig();
              
            } else {
              strcpy(radioHeader, "#M");
              strcpy(radioMessage, "ERROR Bad Configuration Data from Pad");
              RadioSend();
            }      
            return;   
          }

          //-------------- RESET or RECYCLE  --------------------------------
          if (strncmp("#RSET,",theWord,6) == 0) { 
            if(serialDebug) Serial.println(F("Reset Request from Remote"));
            // Perform a software reboot/reset by writing to the System Control Block's AIRCR register
            SCB_AIRCR = 0x05FA0004; // Write the reset key and the SYSRESETREQ bit
            return;   
          }

          //-------------- DEFAULT ALL CONFIG  --------------------------------
          if (strncmp("#DEFAULT,",theWord,9) == 0) { 
            if(serialDebug) Serial.println(F("Default All Request from Remote"));
            defaultAll();
            writeConfig();
            strcpy(radioHeader, "#M");
            strcpy(radioMessage, "Configuration Defaults Confirmed");
            RadioSend();  
            sendConfig();
            return;    
          }

          //-------------- VALVE:  Open SFU  --------------------------------
          if (strncmp("#OSFU",theWord,5) == 0) { 
            if(serialDebug) Serial.println(F("Got radio action OPEN VALVE: SFU"));
            if(padStatus.armedState == 1) {
              valve("SFU",1);
              sendFullStatus(); 
              strcpy(radioHeader, "#M");
              strcpy(radioMessage, "Confirmed Fuel Valve Open");
              RadioSend();  
            } else {
              notArmed();
            }
            return;   
          }
          //-------------- VALVE:  Open SOX  --------------------------------
          if (strncmp("#OSOX",theWord,5) == 0) { 
            if(serialDebug) Serial.println(F("Got radio action OPEN VALVE: SOX"));
            if(padStatus.armedState == 1) {
              valve("SOX",1);
              sendFullStatus(); 
              strcpy(radioHeader, "#M");
              strcpy(radioMessage, "Confirmed Ox Valve Open");
              RadioSend();  
            } else {
              notArmed(); 
            }
            return;   
          }
          //-------------- VALVE:  Open SFU  --------------------------------
          if (strncmp("#OFAO",theWord,5) == 0) { 
            if(serialDebug) Serial.println(F("Got radio action OPEN SFU and SOX"));
            if(padStatus.armedState == 1) {
              valve("SFU",1);
              valve("SOX",1);
              sendFullStatus(); 
              strcpy(radioHeader, "#M");
              strcpy(radioMessage, "Confirmed SFU and SOX Open");
              RadioSend();  
            } else {
              notArmed();
            }
            return;   
          }
          //-------------- VALVE:  Open SFL  --------------------------------
          if (strncmp("#OSFL",theWord,5) == 0) { 
            if(serialDebug) Serial.println(F("Got radio action OPEN VALVE: SFL"));
            if(padStatus.armedState == 1) {
              valve("SFL",1);
              sendFullStatus(); 
              strcpy(radioHeader, "#M");
              strcpy(radioMessage, "Confirmed Fill Valve Open");
              RadioSend();  
            } else {
              notArmed();
            }
            return;   
          }
          //-------------- VALVE:  Open SPG  --------------------------------
          if (strncmp("#OSPG",theWord,5) == 0) { 
            if(serialDebug) Serial.println(F("Got radio action OPEN VALVE: SPG"));
            if(padStatus.armedState == 1) {
              valve("SPG",1);
              sendFullStatus(); 
              strcpy(radioHeader, "#M");
              strcpy(radioMessage, "Confirmed Purge Valve Open");
              RadioSend();  
            } else {
              notArmed(); 
            }
            return;   
          }
          //-------------- VALVE:  Close SFU  --------------------------------
          if (strncmp("#CSFU",theWord,5) == 0) { 
            if(serialDebug) Serial.println(F("Got radio action CLOSE VALVE: SFU"));
              valve("SFU",0);
              sendFullStatus(); 
              strcpy(radioHeader, "#M");
              strcpy(radioMessage, "Confirmed Fuel Valve Closed");
              RadioSend();  
              return;   
          }
          //-------------- VALVE:  Close SOX  --------------------------------
          if (strncmp("#CSOX",theWord,5) == 0) { 
            if(serialDebug) Serial.println(F("Got radio action CLOSE VALVE: SOX"));
              valve("SOX",0);
              sendFullStatus(); 
              strcpy(radioHeader, "#M");
              strcpy(radioMessage, "Confirmed Ox Valve Closed");
              RadioSend();  
              return;   
          }
          //-------------- VALVE:  Close SFL  --------------------------------
          if (strncmp("#CSFL",theWord,5) == 0) { 
            if(serialDebug) Serial.println(F("Got radio action CLOSE VALVE: SFL"));
              valve("SFL",0);
              sendFullStatus(); 
              strcpy(radioHeader, "#M");
              strcpy(radioMessage, "Confirmed Fill Valve Closed");
              RadioSend();  
              return;   
          }
          //-------------- VALVE:  Close SPG  --------------------------------
          if (strncmp("#CSPG",theWord,5) == 0) { 
            if(serialDebug) Serial.println(F("Got radio action CLOSE VALVE: SPG"));
              valve("SPG",0);
              sendFullStatus(); 
              strcpy(radioHeader, "#M");
              strcpy(radioMessage, "Confirmed Purge Valve Closed");
              RadioSend();  
              return;   
          }
          //-------------- VALVE:  Close SPG  --------------------------------
          if (strncmp("#CLOSE",theWord,6) == 0) { 
            if(serialDebug) Serial.println(F("Got radio action CLOSE ALL VALVES"));
              valve("SPG",0);
              valve("SFL",0);
              valve("SFU",0);
              valve("SOX",0);
              sendFullStatus(); 
              strcpy(radioHeader, "#M");
              strcpy(radioMessage, "Confirmed Close All Valves");
              RadioSend();  
              return;   
          }
          //-------------- ARM CONTROLLER  --------------------------------
          if (strncmp("#ARM1,",theWord,6) == 0) { 
            if(serialDebug) Serial.println(F("Got radio action ARM ALL"));
            if(padStatus.padHot == 1) {
              padStatus.armedState = 1;
              sendFullStatus(); 
              strcpy(radioHeader, "#M");
              strcpy(radioMessage, "Confirmed: Pad is Armed");
              RadioSend();       
            } else {
              strcpy(radioHeader, "#M");
              strcpy(radioMessage, "Denied Arming. Pad is not hot.");   
            }
            return;
          }
          //-------------- DISARM CONTROLLER  --------------------------------
          if (strncmp("#ARM0,",theWord,6) == 0) { 
            if(serialDebug) Serial.println(F("Got radio action DISARM"));
            padStatus.armedState = 0;
            //close all valves when disarmed
            valve("SFL",0);
            valve("SPG",0);
            valve("SFU",0);
            valve("SOX",0);

            if (padStatus.processFill60 == 2) padStatus.processFill60 = 0; // stop  processes if running
            if (padStatus.processPurge == 2) padStatus.processPurge = 0; // stop  processes if running
            if (padStatus.processLaunch == 2) padStatus.processLaunch = 0; // stop  processes if running
            if (padStatus.processAbort == 2) padStatus.processAbort = 0; // stop  processes if running

            sendFullStatus(); 
            strcpy(radioHeader, "#M");
            strcpy(radioMessage, "Confirmed: Pad is Disarmed");
            RadioSend();  
            return;         
          }

          //-------------- FIRE IGNITER --------------------------------
          if (strncmp("#FIRE,",theWord,6) == 0) { 
            if(serialDebug) Serial.println(F("Got radio action FIRE"));
              if(padStatus.armedState == 1) {
                strcpy(radioHeader, "#M");
                strcpy(radioMessage, "Confirmed: Igniter FIRE!");
                RadioSend();
                digitalWrite(pin_igniterFIRE, HIGH);
                timers.igniterTimer = millis() + 3000;
              } else {
                strcpy(radioHeader, "#M");
                strcpy(radioMessage, "DENIED - Controller is not armed");
                RadioSend();
              }
            return;  
          }

          //-------------- AUTO PROCESS FILL 60 SECONDS --------------------------------
          if (strncmp("#PFILL,",theWord,7) == 0) {  
              // Pressurization -- haz command
                if(serialDebug) Serial.println(F("Got radio action PFILL"));
                if(padStatus.armedState == 1) {
                  if(padStatus.processFill60 == 0 || padStatus.processFill60 == 1) { // don't restart if active (2)
                    char tempFill[35][20]={0x0};
                    parseit(theWord,tempFill);
                    working.fillTime = atoi(tempFill[1]);
                    if(working.fillTime >200 || working.fillTime <1) {
                      strcpy(radioHeader, "#M");
                      strcpy(radioMessage, "ERROR Fill Duration out of bounds");
                      RadioSend(); 
                      return;
                    }
                    padStatus.processFill60 = 2;
                    valve("SPG",0);
                    valve("SOX",0);
                    valve("SFU",0);
                    delay(200);
                    valve("SFL",1);
                    timers.processPF60 = 0;
                    sendFullStatus(); 
                    strcpy(radioHeader, "#M");
                    strcpy(radioMessage, "Fill Pressurization Started for ");
                    char intStr[10];
                    itoa(working.fillTime, intStr, 10);
                    strcat(radioMessage, intStr);
                    RadioSend(); 
                  } else {
                    strcpy(radioHeader, "#M");
                    strcpy(radioMessage, "Fill Pressurization Already Running");
                    RadioSend(); 
                  }
                } else {
                  strcpy(radioHeader, "#M");
                  strcpy(radioMessage, "Denied - Pad is not armed");
                  RadioSend();
                }
          }

          //-------------- STOP AUTO PROCESS FILL 60 SECONDS --------------------------------
          if (strncmp("#PFILL0,",theWord,8) == 0) {  
              // Pressurization Stop -- haz command
                if(serialDebug) Serial.println(F("Got radio action PF60 stop"));
                if(padStatus.processFill60 == 2) { 
                    padStatus.processFill60 = 0;
                    valve("SFL",0);
                    sendFullStatus(); 
                    strcpy(radioHeader, "#M");
                    strcpy(radioMessage, "Fill Pressurization STOPPED");
                    RadioSend(); 
                  } else {
                    strcpy(radioHeader, "#M");
                    strcpy(radioMessage, "Fill Pressurization Not Running");
                    RadioSend(); 
                  }
          }

          //-------------- AUTO PROCESS PURGE 10 SECONDS --------------------------------
          if (strncmp("#PPRG,",theWord,6) == 0) {   
              // Purge -- haz command
                if(serialDebug) Serial.println(F("Got radio action Purge PPRG"));
                if(padStatus.armedState == 1) {
                  if(padStatus.processPurge == 0 || padStatus.processPurge == 1) { // don't restart if active (2)
                    char tempFill[35][20]={0x0};
                    parseit(theWord,tempFill);
                    working.purgeTime = atoi(tempFill[1]);
                    if(working.purgeTime >200 || working.purgeTime <1) {
                      strcpy(radioHeader, "#M");
                      strcpy(radioMessage, "ERROR Purge Duration out of bounds");
                      RadioSend(); 
                      return;
                    }
                    padStatus.processPurge = 2;
                    valve("SFL",0);
                    valve("SOX",1);
                    valve("SFU",1);
                    delay(200);
                    valve("SPG",1);
                    timers.processPurge = 0;
                    sendFullStatus(); 
                    strcpy(radioHeader, "#M");
                    strcpy(radioMessage, "Purge Started for ");
                    char intStr[10];
                    itoa(working.purgeTime, intStr, 10);
                    strcat(radioMessage, intStr);
                    RadioSend(); 
                  } else {
                    strcpy(radioHeader, "#M");
                    strcpy(radioMessage, "Purge Already Running");
                    RadioSend(); 
                  }
                } else {
                  strcpy(radioHeader, "#M");
                  strcpy(radioMessage, "Denied - Pad is not armed");
                  RadioSend();
                }
          }
          //-------------- STOP AUTO PROCESS PURGE --------------------------------
          if (strncmp("#PPRG0,",theWord,7) == 0) {  
              // Purge Stop -- haz command
                if(serialDebug) Serial.println(F("Got radio action Purge stop"));
                if(padStatus.processPurge == 2) { 
                    padStatus.processPurge = 0;
                    valve("SPG",0);
                    sendFullStatus(); 
                    strcpy(radioHeader, "#M");
                    strcpy(radioMessage, "Purge STOPPED");
                    RadioSend(); 
                  } else {
                    strcpy(radioHeader, "#M");
                    strcpy(radioMessage, "Purge Not Running");
                    RadioSend(); 
                  }
          }


          //-------------- AUTO PROCESS TO LAUNCH --------------------------------
          if (strncmp("#PLCH,",theWord,6) == 0) {   
              // Launch -- haz command
              // Assumes it is pressurized. 1) close fill, 2) Open fuel / wait dealy  3) igniter fire, 4) open Ox
                if(serialDebug) Serial.println(F("Got radio action LAUNCH"));
                if(padStatus.armedState == 1) {
                  if(padStatus.processLaunch == 0 || padStatus.processLaunch == 1) { // don't restart if active (2)
                    padStatus.processLaunch = 2;
                    valve("SFL",0); // ensures the fill is closed with extra command
                    // **** Code sequence to launch is here *****
                    valve("SFU",1); // open fuel first, longer travel time
                    delay(70); //delay 70ms then ignite and open ox
                    digitalWrite(pin_igniterFIRE, HIGH); // igniter
                    timers.igniterTimer = millis() + 2000; // turn off in 2 seconds
                    valve("SOX",1);
                    //Weeee here we go!
                    padStatus.processLaunch = 1;
                    sendFullStatus(); 
                    strcpy(radioHeader, "#M");
                    strcpy(radioMessage, "Launch Confirmed");
                    RadioSend(); 
                    //resend commands to valves as backup
                    valve("SFU",1);
                    valve("SOX",1);
                    valve("SFL",0); // ensures the fill is closed with extra command
                  } else {
                    strcpy(radioHeader, "#M");
                    strcpy(radioMessage, "Launch Already Running");
                    RadioSend(); 
                  }
                } else {
                  strcpy(radioHeader, "#M");
                  strcpy(radioMessage, "Denied - Pad is not armed");
                  RadioSend();
                }
          }

          //-------------- ABORT --------------------------------
          if (strncmp("#ABORT,",theWord,7) == 0) { 
              // close all valves
              if(serialDebug) Serial.println(F("Got radio action ABORT"));
              valve("SOX",0);
              valve("SFU",0);
              valve("SFL",0);
              valve("SPG",0);
              digitalWrite(pin_igniterFIRE, LOW);
              if(padStatus.processFill60 == 2) padStatus.processFill60 = 0; // stop fill processes if running
              if(padStatus.processPurge == 2) padStatus.processPurge = 0; // stop purge processes if running
              strcpy(radioHeader, "#M");
              strcpy(radioMessage, "Confirmed: ABORT in process");
              RadioSend();               
              padStatus.armedState = 0;              
              sendFullStatus(); 
              return;
          }

          //-------------- RADIO TOGGLE ON OFF --------------------------------
          if (strncmp("#RAD,",theWord,5) == 0) { 
            if(serialDebug) Serial.println(F("Got radio action RADIO OFF TOGGLE"));
              if(working.radioOff) {
                working.radioOff = false;
                strcpy(radioHeader, "#M");
                strcpy(radioMessage, "Radio is back ON");
                RadioSend();
              } else {
                working.radioOff = true; //silent mode
              }
            return;  
          }

          //-------------- ZERO PTs  --------------------------------
          if (strncmp("#ZPTK",theWord,5) == 0) { 
            if(serialDebug) Serial.println(F("Got radio action PTK zero"));
            if(padStatus.PTK == 99999) {
                strcpy(radioHeader, "#M");
                strcpy(radioMessage, "Error zeroing Tank PT");
                RadioSend(); return; 
            } else {
                working.PTKzero = true;
                strcpy(radioHeader, "#M");
                strcpy(radioMessage, "Processing Tank PT zero");
                RadioSend(); return;     
            }             
          }
          if (strncmp("#ZPCH",theWord,5) == 0) { 
            if(serialDebug) Serial.println(F("Got radio action PCH zero"));
            if(padStatus.PCH == 99999) {
                strcpy(radioHeader, "#M");
                strcpy(radioMessage, "Error zeroing Chamber PT");
                RadioSend(); return; 
            } else {
                working.PCHzero = true;
                strcpy(radioHeader, "#M");
                strcpy(radioMessage, "Processing Chamber PT zero");
                RadioSend(); return;     
            }                   
          }
          //-------------- TEST SERVO POSITIONING NUMBER --------------------------------
          // #SERV,XXX,NUM,! - Where XXX is SFU,SOX,SFL,SPG and NUM is 0-3000
          if (strncmp("#TSERV",theWord,6) == 0) {   
              //Test Servo -- haz command
                if(serialDebug) Serial.println(F("Got radio action Test Servo"));
                if(padStatus.armedState == 0) {
                  strcpy(radioHeader, "#M");
                  strcpy(radioMessage, "Denied - Pad is not armed");
                  RadioSend();
                  return;
                }
                char tempFill[35][20]={0x0};
                parseit(theWord,tempFill);
                int pos = atoi(tempFill[2]);
                if(pos < 0 || pos > 3000) {
                  strcpy(radioHeader, "#M");
                  strcpy(radioMessage, "Denied - Position out of range");
                  RadioSend();
                  return;
                }
                ServoTest(tempFill[1],pos);
          }

          strcpy(theWord,"");

      } // End Radio Processing
}

void notArmed() {
    sendFullStatus(); 
    strcpy(radioHeader, "#M");
    strcpy(radioMessage, "Denied - Controller not armed");
    RadioSend(); 
}


//***********************************************************        VALVES     ************************************************************
//***********************************************************        VALVES     ************************************************************
//***********************************************************        VALVES     ************************************************************



void valve(const char* name, int state) {
  
  if(serialDebug) {Serial.print(F("Valve Command Called: "));Serial.print(name);Serial.print(" ");Serial.println(state);}

  if(strncmp("SFU",name,3) == 0) {
    digitalWrite(pin_ext6v, HIGH); //turn on servo power
    delay(5);
    SFU.attach(pin_SFU); 
    timers.timer6v = millis() + local.servo6vhold; 
    if(state == 0) {
      SFU.writeMicroseconds(configuration.SFUclose);
      padStatus.SFU = 0;
    } else {
      SFU.writeMicroseconds(configuration.SFUopen);
      padStatus.SFU = 1;
    }
    return;
  }

  if(strncmp("SOX",name,3) == 0) {
    digitalWrite(pin_ext6v, HIGH); //turn on servo power
    delay(5);
    SOX.attach(pin_SOX); 
    timers.timer6v = millis() + local.servo6vhold; 
    if(state == 0) {
      SOX.writeMicroseconds(configuration.SOXclose);
      padStatus.SOX = 0;
    } else {
      SOX.writeMicroseconds(configuration.SOXopen);
      padStatus.SOX = 1;
    }
    return;
  }

  if(strncmp("SFL",name,3) == 0) {
    digitalWrite(pin_ext6v, HIGH); //turn on servo power
    delay(5);
    SFL.attach(pin_SFL); 
    timers.timer6v = millis() + local.servo6vhold;  
    if(state == 0) {
      SFL.writeMicroseconds(configuration.SFLclose);
      if(serialDebug) {Serial.println("SFL close");Serial.println(configuration.SFLclose);}
      padStatus.SFL = 0;
    } else {
      SFL.writeMicroseconds(configuration.SFLopen);
      if(serialDebug) {Serial.println("SFL open");Serial.println(configuration.SFLopen);}
      padStatus.SFL = 1;
    }
    return;
  }

  if(strncmp("SPG",name,3) == 0) {
    digitalWrite(pin_ext6v, HIGH); //turn on servo power
    delay(5);
    SPG.attach(pin_SPG); 
    timers.timer6v = millis() + local.servo6vhold;  
    if(state == 0) {
      SPG.writeMicroseconds(configuration.SPGclose);
      padStatus.SPG = 0;
    } else {
      SPG.writeMicroseconds(configuration.SPGopen);
      padStatus.SPG = 1;
    }
    return;
  }



}

void ServoTest(const char* name, int position) {

    if (!ina219.begin()) {
      Serial.println("Failed to find INA219 chip");
      strcpy(radioHeader, "#M");
      strcpy(radioMessage, "ERROR INA219 Startup issue");
      RadioSend();
      return;
    }
         
    digitalWrite(pin_ext6v, HIGH); //turn on servo power
    delay(50);
    timers.timer6v = millis() + local.servo6vhold;  
    float startC = 0.0;
    startC = ina219.getCurrent_mA();
    if(serialDebug) {Serial.print(F("Servo Starting Current: "));Serial.println(startC,2);}
    if(startC > 1000) {
      strcpy(radioHeader, "#M");
      strcpy(radioMessage, "Starting Current > 1A");
      RadioSend(); 
    }

  if(strncmp("SFL",name,3) == 0) {
    SFL.attach(pin_SFL);delay(5);
    if(serialDebug) {Serial.print(F("SFL Servo Position Test:  "));Serial.println(position);}
    SFL.writeMicroseconds(position);
    timers.servoTimer = 0; //zero the elapsed timer
    working.servoTest = true;
    delay(30);
    startC = ina219.getCurrent_mA();
    if(serialDebug) {Serial.print(F("Servo Active Current: "));Serial.println(startC,2);}
    strcpy(radioHeader, "#M");
    strcpy(radioMessage, "Testing Servo SFL");
    RadioSend(); 
    return; 
  }

  if(strncmp("SPG",name,3) == 0) {
    SPG.attach(pin_SPG);delay(5);
    if(serialDebug) {Serial.print(F("SPG Servo Position Test:  "));Serial.println(position);}
    SPG.writeMicroseconds(position);
    timers.servoTimer = 0; //zero the elapsed timer
    working.servoTest = true;
    delay(30);
    startC = ina219.getCurrent_mA();
    if(serialDebug) {Serial.print(F("Servo Active Current: "));Serial.println(startC,2);}
    strcpy(radioHeader, "#M");
    strcpy(radioMessage, "Testing Servo SPG");
    RadioSend(); 
    return; 
  }

  if(strncmp("SOX",name,3) == 0) {
    SOX.attach(pin_SOX);delay(5);
    if(serialDebug) {Serial.print(F("SOX Servo Position Test:  "));Serial.println(position);}
    SOX.writeMicroseconds(position);
    timers.servoTimer = 0; //zero the elapsed timer
    working.servoTest = true;
    delay(30);
    startC = ina219.getCurrent_mA();
    if(serialDebug) {Serial.print(F("Servo Active Current: "));Serial.println(startC,2);}
    strcpy(radioHeader, "#M");
    strcpy(radioMessage, "Testing Servo SOX");
    RadioSend(); 
    return; 
  }

  if(strncmp("SFU",name,3) == 0) {
    SFU.attach(pin_SFU);delay(5);
    if(serialDebug) {Serial.print(F("SFU Servo Position Test:  "));Serial.println(position);}
    SFU.writeMicroseconds(position);
    timers.servoTimer = 0; //zero the elapsed timer
    working.servoTest = true;
    delay(30);
    startC = ina219.getCurrent_mA();
    if(serialDebug) {Serial.print(F("Servo Active Current: "));Serial.println(startC,2);}
    strcpy(radioHeader, "#M");
    strcpy(radioMessage, "Testing Servo SFU");
    RadioSend(); 
    return; 
  }



  strcpy(radioHeader, "#M");
  strcpy(radioMessage, "Invalid Servo Code");
  RadioSend(); 
  return; 

}

//***********************************************************        SD LOGGING THREAD    ************************************************************
//***********************************************************        SD LOGGING THREAD     ************************************************************
//***********************************************************        SD LOGGING THREAD     ************************************************************


void logIt(const char* theW, const bool rx){

    //---------------- LOG THE MESSAGE -----------------
      char wordLog[190];
      strncpy(wordLog,theW, 190);
      wordLog[189] = '\0';
       
      float tClock = (float) millis() / (float) 1000.0;
      dtostrf(tClock, 5, 3, working.radioLogString);
      if(rx) {strcat(working.radioLogString,",RX:, ");} else {strcat(working.radioLogString,",TX:, ");}
      strcat(working.radioLogString, wordLog);

    if(working.DAQrecording == 0) {      
      working.radioLog = 1;
      radioLog();
    } else {
      if(working.logCache < 24) { //capture inbound radio to cache if DAQ is recording
        working.logCache++;
        strncpy(working.logCacheStr[working.logCache],working.radioLogString, 200);
        working.logCacheStr[working.logCache][199] = '\0';
      }            
    }
}


void radioLog() {

//----------------------------  RADIO LOGGING FOR COMMAND TIME REPORTING ----------------------
    if(local.radioLog && working.radioLog == 1 && working.DAQrecording == 0 && errors.SD == 0 && working.SD) { // don't log while DAQ is running
       working.radioLog = 0;
       rLogFile = SD.open(working.rLogFilename, FILE_WRITE);
       if(rLogFile) {
          //---------------  catch up on cache after DAQ if needed
          int tmp = 0;
          while(working.logCache > 0){
            tmp++;
            size_t written = rLogFile.println(working.logCacheStr[tmp]);
            if(written == 0) errors.SD = 1;
            if(serialDebug) Serial.println("Catchup Log: ");
            if(serialDebug) Serial.println(working.logCacheStr[tmp]);  
            if(tmp >= working.logCache) working.logCache = 0;
          } // ------------- end cache
          size_t written = rLogFile.println(working.radioLogString);
          if(written == 0) errors.SD = 1;
          rLogFile.close();
       } else {
          if(serialDebug) Serial.println(F("Flash Open Log File Error"));
          errors.SD = 1;        
       } 
    }
}






//***********************************************************        CONFIG FILE    ************************************************************
//***********************************************************        CONFIG FILE     ************************************************************

void sendConfig() {
    configSentence(); // set the Config sentence
    if(serialDebug) Serial.println(F("Sending Config"));
    if(serialDebug) Serial.println(F(working.configString));
    strcpy(radioHeader, "#CFG");
    strcpy(radioMessage, working.configString);
    RadioSend();  
}

void readSDconfig() {
    if(errors.SD == 1) {
      if(serialDebug) Serial.println("SD errors - can't get Configuration...");
      return;
    }
    
    if(serialDebug) Serial.println("Reading Configuration...");
    char result[25];
    char fname[32] = "/config.txt";
    if(!SD.exists(fname)) {
      if(serialDebug) Serial.println("Config file does not exist");
      writeConfig();
    } else {
      if(serialDebug) Serial.println("Config file found");
      configFile = SD.open(fname, FILE_READ);
      //removed PT zero values from config. Force set each time restart.
      strcpy(result,"");readLine(result);configuration.PTKenabled = atoi(result);
      strcpy(result,"");readLine(result);configuration.PTKrange = atoi(result);
      strcpy(result,"");readLine(result);configuration.PTKalarm = atoi(result);
      strcpy(result,"");readLine(result);configuration.PLDenabled = atoi(result);
      strcpy(result,"");readLine(result);configuration.PCHenabled = atoi(result);
      strcpy(result,"");readLine(result);configuration.PCHrange = atoi(result);
      strcpy(result,"");readLine(result);configuration.CPUtempAlarm = atoi(result);
      strcpy(result,"");readLine(result);configuration.SFUopen = atoi(result);
      strcpy(result,"");readLine(result);configuration.SFUclose = atoi(result);
      strcpy(result,"");readLine(result);configuration.SOXopen = atoi(result);
      strcpy(result,"");readLine(result);configuration.SOXclose = atoi(result);
      strcpy(result,"");readLine(result);configuration.SFLopen = atoi(result);
      strcpy(result,"");readLine(result);configuration.SFLclose = atoi(result);
      strcpy(result,"");readLine(result);configuration.SPGopen = atoi(result);
      strcpy(result,"");readLine(result);configuration.SPGclose = atoi(result);
      configFile.close(); 

    }
}

void setConfig(char tF[35][20]) {  //unused by radio logic

  configuration.PTKenabled = atoi(tF[1]);
  configuration.PTKrange = atoi(tF[2]);
  configuration.PTKalarm = atoi(tF[3]);
  configuration.PLDenabled = atoi(tF[4]);
  configuration.PCHenabled = atoi(tF[5]);
  configuration.PCHrange = atoi(tF[6]);
  configuration.CPUtempAlarm = atoi(tF[7]);
  configuration.SFUopen = atoi(tF[8]);
  configuration.SFUclose = atoi(tF[9]);
  configuration.SOXopen = atoi(tF[10]);
  configuration.SOXclose = atoi(tF[11]);
  configuration.SFLopen = atoi(tF[12]);
  configuration.SFLclose = atoi(tF[13]);
  configuration.SPGopen = atoi(tF[14]);
  configuration.SPGclose = atoi(tF[15]);

}

void setConfig1(char tF[35][20]) {

  configuration.PTKenabled = atoi(tF[1]);
  configuration.PTKrange = atoi(tF[2]);
  configuration.PTKalarm = atoi(tF[3]);
  configuration.PLDenabled = atoi(tF[4]);
  configuration.PCHenabled = atoi(tF[5]);
  configuration.PCHrange = atoi(tF[6]);
  configuration.CPUtempAlarm = atoi(tF[7]);
}

void setConfig2(char tF[35][20]) {
  configuration.SFUopen = atoi(tF[1]);
  configuration.SFUclose = atoi(tF[2]);
  configuration.SOXopen = atoi(tF[3]);
  configuration.SOXclose = atoi(tF[4]);
  configuration.SFLopen = atoi(tF[5]);
  configuration.SFLclose = atoi(tF[6]);
  configuration.SPGopen = atoi(tF[7]);
  configuration.SPGclose = atoi(tF[8]);

}




void configSentence() {
  // NOTE: do not send PT zero values. they stay local.

  char comma[5] = ",";
  char str_int[16];

  strcpy(working.configString, "");  //zero out the string
  //------------ START ------------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , configuration.PTKenabled);
  strcat(working.configString, str_int);strcat(working.configString, comma);
  //-------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , configuration.PTKrange);
  strcat(working.configString, str_int);strcat(working.configString, comma);
  //-------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , configuration.PTKalarm);
  strcat(working.configString, str_int);strcat(working.configString, comma);
  //-------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , configuration.PLDenabled);
  strcat(working.configString, str_int);strcat(working.configString, comma);
  //-------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , configuration.PCHenabled);
  strcat(working.configString, str_int);strcat(working.configString, comma);
  //-------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , configuration.PCHrange);
  strcat(working.configString, str_int);strcat(working.configString, comma);
  //-------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , configuration.CPUtempAlarm); //26
  strcat(working.configString, str_int);strcat(working.configString, comma);
  //-------------
  strcpy(str_int, ""); sprintf (str_int, "%d" , configuration.SFUopen); //26
  strcat(working.configString, str_int);strcat(working.configString, comma);
  //------------- 
  strcpy(str_int, ""); sprintf (str_int, "%d" , configuration.SFUclose); //26
  strcat(working.configString, str_int);strcat(working.configString, comma);
  //-------------   
  strcpy(str_int, ""); sprintf (str_int, "%d" , configuration.SOXopen); //26
  strcat(working.configString, str_int);strcat(working.configString, comma);
  //------------- 
  strcpy(str_int, ""); sprintf (str_int, "%d" , configuration.SOXclose); //26
  strcat(working.configString, str_int);strcat(working.configString, comma);
  //-------------     
  strcpy(str_int, ""); sprintf (str_int, "%d" , configuration.SFLopen); //26
  strcat(working.configString, str_int);strcat(working.configString, comma);
  //------------- 
  strcpy(str_int, ""); sprintf (str_int, "%d" , configuration.SFLclose); //26
  strcat(working.configString, str_int);strcat(working.configString, comma);
  //-------------   
  strcpy(str_int, ""); sprintf (str_int, "%d" , configuration.SPGopen); //26
  strcat(working.configString, str_int);strcat(working.configString, comma);
  //------------- 
  strcpy(str_int, ""); sprintf (str_int, "%d" , configuration.SPGclose); //26
  strcat(working.configString, str_int);
  //-------------   
  

}

void defaultAll() {

      configuration.PTKenabled = 1;
      configuration.PTKrange = 1600;
      configuration.PTKalarm = 1200;
      
      configuration.PLDenabled = 1;
      configuration.PCHenabled = 1;
      configuration.PCHrange = 500;
      configuration.PCHzero = 0.5;
      configuration.CPUtempAlarm = 165;

      configuration.SFUopen = 500;
      configuration.SFUclose = 1200;
      configuration.SOXopen = 500;
      configuration.SOXclose = 1200;
      configuration.SFLopen = 500;
      configuration.SFLclose = 1200;
      configuration.SPGopen = 500;
      configuration.SPGclose = 1200;

}

void writeConfig() {
  // write the config using the current live or default start-up settings
  if(serialDebug) Serial.println(F("Writing new config file"));
  char fname[32] = "/config.txt";
  SD.remove(fname);
  File configFile;
  configFile = SD.open(fname, FILE_WRITE);
  delay(10);
  
  configFile.println(configuration.PTKenabled);
  configFile.println(configuration.PTKrange);
  configFile.println(configuration.PTKalarm);
  configFile.println(configuration.PLDenabled);
  configFile.println(configuration.PCHenabled);
  configFile.println(configuration.PCHrange);
  configFile.println(configuration.CPUtempAlarm);
  configFile.println(configuration.SFUopen);
  configFile.println(configuration.SFUclose);
  configFile.println(configuration.SOXopen);
  configFile.println(configuration.SOXclose);
  configFile.println(configuration.SFLopen);
  configFile.println(configuration.SFLclose);
  configFile.println(configuration.SPGopen);
  configFile.println(configuration.SPGclose);
  configFile.close();
  if(serialDebug) Serial.println("Wrote new config.txt file");
  
}


void readLine(char* res) {
   char cr;
   int i = 0;
   while(true){
    cr = configFile.read();
    res[i] = cr;
    i++;
    if(cr == '\n' || i == 23){
      return;
    }
   }
}

void printConfig() {
  if(serialDebug) {
    Serial.println("CONFIGURATION: ");
    Serial.print("       PTKenabled: ");Serial.println(configuration.PTKenabled);
    Serial.print("         PTKrange: ");Serial.println(configuration.PTKrange);
    Serial.print("         PTKalarm: ");Serial.println(configuration.PTKalarm);
    Serial.print("       PLDenabled: ");Serial.println(configuration.PLDenabled);
    Serial.print("       PCHenabled: ");Serial.println(configuration.PCHenabled);
    Serial.print("         PCHrange: ");Serial.println(configuration.PCHrange);
    Serial.print("         SFU Open: ");Serial.println(configuration.SFUopen);
    Serial.print("        SFU Close: ");Serial.println(configuration.SFUclose);
    Serial.print("         SOX Open: ");Serial.println(configuration.SOXopen);
    Serial.print("        SOX Close: ");Serial.println(configuration.SOXclose);
    Serial.print("         SFL Open: ");Serial.println(configuration.SFLopen);
    Serial.print("        SFL Close: ");Serial.println(configuration.SFLclose);
    Serial.print("         SPG Open: ");Serial.println(configuration.SPGopen);
    Serial.print("        SPG Close: ");Serial.println(configuration.SPGclose);
    Serial.print("Pres TX wait time: ");Serial.println(configuration.pressureWaitTime);
    Serial.print(" CPU Temp Alarm-F: ");Serial.println(configuration.CPUtempAlarm);
    Serial.println("  ");
  }
}



//Config data integrity testing
  
bool configTest1(char tF[35][20]) {
  // test for valid config data
  if(zeroOne(atoi(tF[1]))) return false;
  if(PTrange(atoi(tF[2]))) return false;
  if(PTrange(atoi(tF[3]))) return false;
  if(zeroOne(atoi(tF[4]))) return false;
  if(zeroOne(atoi(tF[5]))) return false;
  if(PTrange(atoi(tF[6]))) return false;
  if(oneThreeh(atoi(tF[7]))) return false;
  return true;
}

bool configTest2(char tF[35][20]) {
  // test for valid config data
  if(PTrange(atoi(tF[1]))) return false;
  if(PTrange(atoi(tF[2]))) return false;
  if(PTrange(atoi(tF[3]))) return false;
  if(PTrange(atoi(tF[4]))) return false;
  if(PTrange(atoi(tF[5]))) return false;
  if(PTrange(atoi(tF[6]))) return false;
  if(PTrange(atoi(tF[7]))) return false;
  if(PTrange(atoi(tF[8]))) return false;
  return true;
}

bool zeroOne(int theT) {
  if(theT == 0 || theT == 1) {
    return false;
  } else {
    return true;
  }
}

bool oneTen(int theT) {
  if(theT > 0 && theT < 11) {
    return false;
  } else {
    return true;
  }
}

bool oneThreeh(int theT) {
  if(theT > 0 && theT < 301) {
    return false;
  } else {
    return true;
  }
}

bool PTrange(int theT) {
  if(theT > 0 && theT < 6001) {
    return false;
  } else {
    return true;
  }
}


bool chkTime(int theT) {
  if(theT > 100 && theT < 60000) {
    return false;
  } else {
    return true;
  }
}





//***********************************************************        DAQ DATA THREAD    ************************************************************
//***********************************************************        DAQ DATA THREAD     ************************************************************
//***********************************************************        DAQ DATA THREAD     ************************************************************



void getRlog() {
  // read the current log number and increment it from flash
  int thevalue;
  char buf[2];
  if(serialDebug) Serial.println(F("Getting Log Number"));
  
  File theFile; 
  strcpy(buf,"");
  theFile = SD.open("logcount.txt", FILE_READ);
  delay(2);
  int fx = 0;
  if(serialDebug) Serial.print("current logcount.txt file: ");
  if(theFile.available()) {
    theFile.read(buf,1);
    delay(1);
    buf[1] = '\0';
    if(serialDebug) Serial.println(atoi(buf));
    fx = 1;
  } else {
    if(serialDebug) Serial.println("No file exists");
  }
  delay(2);
  if(fx > 0) {
    thevalue = atoi(buf);
    if(serialDebug) Serial.print(F("confirming old value: "));
    if(serialDebug) Serial.println(thevalue);
  } else {
    if(serialDebug) Serial.println(F("Error opening logcount.txt"));
    thevalue = 0;
  }
  if (thevalue > 8) {
    thevalue = 0; // rotate through 10 log files to save space
    if(serialDebug) Serial.println(F("resetting value to 0"));
  }
  thevalue = thevalue + 1;
  char tempS[12];
  strcpy(tempS,"");
  //sprintf (tempS, "%d" , thevalue);
  tempS[0] = '0' + thevalue;
  tempS[1] = '\0'; 
  strcpy(working.rLogFilename,"/log");
  strcat(working.rLogFilename, tempS);
  strcat(working.rLogFilename, ".txt"); 
  if(serialDebug) Serial.print(F("New Radio File: "));
  if(serialDebug) Serial.println(working.rLogFilename);

  SD.remove("logcount.txt");
  delay(2);
  theFile = SD.open("logcount.txt", FILE_WRITE);
  theFile.println(thevalue);
  delay(5);    
  theFile.close();delay(5);
  
  SD.remove(working.rLogFilename);
  theFile = SD.open(working.rLogFilename, FILE_WRITE);
  theFile.println("Starting new GSE log file");delay(1);    
  theFile.close();

  if(serialDebug) Serial.print(F("New Log Number: "));
  if(serialDebug) Serial.println(thevalue);
} 




//***********************************************************        SAMPLE FUNCTIONS    ************************************************************
//***********************************************************        SAMPLE FUNCTIONS     ************************************************************
//***********************************************************        SAMPLE FUNCTIONS     ************************************************************

/* Dedicated thread for sampling the ADC
 *  
 
 5v full range is 5000 on ADC. Usable range is .5v to 4.5v so 4000 range.

 ADC A:
   0 = Main Voltage through opAmp (/ 10)
   1 = PTK - Rocket PT Tank
   2 = PLD - PT Load Cell
   3 = PCH - PT Chamber

 */

void sampleLoop() {


    // ----------------------------------  Check Main Voltage  ------------------------
    if(millis() > mainVoltageTimer && rotateA == 1) {
      if(local.mvTimer > local.mvInt) { // skip voltage interval if not ready 
        ads1115a.setMultiplexer(ADS1115_MUX_AIN0_GND);
        ads1115a.startSingleConvertion();
        rotateAwait = 0;
        rotateA = 2;
        local.mvTimer = 0;
      } else { 
        rotateA = 3;}
    }
    if(rotateA == 2 && rotateAwait >= local.ADCwait) {
      valueADC = ads1115a.readConvertedValue();
      float theVolts = valueADC / 100; // actual voltage
      float thePercent = 0.0;
      //8.3 is 100% and 6.8 is zero, so diff of 1.5
      thePercent = (theVolts - 6.8)/ (float) 1.5;
      thePercent = thePercent * 100;
      if (thePercent > 100) thePercent = 100;
      if(thePercent < 0) thePercent = 0;
      padStatus.mainVolts = (int) thePercent; // opAmp voltage divider x10
      mainVoltageTimer = millis() + local.ADCint;
      rotateA = 3;
    }


    // ----------------------------------  Check PT Rocket Tank  ------------------------

    if(millis() > PTKtimer && rotateA == 3) {
      ads1115a.setMultiplexer(ADS1115_MUX_AIN1_GND);
      ads1115a.startSingleConvertion();
      rotateAwait = 0;
      rotateA = 4;
    }
    if(rotateA == 4 && rotateAwait >= local.ADCwait) {
      valueADC = ads1115a.readConvertedValue(); // v x 1000    
      if(valueADC < (float) 300) {
        padStatus.PTK = 99999; // error state below .3v
      } else {
        valueADC = valueADC / (float) 1000.0;
        if(working.PTKzero) { configuration.PTKzero = valueADC; working.PTKzero = false;} // zero command
        valueADC = valueADC - configuration.PTKzero; // remove .5v base
        valueADC = valueADC / (float) 4.0 * (float) configuration.PTKrange;
        padStatus.PTK = (int) valueADC; // the PSI
      }
      PTKtimer = millis() + local.ADCint;
      if(configuration.PLDenabled == 1) {rotateA = 5;} else {rotateA = 7;}
    }

    // ----------------------------------  Check PT Load Cell  ------------------------

    // This code shanged to monitor load values 7/2024 - the old code will not work
    
    if(millis() > PLDtimer && rotateA == 5) {
      ads1115a.setMultiplexer(ADS1115_MUX_AIN2_GND);
      ads1115a.startSingleConvertion();
      rotateAwait = 0;
      rotateA = 6;
    }
    if(rotateA == 6 && rotateAwait >= local.ADCwait) {
      valueADC = ads1115a.readConvertedValue(); // v x 1000
      if(valueADC < 5001 && valueADC >= 0) {
        padStatus.PLD = (int) valueADC;
      } else {
        padStatus.PLD = (int) 999;
      } 
      PLDtimer = millis() + local.ADCint;
      if(configuration.PCHenabled == 1) {rotateA = 7;} else {rotateA = 1;}
    }   

    // ----------------------------------  Check PT Chamber  ------------------------

    if(millis() > PCHtimer && rotateA == 7) {
      ads1115a.setMultiplexer(ADS1115_MUX_AIN3_GND);
      ads1115a.startSingleConvertion();
      rotateAwait = 0;
      rotateA = 8;
    }
    if(rotateA == 8 && rotateAwait >= local.ADCwait) {
      valueADC = ads1115a.readConvertedValue(); // v x 1000    
      if(valueADC < (float) 300) {
        padStatus.PCH = 99999; // error state below .3v
      } else {
        valueADC = valueADC / (float) 1000.0;
        if(working.PCHzero) { configuration.PCHzero = valueADC; working.PCHzero = false;} // zero command
        valueADC = valueADC - configuration.PCHzero; // remove .5v base
        valueADC = valueADC / (float) 4.0 * (float) configuration.PCHrange;
        padStatus.PCH = (int) valueADC; // the PSI
      }
      PCHtimer = millis() + local.ADCint;
      rotateA = 1;
    }

} // end sample thread


//***********************************************************        DAQ DATA THREAD    ************************************************************
//***********************************************************        DAQ DATA THREAD     ************************************************************
//***********************************************************        DAQ DATA THREAD     ************************************************************


void DAQlogging() {

    //----------------------------  START DAQ ----------------------------------
    if(working.DAQstart == 1 && errors.SD == 0 && working.SD) {  // added SD cuz now only on SD

      if(working.DAQrecording == 0) {
        // start new logging session and file
        if(serialDebug) Serial.println(F("DAQ start-up in process"));
        working.DAQrecording = 1;
        configuration.pressureWaitTime = 5000; // only send pressure data to remote every 5 sec during DAQ
        set_arm_clock(384'000'000); //increase MCU clock for DAQ operations - temporary to manage MCU temperature
        delay(20);
        working.DAQstart = 0;
        if(working.DAQstudent == 1) getDAQfilename();
        strcpy(theFilename,"/");
        strcat(theFilename,working.DAQfilename);
        SD.remove(theFilename);
        delay(2);
        logFile = SD.open(theFilename, FILE_WRITE);
        delay(2);
        if(logFile) {
          if(serialDebug) Serial.println(F("DAQ file open was successful"));
          strcpy(fileLine, "CPUmillis,Milliseconds,Tank,Chamber,Load,");
          strcat(fileLine, working.rLogFilename); // include reference to the active log file
          logFile.println(fileLine); // write the header file
          delay(2);
          logFile.flush();
          logIt(working.DAQfilename,0); // put the name of the DAQ file in the radio log file
          sTimeout = millis() + local.DAQsuperTimeout;
          hardSave = millis() + local.DAQflush;
          DAQnextTime = 0;
          fileClock = 0;
        } else {
            if(serialDebug) Serial.println(F("SD Failed"));
            errors.SD = 1;
            working.DAQrecording = 0;
            working.DAQstart = 0; 
            set_arm_clock(150'000'000);
            return;
        }
      } else {
        working.DAQstart = 0; // error call for start again
        if(serialDebug) Serial.println(F("DAQ start while started error"));
        return;
      }
      fileClock = 0;
    }
    //----------------------------  DO DAQ ---------------------------------------------------------------
    //----------------------------  DO DAQ ---------------------------------------------------------------
    if(working.DAQrecording == 1 && errors.SD == 0) {  // we are ballin'
      if(millis() < sTimeout) {
        if(millis() >= DAQnextTime) {
          DAQnextTime = millis() + local.logRate;
          // build the string
          char str[20]; 
          char theLine[200];
          strcpy(theLine,"");
          unsigned long fc = 0;
          float tClock = (float) millis() / (float) 1000.0;
          dtostrf(tClock, 5, 3, theLine);strcat(theLine, ",");
          fc = fileClock;
          sprintf(str, "%lu", fc);
          strcat(theLine,str);
          strcat(theLine, ",");
          sprintf(str, "%d", padStatus.PTK);
          strcat(theLine,str);
          strcat(theLine, ",");
          sprintf(str, "%d", padStatus.PCH);
          strcat(theLine,str);
          strcat(theLine, ",");
          sprintf(str, "%d", padStatus.PLD);
          strcat(theLine,str);
          size_t written = logFile.println(theLine);
          if(written == 0) errors.SD = 1;

        }
        if(millis() > hardSave) {
          hardSave = millis() + local.DAQflush;
          logFile.close();
          logFile = SD.open(theFilename, FILE_WRITE);
          if(!logFile) errors.SD = 1;
        }
      } else {
        working.DAQstart = 2; // stop it due to timeout
        if(serialDebug) Serial.println(F("DAQ stop due to timeout"));
      }
      //----------------------------  END DAQ ---------------------------------------------------------------
    }
    if(working.DAQstart == 2) {
      //Done DAQ
      if(serialDebug) Serial.println(F("DAQ stopped"));
      working.DAQrecording = 0;
      working.DAQstart = 0;     
      working.DAQstudent = 1;   
      logFile.close();
      delay(5);
      set_arm_clock(150'000'000); //reset MCU speed for thermal management
      delay(2);
      configuration.pressureWaitTime = 2000; //reset pressure send data to 2 seconds 
      strcpy(radioHeader, "#DOFF");
      strcpy(radioMessage, "DAQ Stop");  
      RadioSend();  
      if(serialDebug) Serial.println(F("Done Doing DAQ"));
    }
}

void getDAQfilename() {
  // read the current generic DAQ filename number and increment it from flash
  int thevalue;
  char buf[2];
  if(serialDebug) Serial.println(F("Getting DAQ file Number"));
  
  File theFile; 
  strcpy(buf,"");
  theFile = SD.open("DAQcount.txt", FILE_READ);
  delay(2);
  int fx = 0;
  if(serialDebug) Serial.print("DAQcount.txt file: ");
  if(theFile.available()) {
    theFile.read(buf,1);
    delay(1);
    buf[1] = '\0';
    if(serialDebug) Serial.print(atoi(buf));
    fx++;
  }
  theFile.close();
  delay(2);
  if(fx > 0) {
    if(serialDebug) Serial.print(F(" old value: "));
    thevalue = atoi(buf);
  } else {
    if(serialDebug) Serial.println(F("error opening DAQcount.txt"));
    thevalue = 0;
  }
  if (thevalue > 8) thevalue = 0; // rotate through 10 DAQ files to save space
  thevalue++;
  char tempS[4];
  strcpy(tempS,"");
  tempS[0] = '0' + thevalue;
  tempS[1] = '\0'; 
  strcpy(working.DAQfilename,"/DAQ");
  strcat(working.DAQfilename, tempS);
  strcat(working.DAQfilename, ".txt"); 

  SD.remove("DAQcount.txt");
  delay(2);
  theFile = SD.open("DAQcount.txt", FILE_WRITE);delay(2);
  theFile.println(thevalue);delay(2);
  theFile.close();delay(2);
  
  if(serialDebug) Serial.print(F("New DAQ Number: "));
  if(serialDebug) Serial.println(thevalue);
} 




//******************************************************************** CHANGE LOG *********************************************************
//******************************************************************** CHANGE LOG *********************************************************
//******************************************************************** CHANGE LOG *********************************************************

/*



*/





