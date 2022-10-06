#define REMOTEXY_MODE__HARDSERIAL
#include <RemoteXY.h> // https://www.arduino.cc/reference/en/libraries/remotexy/

/*** RemoteXY Setup ***/
#define REMOTEXY_SERIAL Serial
#define REMOTEXY_SERIAL_SPEED 9600

/*** Libraries ***/
#include <AltSoftSerial.h> // https://www.arduino.cc/reference/en/libraries/altsoftserial/
#include <ServoTimer2.h> // https://github.com/nabontra/ServoTimer2
#include <neotimer.h> // https://github.com/jrullan/neotimer
#include <ADCTouch.h> // https://www.arduino.cc/reference/en/libraries/adctouch/
#include <Ramp.h> // https://www.arduino.cc/reference/en/libraries/ramp/
#include <EEPROM.h> // https://docs.arduino.cc/learn/built-in-libraries/eeprom
#include <DFPlayerMini_Fast.h> // https://www.arduino.cc/reference/en/libraries/dfplayermini_fast/

/*** DfPlayer Setup ***/
int serial1 = 8; // RX
int serial2 = 9; // TX
AltSoftSerial mp3Serial; // RX, TX
DFPlayerMini_Fast myMP3;
int TrackToPlay;
int potVal;
int oldPotVal;
int previousMillis;
boolean isPlaying = false;

/*** Servo Setup ***/
#define servoPin 2
ServoTimer2 servo; 
rampUnsignedInt moveWing;

/*** Touch Button Setup ***/
int ref1, ref2;
#define TOUCHPIN1 A0
#define TOUCHPIN2 A1
#define RESOLUTION 10
#define SMOOTH 100
// determen when the sensor is understood as "ON"
float multiplier = 1.15;

// smooth data a little:
// the last readings
int previousReadings1[SMOOTH];
int previousReadings2[SMOOTH];

// used for cycling through the array
int currentIndex1 = 0;
int currentIndex2 = 0;

// the latest reading
int reading1;
int reading2;

// calculate the average of the previous readings
int average1(){
  // calculate the sum of all previous readings
  unsigned long sum = 0;
  for(int i = 0; i < SMOOTH; i++){
    sum += previousReadings1[i];
  }
  // return the sum divided by the number of elements
  // or, in other words, the average of all previous readings
  //Serial.println(sum/SMOOTH);
  return sum / SMOOTH;
}
// calculate the average of the previous readings
int average2(){
  // calculate the sum of all previous readings
  unsigned long sum = 0;
  for(int i = 0; i < SMOOTH; i++){
    sum += previousReadings2[i];
  }
  // return the sum divided by the number of elements
  // or, in other words, the average of all previous readings
  //Serial.println(sum/SMOOTH);
  return sum / SMOOTH;
}
bool button1 = 0;
bool button2 = 0;

int button1Status;
int button2Status;

enum buttonActionEnum{
  ON = LOW,
  OFF = HIGH
} state = OFF;

/*** LED Setup ***/
// Define which pins will be used for 
// the Shift Register controls
// Wing LEDs
#define dataPinWing 12 
#define latchPinWing 11
#define clockPinWing 10

// Main Screen LEDs
#define dataPinMain 7
#define latchPinMain 6
#define clockPinMain 5

/*** LED Order for Reference ***/
// GB1 light order - 3, 5, 7, 4, 1, 6, 2
// GB2 light order - 3, 1, 6, 4, 7, 5, 2
// Byte sequential order 1,2,4,8,16,32,64
//       Numerical order 1,2,3,4, 5, 6, 7

int wing[7] = {4,16,32,2,64,8,1};
int wing2[7] = {4,1,32,8,64,16,2};
int mainScreen[7] = {1,2,4,8,16,32,64};

/*** Indicator LEDS ***/
int redIndicator = 18;
int screenIndicator = 19;
long previousMillisLED[2];
int redBtnLED1 = 16;
int redBtnLED2 = 17;

/*** Wing Positions ***/
int fullopen = 1801;
int halfopen = 1255;
int closed = 750;

/*** Timers ***/
Neotimer timeOut = Neotimer(2000); // 2 second timer
Neotimer sleepPKE = Neotimer(75000); // 75 seconds timer

/*** Menu Settings ***/
byte buttonPresses = 0; // how many times the button has been pressed
byte lastPressCount = 0; // to keep track of last press count
int menuSettings = 0;
int menuToggle = 1;
int wingPattern = 2;
int pkeVolume = 3;
int playOnce = 0;
int vol;
unsigned long pressedTime  = 0;
unsigned long releasedTime = 0;
unsigned long pressedTime1  = 0;
unsigned long releasedTime1 = 0;
unsigned long pressedTime2  = 0;
unsigned long releasedTime2 = 0;
const int LONG_PRESS_TIME = 10000; // 10 seconds
const int SHORT_PRESS_TIME = 2000; // 2 seconds
int button1State;
int button2State;
byte lastState1 = 0;
byte lastState2 = 0;
int buttonHoldMillis;
long pressDuration;
long pressDuration1;
long pressDuration2;
long exitPressDuration;
int N = 1;
int runXTimes = 0;
int run1Times = 0;
int run2Times = 0;
int run3Times = 0;
int run4Times = 0;
int run5Times = 0;
int run6Times = 0;
int run7Times = 0;
int run8Times = 0;
int run9Times = 0;
int run10Times = 0;

/*** Remote Settings ***/
int passFlag = 0;
int timerSet = 4;
int timerVal = 5;
int timerGo = 6;
int wingsHalf = 7;
int wingsFull = 8;
int lightsSpd = 9;

// RemoteXY configurate  
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =   // 671 bytes
  { 255,9,0,0,0,152,2,16,16,5,130,3,32,89,31,13,0,16,130,3,
  0,89,32,14,0,31,4,64,43,36,6,29,2,1,26,2,1,39,17,14,
  6,2,1,26,31,31,79,78,0,79,70,70,0,129,0,38,30,15,3,2,
  24,76,69,68,32,83,112,101,101,100,0,129,0,8,30,22,3,2,8,87,
  105,110,103,115,32,67,111,110,116,114,111,108,115,0,129,0,42,13,7,3,
  2,8,77,117,116,101,0,3,130,25,76,14,8,2,1,26,129,0,22,71,
  18,3,2,8,76,105,103,104,116,32,80,97,116,116,101,114,110,0,129,0,
  3,4,57,6,0,8,80,75,69,32,77,101,116,101,114,32,67,111,110,116,
  114,111,108,115,0,2,1,11,17,15,6,2,1,26,31,31,79,78,0,79,
  70,70,0,129,0,20,56,10,4,2,8,49,48,48,37,0,129,0,21,40,
  8,4,2,8,53,48,37,0,129,0,10,13,16,3,2,8,77,97,105,110,
  32,80,111,119,101,114,0,131,2,34,90,28,9,1,16,24,84,105,109,101,
  114,0,131,3,1,90,29,9,2,31,24,77,97,105,110,0,7,53,17,33,
  31,8,1,24,29,1,129,0,22,27,21,5,1,8,83,101,116,32,84,105,
  109,101,0,2,1,23,71,19,7,1,1,26,31,31,79,78,0,79,70,70,
  0,129,0,14,56,35,5,1,8,65,99,116,105,118,97,116,101,32,84,105,
  109,101,114,0,129,0,3,14,56,2,1,8,80,75,69,32,77,97,105,110,
  32,80,111,119,101,114,32,109,117,115,116,32,98,101,32,105,110,32,116,104,
  101,32,79,70,70,32,112,111,115,105,116,105,111,110,32,116,111,32,117,115,
  101,32,116,104,101,32,116,105,109,101,114,46,0,129,0,3,17,57,2,1,
  8,85,115,101,32,116,104,101,32,109,97,105,110,32,99,111,110,116,114,111,
  108,115,32,116,111,32,115,101,116,32,116,104,101,32,119,105,110,103,115,32,
  97,110,100,32,108,105,103,104,116,115,32,102,111,114,32,116,104,101,32,116,
  105,109,101,114,46,0,129,0,16,42,33,3,1,8,77,105,110,32,49,109,
  105,110,32,45,32,77,97,120,32,54,48,109,105,110,115,0,129,0,8,62,
  48,2,1,8,79,110,99,101,32,116,104,101,32,116,105,109,101,114,32,105,
  115,32,97,99,116,105,118,97,116,101,100,44,32,116,104,101,32,80,75,69,
  32,119,105,108,108,32,114,101,116,117,114,110,32,116,111,32,116,104,101,0,
  129,0,14,65,37,2,1,8,99,108,111,115,101,100,32,112,111,115,105,116,
  105,111,110,32,117,110,116,105,108,32,116,104,101,32,116,105,109,101,32,105,
  115,32,114,101,97,99,104,101,100,46,0,129,0,14,79,8,4,2,8,71,
  66,49,0,129,0,42,79,8,4,2,8,71,66,50,0,10,48,9,38,9,
  9,2,1,26,31,79,78,0,31,79,70,70,0,10,48,9,53,9,9,2,
  1,26,31,79,78,0,31,79,70,70,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  int8_t led_speed; // =0..100 slider position 
  uint8_t mute; // =1 if switch ON and =0 if OFF 
  uint8_t led_pattern; // =0 if select position A, =1 if position B, =2 if position C, ... 
  uint8_t power; // =1 if switch ON and =0 if OFF 
  int16_t timer_num;  // 32767.. +32767 
  uint8_t timer_on; // =1 if switch ON and =0 if OFF 
  uint8_t half_open; // =1 if state is ON, else =0 
  uint8_t full_open; // =1 if state is ON, else =0 

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0 

} RemoteXY;
#pragma pack(pop)

void setup() {
  RemoteXY_Init ();
  
  // Configure Wing Pins
  pinMode(dataPinWing, OUTPUT);
  pinMode(latchPinWing, OUTPUT);
  pinMode(clockPinWing, OUTPUT);
  // Configure Main Screen Pins
  pinMode(dataPinMain, OUTPUT);
  pinMode(latchPinMain, OUTPUT);
  pinMode(clockPinMain, OUTPUT); 

  // Indicator LEDs
  pinMode(redIndicator, OUTPUT); // Red
  pinMode(screenIndicator, OUTPUT); // Yellow & Green
  
  pinMode(redBtnLED1, OUTPUT); // Red Button LED - Right
  pinMode(redBtnLED2, OUTPUT); // Red Button LED - Left
  
  // fill the [previousReaings] array with readings
  for(int i = 0; i < SMOOTH; i++){
    previousReadings1[i] = ADCTouch.read(TOUCHPIN1, RESOLUTION);
  }

  for(int i = 0; i < SMOOTH; i++){
    previousReadings2[i] = ADCTouch.read(TOUCHPIN2, RESOLUTION);
  }

  // Dfplayer serial  
  delay(1000);
  mp3Serial.begin(9600);
  myMP3.begin(mp3Serial);
  // Monitor serial
  Serial.begin(9600);

  pinMode(servoPin, OUTPUT);
  servo.attach(servoPin);

  if(EEPROM.read(pkeVolume) > 0){
    myMP3.volume(EEPROM.read(pkeVolume));
  } else if (EEPROM.read(pkeVolume) == 0) {
    myMP3.volume(15);  //Set volume value. From 0 to 30
  }
  
  timeOut.start();
  sleepPKE.start();
  lightsOff();

  // Resets all EEPROM - leave for testing
//  for (int i = 0 ; i < EEPROM.length() ; i++) {
//    EEPROM.write(i, 0);
//  }
  EEPROM.write(menuSettings, 0);
  EEPROM.write(timerSet, 0);
  EEPROM.write(timerVal, 0);
  EEPROM.write(timerGo, 0);
  delay(20);
  
  // Play PKE bootup sound
  myMP3.playFolder(2, 1);
}

void loop() {
  RemoteXY_Handler ();

  static byte lastbutton1state = HIGH; // last state of button 1
  static byte lastbutton2state = HIGH; // last state of button 2

  reading1 = ADCTouch.read(TOUCHPIN1, RESOLUTION);
  reading2 = ADCTouch.read(TOUCHPIN2, RESOLUTION);
  int val1, val2;

  // check if triggered
  if(reading1 > average1() * multiplier){
    val1 = HIGH;
    // don't use this reading for smoothing
  } else {
    // executes if the sensor is not triggered
    val1 = LOW;
    // use this reading to compensate for environmental changes (i.e. smoothing)
    previousReadings1[currentIndex1] = reading1;

    // set index for the next reading
    currentIndex1++;

    // mnake sure [currentIndex] doesn't get out of bounds
    if(currentIndex1 >= SMOOTH){
      currentIndex1 = 0;
    }
  }  

  // check if triggered
  if(reading2 > average2() * multiplier){
    // executes if the sensor is triggered
    val2 = HIGH;
    // don't use this reading for smoothing
  }else{
    // executes if the sensor is not triggered
    val2 = LOW;
    // use this reading to compensate for environmental changes (i.e. smoothing)
    previousReadings1[currentIndex2] = reading2;

    // set index for the next reading
    currentIndex2++;

    // mnake sure [currentIndex] doesn't get out of bounds
    if(currentIndex2 >= SMOOTH){
      currentIndex2 = 0;
    }
  }

  if(RemoteXY.power == 1 ){
    button1Status = buttonState(RemoteXY.half_open, lastbutton1state);
    button2Status = buttonState(RemoteXY.full_open, lastbutton2state);
  } else if (EEPROM.read(timerGo) == 1) {
    if(EEPROM.read(wingsHalf) == 1) {
      button1Status = buttonState(1, lastbutton1state);
    } else if (EEPROM.read(wingsFull) == 1) {
      button2Status = buttonState(1, lastbutton2state);
    }
  } else {
    button1Status = buttonState(val1, lastbutton1state);
    button2Status = buttonState(val2, lastbutton2state);    
  }
  
  /*** System Status ***/
  static enum {
    NO_TOUCH,
    BUTTON1_TOUCH,
    BUTTON1_MOVE,
    BUTTON2_TOUCH,
    BUTTON2_MOVE,
    ALL_BUTTON_WAIT,
    MENU_BASE
  } state = NO_TOUCH;
  
  static enum {
    ON,
    OFF
  } buttonState = OFF;

  switch (state){ 
    case NO_TOUCH:
      if (button2Status == 1 && EEPROM.read(menuSettings) == 0 || RemoteXY.power == 1 && RemoteXY.half_open == 1) {
        RemoteXY.full_open = 0;
        state = BUTTON2_MOVE;
        digitalWrite(redBtnLED2, HIGH);
        digitalWrite(redIndicator, HIGH);
        sleepPKE.start();
      } else if (button1Status == 1 && EEPROM.read(menuSettings) == 0 || RemoteXY.power == 1 && RemoteXY.full_open == 1) {
        RemoteXY.half_open = 0;
        state = BUTTON1_MOVE;
        digitalWrite(redBtnLED1, HIGH);
        digitalWrite(redIndicator, HIGH);
        sleepPKE.start();
      } else if (EEPROM.read(menuSettings) == 1) {
        state = MENU_BASE;
      } else {
        //Serial.println("NO_TOUCH");
        if(sleepPKE.done()){
          servo.detach();
          digitalWrite(redIndicator, LOW);
        }
        if(timeOut.done()){
          myMP3.stop();
          buttonState = OFF;
          lightsOff();
          digitalWrite(screenIndicator, LOW);
          //Serial.println("timer done");
          if (moveWing.isRunning()) servo.write(moveWing.update());
          moveWing.go(closed, 80, LINEAR, ONCEFORWARD);
        }
      }
      break;
    case BUTTON1_MOVE:
      if (button1Status == 0) {
        state = ALL_BUTTON_WAIT;
        digitalWrite(redBtnLED1, LOW);
      } else {
        servo.attach(servoPin);
        moveWing.go(halfopen, 400, LINEAR, ONCEFORWARD);
        state = BUTTON1_TOUCH;
      }
      break;
    case BUTTON1_TOUCH:
      if (button1Status == 0) {
        state = ALL_BUTTON_WAIT;
        timeOut.start();
        sleepPKE.start();
        digitalWrite(redBtnLED1, LOW);
      } else {
        if (moveWing.isRunning()) {
          servo.write(moveWing.update());
          if(!moveWing.isRunning()) {
            buttonState = ON;
          }
        }
      }
      break;
    case BUTTON2_MOVE:
      if (button2Status == 0) {
        state = ALL_BUTTON_WAIT;
        digitalWrite(redBtnLED2, LOW);
      } else {
        servo.attach(servoPin);
        moveWing.go(fullopen, 400, LINEAR, ONCEFORWARD);
        state = BUTTON2_TOUCH;
      }
      break;
    case BUTTON2_TOUCH:
      if (button2Status == 0) {
        state = ALL_BUTTON_WAIT;
        digitalWrite(redBtnLED2, LOW);
        timeOut.start();
        
      } else {
        if (moveWing.isRunning()) {
          servo.write(moveWing.update());
          if(!moveWing.isRunning()) {
            buttonState = ON;
          }
        }
      }
      break;
    case ALL_BUTTON_WAIT:
      if (button1Status == 1) {
        state = BUTTON1_MOVE;
      } else if (button2Status == 1) {
        state = BUTTON2_MOVE;
      } else {
        if (moveWing.isFinished()) state = NO_TOUCH;
      }
      break;
    case MENU_BASE:
      if(EEPROM.read(menuSettings) == 1) {
        //Serial.println("PKE Meter Menu");
        Serial.println(buttonPresses);
        myMP3.stop();
        lightsOff();
        digitalWrite(screenIndicator, LOW);
        digitalWrite(redIndicator, LOW);
        if (moveWing.isRunning()) servo.write(moveWing.update());
        moveWing.go(closed, 500, LINEAR, ONCEFORWARD);
        shortPressMenu(); 
        if (buttonPresses == 6) {
          buttonPresses = 0;
        }

        static enum {
          MENU_BASE,
          VOLUME,
          LED_PATTERN,
          RESET_DEFAULTS,
          EXIT_MENU
        } menuState = MENU_BASE;

        switch (menuState){ 
          case MENU_BASE:
            runXTimes = 0;
            if (buttonPresses == 0) {
              if (runXTimes < N) {
                myMP3.begin(mp3Serial);
                playPKE(1); // "PKE Meter Menu"
                delay(3000);
                runXTimes++;
              }
              Serial.println("Main Menu");
              runXTimes = 0;
              menuState = VOLUME;
            }
            break;
          case VOLUME:
            runXTimes = 0;
            if (buttonPresses == 1) {
              delay(500);
              runXTimes = 0;
              myMP3.begin(mp3Serial);
              playPKE(11); // "Wing LED Pattern"
              menuState = LED_PATTERN;
            } else {
              Serial.println("Volume");
              if(playOnce == 0){
                myMP3.begin(mp3Serial);
                playPKE(3); // "Volume"
                playOnce = 1;
              }
              int potVol = analogRead(A7);
                  potVol = map(potVol, 0, 1023, 1, 7);
              Serial.println(potVol);
              if(potVol == 6) {
                if (run6Times < N) {
                  if(!myMP3.isPlaying()){
                    if(myMP3.currentVolume() != 30){
                      myMP3.volume(30);
                    }
                    myMP3.volume(30);                    
                  }
                  myMP3.volume(30);
                  EEPROM.write(pkeVolume, 30);
                  myMP3.begin(mp3Serial);
                  playPKE(10); // Volume 6
                  Serial.println("Volume 4");
                  delay(1000);
                  run6Times++;
                }
                run1Times = 0;
                run2Times = 0;
                run3Times = 0;
                run4Times = 0;
                run5Times = 0;
              } else if (potVol == 5) {
                if (run5Times < N) {
                  if(!myMP3.isPlaying()){
                    if(myMP3.currentVolume() != 25){
                      myMP3.volume(25);
                    }
                    myMP3.volume(25);                    
                  }
                  myMP3.volume(25);
                  EEPROM.write(pkeVolume, 25);
                  myMP3.begin(mp3Serial);
                  playPKE(9); // Volume 5
                  Serial.println("Volume 5");
                  delay(1000);
                  run5Times++;
                }
                run1Times = 0;
                run2Times = 0;
                run3Times = 0;
                run4Times = 0;
                run6Times = 0;
              } else if (potVol == 4) {
                if (run4Times < N) {
                  if(!myMP3.isPlaying()){
                    if(myMP3.currentVolume() != 20){
                      myMP3.volume(20);
                    }
                    myMP3.volume(20);                    
                  }
                  myMP3.volume(20);
                  EEPROM.write(pkeVolume, 20);
                  myMP3.begin(mp3Serial);
                  playPKE(8); // Volume 4
                  Serial.println("Volume 4");
                  delay(1000);
                  run4Times++;
                }
                run1Times = 0;
                run2Times = 0;
                run3Times = 0;
                run5Times = 0;
                run6Times = 0;
              } else if (potVol == 3) {
                if (run3Times < N) {
                  if(!myMP3.isPlaying()){
                    if(myMP3.currentVolume() != 15){
                      myMP3.volume(15);
                    }
                    myMP3.volume(15);                    
                  }
                  myMP3.volume(15);
                  EEPROM.write(pkeVolume, 15);
                  myMP3.begin(mp3Serial);
                  playPKE(7); // Volume 3
                  Serial.println("Volume 3");
                  delay(1000);
                  run3Times++;
                }
                run1Times = 0;
                run2Times = 0;
                run4Times = 0;
                run5Times = 0;
                run6Times = 0;
              } else if (potVol == 2) {
                if (run2Times < N) {
                  if(!myMP3.isPlaying()){
                    if(myMP3.currentVolume() != 10){
                      myMP3.volume(10);
                    }
                    myMP3.volume(10);                    
                  }
                  myMP3.volume(10);
                  EEPROM.write(pkeVolume, 10);
                  myMP3.begin(mp3Serial);
                  playPKE(6); // Volume 2
                  Serial.println("Volume 2");
                  delay(1000);
                  run2Times++;
                }
                run1Times = 0;
                run3Times = 0;
                run4Times = 0;
                run5Times = 0;
                run6Times = 0;
              } else if (potVol == 1) {
                if (run1Times < N) {
                  if(!myMP3.isPlaying()){
                    if(myMP3.currentVolume() != 5){
                      myMP3.volume(5);
                    }
                    myMP3.volume(5);                   
                  }
                  myMP3.volume(5);
                  EEPROM.write(pkeVolume, 5);
                  myMP3.begin(mp3Serial);
                  playPKE(5); // Volume 1
                  Serial.println("Volume 1");
                  delay(1000);
                  run1Times++;
                }
                run2Times = 0;
                run3Times = 0;
                run4Times = 0;
                run5Times = 0;
                run6Times = 0;
              }
            }
            if (button2Status == 1) {
              delay(1000);
              runXTimes = 0;
              delay(1000);
              if (runXTimes < N) {
                runXTimes++;
                myMP3.begin(mp3Serial);
                playPKE(4); // "Volume Saved"
                Serial.println("Volume Saved!");
                delay(2000);
                menuState = LED_PATTERN;
              }
            }
            button2Status = 0;
            break;
          case LED_PATTERN:
            if (buttonPresses == 2) {
              delay(500);
              run7Times = 0;
              run8Times = 0;
              menuState = RESET_DEFAULTS;
            } else {
              if (run9Times < N) {
                myMP3.begin(mp3Serial);
                playPKE(11); // "Wing LED Pattern"
                Serial.println("Wing LED Pattern");
                delay(3000);
                button2Status = 0;
                run9Times++;
                run7Times = 0;
                run8Times = 0;
              }
              int potLED = analogRead(A7);
              potLED = map(potLED, 0, 1023, 1, 3);
              if(potLED == 2) {
                EEPROM.write(wingPattern, 1);
                // GB2 light order - 3, 1, 6, 4, 7, 5, 2
                if (run7Times < N) {
                  myMP3.begin(mp3Serial);
                  playPKE(16); // "Ghostbusters 2"
                  Serial.println("Ghostbusters 2");
                  run7Times++;
                  run8Times = 0;
                  uint8_t wingData = 1<<2 | 1<<0; // use bit shifting
                  digitalWrite(latchPinWing, LOW);
                  shiftOut(dataPinWing, clockPinWing, MSBFIRST, wingData);
                  digitalWrite(latchPinWing, HIGH);
                  delay(4000);
                }
              } else if (potLED == 1) {
                EEPROM.write(wingPattern, 0);
                // GB1 light order - 3, 5, 7, 4, 1, 6, 2
                if (run8Times < N) {
                  myMP3.begin(mp3Serial);
                  playPKE(17); // "Ghostbusters 1"
                  Serial.println("Ghostbusters 1");
                  run8Times++;
                  run7Times = 0;
                  uint8_t wingData = 1<<2 | 1<<4; // use bit shifting
                  digitalWrite(latchPinWing, LOW);
                  shiftOut(dataPinWing, clockPinWing, MSBFIRST, wingData);
                  digitalWrite(latchPinWing, HIGH);
                  delay(4000);
                }
              }
              if (button2Status == 1) { // check if button was pressed
                delay(1000);
                if (run10Times < N) {
                  myMP3.begin(mp3Serial);
                  playPKE(19); // "Wing LED Pattern Saved"
                  Serial.println("Wing LED Pattern Saved");
                  delay(2000);
                  run10Times++;
                  menuState = EXIT_MENU;
                }
              }
            }  
            break; 
          case EXIT_MENU:
            if (buttonPresses == 3) {
              delay(500);
              menuState = MENU_BASE;
            } else {
              Serial.println("Exit Menu");
              myMP3.begin(mp3Serial);
              playPKE(15);
              if( button2State == 1 && EEPROM.read(menuSettings) == 1) {
                EEPROM.write(menuSettings, 0);
                button2State = 0;
                state = NO_TOUCH;
              }
            }  
          break;      
        }
      }
    break;
  } 

  switch (buttonState) {
    case ON:
      if (EEPROM.read(menuSettings) == 0) {
        shiftRegister();
        digitalWrite(redIndicator, HIGH);
        digitalWrite(screenIndicator, HIGH);
        indicatorBlink(screenIndicator);
        pot_sound();
      }
      break;
    case OFF:
      if (EEPROM.read(menuSettings) == 0) {

      }
      break;
  }

  longPressMenu();
  timerRemote();
  if(RemoteXY.power == 1) {
    remoteSound();   
  }
}

void writeLongIntoEEPROM(int address, long number){ 
  EEPROM.write(address, (number >> 24) & 0xFF);
  EEPROM.write(address + 1, (number >> 16) & 0xFF);
  EEPROM.write(address + 2, (number >> 8) & 0xFF);
  EEPROM.write(address + 3, number & 0xFF);
}

long readLongFromEEPROM(int address){
  return ((long)EEPROM.read(address) << 24) +
         ((long)EEPROM.read(address + 1) << 16) +
         ((long)EEPROM.read(address + 2) << 8) +
         (long)EEPROM.read(address + 3);
}

void remoteSound() {
  if(RemoteXY.mute == 1){
    myMP3.volume(0);
  } else {
    myMP3.begin(mp3Serial,true);
    myMP3.volume(EEPROM.read(pkeVolume));
  }
}

void timerRemote() {
  unsigned long currMillis = millis();
  unsigned long timer = RemoteXY.timer_num;
  unsigned long lights = RemoteXY.led_speed;
  unsigned long timerSec = timer*60;
  unsigned long timeMilli = timerSec*1000;
  unsigned long setTime = timeMilli;

  // Set all values for what is set before timer goes off
  if(RemoteXY.timer_on == 1 && RemoteXY.power == 0) {
    EEPROM.write(timerSet, 1);
    EEPROM.write(lightsSpd, lights);
    if (RemoteXY.half_open == 1) {
      EEPROM.write(wingsHalf, 1);
    } else if (RemoteXY.full_open == 1) {
      EEPROM.write(wingsFull, 1);
    }
    if(RemoteXY.mute == 1){
      myMP3.volume(0);
      EEPROM.write(pkeVolume, 0);
    } else if (EEPROM.read(pkeVolume) > 0){
      myMP3.volume(EEPROM.read(pkeVolume));
    } else {
      myMP3.volume(30);
    }
    if(RemoteXY.led_pattern == 1){
      EEPROM.write(wingPattern, 1);
    } else {
      EEPROM.write(wingPattern, 0);
    }
  } else if (RemoteXY.power == 1 && RemoteXY.timer_on == 0) {
    EEPROM.write(timerSet, 0);
    EEPROM.write(timerGo, 0);
    EEPROM.write(wingsHalf, 0);
    EEPROM.write(wingsFull, 0);
    writeLongIntoEEPROM(timerVal, 0);
  }
  if(EEPROM.read(timerSet) == 1){
    if (passFlag == 0) {
      writeLongIntoEEPROM(timerVal, setTime);
      unsigned long savedTime = readLongFromEEPROM(timerVal);
      unsigned long prevMillis = 0;
      if (currMillis - prevMillis >= savedTime) {
        prevMillis = currMillis;
        EEPROM.write(timerGo, 1);
        if (EEPROM.read(wingsHalf) == 1) {
          button2Status = 1;
        } else if (EEPROM.read(wingsFull) == 1) {
          button1Status = 1;
        }
        EEPROM.write(timerSet, 0);
        passFlag++;
      }
    }
  }
}

int buttonState(byte buttonPin, byte& lastButtonState){

  byte currentButtonState;

  if(buttonPin == HIGH){
    currentButtonState = HIGH;
  }
  else currentButtonState = LOW;
  
  buttonActionEnum buttonAction = (buttonActionEnum) currentButtonState;

  if (currentButtonState != lastButtonState){

    delay(50); // debounce
    lastButtonState = currentButtonState;

    if (currentButtonState == HIGH){
      buttonAction = ON;
    } else {
      buttonAction = OFF;
    }
  }

  return buttonAction;
}

void longPressMenu() {

  button1State = button1Status;
  button2State = button2Status;
  //Serial.println(EEPROM.read(menuSettings));
  if(button1State == 0 && button2State == 0){
    pressedTime = millis();
    //Serial.println("Waiting for press...");
  } else if (button1State == 1 && button2State == 1) {
    releasedTime = millis(); 
    Serial.println("Holding...");
  }

  pressDuration = releasedTime - pressedTime;
  exitPressDuration = releasedTime - pressedTime;

  if( pressDuration > LONG_PRESS_TIME) {
    EEPROM.write(menuSettings, 1);
    Serial.println("A long press is detected. Entering Menu...");
  }

  lastState1 = button1State;
  lastState2 = button2State;
}

void shortPressMenu() {
  button1State = button1Status;
  button2State = button2Status;
  
  //Serial.println(EEPROM.read(menuSettings));
  if(button1State == 0 && button2State == 0){
    pressedTime = millis();
    //Serial.println("Waiting for press...");
  } else if (button1State == 1) {
    releasedTime1 = millis(); 
  } else if (button2State == 1) {
    releasedTime2 = millis(); 
  }

  pressDuration1 = releasedTime1 - pressedTime;
  pressDuration2 = releasedTime2 - pressedTime;
  //exitPressDuration = releasedTime - pressedTime;

  if( pressDuration2 > SHORT_PRESS_TIME) {
    if (button2Status == 1) { // check if button was pressed
      delay(250); // debounce switch
      buttonPresses++;
    }
  }
  if( pressDuration1 > SHORT_PRESS_TIME) {
    button1Status == 1;   
  }
  lastState1 = button1State;
  lastState2 = button2State;
}

long previousMillis1 = 0;
int n;
int r;
int ledState = LOW;
void shiftRegister() {
  int potValueLED;
  int potValue;
  
  if(RemoteXY.power == 1 ){
    potValue = RemoteXY.led_speed;
    potValueLED = map(potValue, 0, 100, 1, 8);
  } else if (EEPROM.read(timerSet) == 1) {
    potValue = EEPROM.read(lightsSpd);
    potValueLED = map(potValue, 0, 100, 1, 8);
  } else {
    potValue = analogRead(A7);
    potValueLED = map(potValue, 0, 1030, 1, 8);
  }
  int lightSpeed;

  if(potValueLED == 8) {
    lightSpeed = 1200;
  } else if (potValueLED == 7) {
    lightSpeed = 800;
  } else if (potValueLED == 6) {
    lightSpeed = 500;
  } else if (potValueLED == 5) {
    lightSpeed = 375;
  } else if (potValueLED == 4) {
    lightSpeed = 175;
  } else if (potValueLED == 3) {
    lightSpeed = 125;
  } else if (potValueLED == 2) {
    lightSpeed = 75;
  } else if (potValueLED == 1) {
    lightSpeed = 20;
  }

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis1 >= lightSpeed) {
    previousMillis1 = currentMillis;
    
    if (ledState == LOW) {
      n = n + 1;
      r = r + 1;
    }
    if(n == 7){
      n = 0;
      r = 0;
    }
    
    // Wing LEDs
    digitalWrite(latchPinWing, LOW);
    if(EEPROM.read(wingPattern) == 1 || RemoteXY.led_pattern == 1){    
      shiftOut(dataPinWing, clockPinWing, MSBFIRST, wing2[n]);
    } else {
      shiftOut(dataPinWing, clockPinWing, MSBFIRST, wing[n]);      
    }
    digitalWrite(latchPinWing, HIGH);
    // Main Screen LEDs
    digitalWrite(latchPinMain, LOW);
    shiftOut(dataPinMain, clockPinMain, MSBFIRST, mainScreen[r]);
    digitalWrite(latchPinMain, HIGH);

  }
}

void pot_sound(){

  if(RemoteXY.power == 1 ){
    int potValueLED = RemoteXY.led_speed;
    potVal = map(potValueLED, 0, 100, 1, 4);
  } else if (EEPROM.read(timerSet) == 1) {
    int potValueLED = EEPROM.read(lightsSpd);
    potVal = map(potValueLED, 0, 100, 1, 4);
  } else {
    int potValueLED = analogRead(A7);
    potVal = map(potValueLED, 0, 1030, 1, 4);
  }
  unsigned long currentMillis = millis();
  
  isPlaying = false;

  if(button2Status == 0 && button1Status == 0){
    oldPotVal = 0;
  } else {
    if (potVal == 1) {
      trackPlaying(1);
    } else if (potVal == 2) {
      trackPlaying(2);
    } else if (potVal == 3) {
      trackPlaying(3);
    } else {
      //Serial.println("Not playing");
      if (isPlaying == true) {
        isPlaying = false;
      }        
    }
  }
}

void trackPlaying(int track) {
  if (isPlaying == false) {
    isPlaying = true;
    if (oldPotVal != potVal){
      oldPotVal = potVal;
      TrackToPlay = track;
      myMP3.begin(mp3Serial,true);
      myMP3.loop(track);
      //Serial.println((String)"TRACK " + TrackToPlay + " PLAYING");
    }
  }
}

void playPKE(int track) {
  if (!myMP3.isPlaying()) {
    //myMP3.begin(mp3Serial,true);
    myMP3.isPlaying() == true;
    delay(5);
    myMP3.playFolder(1, track);
  }
}

void lightsOff() {
  for (int n = 0; n < 7; n++){
    // Wing Light Pattern and Speed
    digitalWrite(latchPinWing, LOW);
    shiftOut(dataPinWing, clockPinWing, MSBFIRST, 0);
    digitalWrite(latchPinWing, HIGH);
    
    digitalWrite(latchPinMain, LOW);
    shiftOut(dataPinMain, clockPinMain, MSBFIRST, 0);
    digitalWrite(latchPinMain, HIGH);
  }
}
const long onDuration = 100;// OFF time for LED
const long offDuration = 500;// ON time for LED
long previousMillis2 = 0;
int ledState1 = LOW;
void indicatorBlink(int ledPIN) {

  int potValueLED;
  int potValue;
  
  if(RemoteXY.power == 1 ){
    potValue = RemoteXY.led_speed;
    potValueLED = map(potValue, 0, 1030, 1, 6);
  } else if (EEPROM.read(timerSet) == 1) {
    potValue = EEPROM.read(lightsSpd);
    potValueLED = map(potValue, 0, 1030, 1, 6);
  } else {
    potValue = analogRead(A7);
    potValueLED = map(potValue, 0, 1030, 1, 6);
  }
  
  int lightSpeed;
  //Serial.println(potValueLED);
  if (potValueLED == 5) {
    lightSpeed = 1700;
  } else if (potValueLED == 4) {
    lightSpeed = 1500;
  } else if (potValueLED == 3) {
    lightSpeed = 1375;
  } else if (potValueLED == 2) {
    lightSpeed = 875;
  } else if (potValueLED == 1) {
    lightSpeed = 75;
  }

  unsigned long currentMillis = millis();

  if( ledState1 == LOW ){
    if( (currentMillis - previousMillis2) >= onDuration){   
      ledState1 = HIGH;// change the state of LED
      previousMillis2 = currentMillis;// remember Current millis() time
    }
  } else {   
    if( (currentMillis - previousMillis2) >= lightSpeed){     
      ledState1 = LOW;// change the state of LED
      previousMillis2 = currentMillis;// remember Current millis() time
    }
  }
  // set the LED with the ledState of the variable:
  digitalWrite(ledPIN, ledState1);
}
