/*** Libraries ***/
#include <AltSoftSerial.h>
#include <ServoTimer2.h>
//#include <CapacitiveSensor.h>
#include <ADCTouch.h>
#include <Ramp.h>
#include <RemoteXY.h>
#include <EEPROM.h>
#include <Chrono.h>
#include <DFRobotDFPlayerMini.h>

/*** DfPlayer Setup ***/
int serial1 = 8; // RX
int serial2 = 9; // TX
AltSoftSerial mp3Serial(serial1, serial2); // RX, TX
DFRobotDFPlayerMini myMP3;
int play_in_progress = 0;

/*** RemoteXY Setup ***/
#define REMOTEXY_MODE__HARDSERIAL
#define REMOTEXY_SERIAL Serial
#define REMOTEXY_SERIAL_SPEED 9600

/*** Servo Setup ***/
// define the pins for the servos
#define servoPin 12
ServoTimer2 servo;    // declare variables for up to eight servos
rampUnsignedInt moveWing;

/*** Touch Button Setup ***/
int ref1, ref2;

#define TOUCHPIN1 A0
#define TOUCHPIN2 A1
#define RESOLUTION 50
#define SMOOTH 100
// determen when the sensor is understood as "ON"
float multiplier = 1.4;

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

const int LONG_PRESS_TIME = 10000; // 500 milliseconds

enum buttonActionEnum{
  ON = LOW,
  OFF = HIGH
} state = OFF;

/*** LED Setup ***/
// Define which pins will be used for 
// the Shift Register controls
// Wing LEDs
#define dataPinWing 7 
#define latchPinWing 6
#define clockPinWing 5

// Main Screen LEDs
//#define dataPinMain 4
//#define latchPinMain 3
//#define clockPinMain 2

// GB1 light order - 3, 5, 7, 4, 1, 6, 2
// GB2 light order - 3, 1, 6, 4, 7, 5, 2
// Byte sequential order 1,2,4,16,8,32,64

//int pattern1[7] = {4,8,64,16,1,32,2};
//int pattern2[7] = {4,1,32,16,64,8,2};

int wing[7] = {4,8,64,16,1,32,2};
int mainScreen[7] = {1,2,4,8,16,32,64};

/*** Variables ***/
// Wing Positions
int fullopen = 1801;
int halfopen = 1255;
int closed = 750;
int lightspeed;

const unsigned long turnOnADelay = 0; // wait to turn on LED AFTER PRESSING BUTTON 1000=1SEC
const unsigned long turnOffADelay = 1500; // turn off LED after this time ONCE BUTTON IS RELEASED 1000=1SEC
const unsigned long turnOnBDelay = 0; // wait to turn on LED AFTER PRESSING BUTTON 1000=1SEC
const unsigned long turnOffBDelay = 1500; // turn off LED after this time ONCE BUTTON IS RELEASED 1000=1SEC

void setup() {
  // DfPlayer pin setup
  //pinMode(serial1, OUTPUT);
  ///pinMode(serial2, OUTPUT);

  // Configure Wing Pins
  pinMode(dataPinWing, OUTPUT);
  pinMode(latchPinWing, OUTPUT);
  pinMode(clockPinWing, OUTPUT);

  // fill the [previousReaings] array with readings
  for(int i = 0; i < SMOOTH; i++){
    previousReadings1[i] = ADCTouch.read(TOUCHPIN1, RESOLUTION);
  }

  for(int i = 0; i < SMOOTH; i++){
    previousReadings2[i] = ADCTouch.read(TOUCHPIN2, RESOLUTION);
  }

  // Dfplayer serial
  mp3Serial.begin(9600);
  // Monitor serial
  Serial.begin(115200);

//  sensor1.set_CS_AutocaL_Millis(0xFFFFFFFF);
//  sensor2.set_CS_AutocaL_Millis(0xFFFFFFFF);
//  sensor1.set_CS_Timeout_Millis(120);
//  sensor2.set_CS_Timeout_Millis(120);

  for (int n = 0; n < 7; n++){
    // Wing Light Pattern and Speed
    digitalWrite(latchPinWing, LOW);
    shiftOut(dataPinWing, clockPinWing, MSBFIRST, 0);
    digitalWrite(latchPinWing, HIGH);
  }

  //ref1 = ADCTouch.read(A0, 5000);
  //ref2 = ADCTouch.read(A1, 5000);

  if (!myMP3.begin(mp3Serial)) {
    Serial.println(F("Unable to begin:"));
    Serial.println(F("Please recheck the connections!"));
  } else {
    Serial.println(F("DFPlayer Mini online."));
  }

  servo.attach(12);
  pinMode(12, OUTPUT);
  delay(1000);
  servo.write(750);

  myMP3.volume(30);  //Set volume value. From 0 to 30
  //myMP3.wakeUp();
  myMP3.play(1);
  delay(3000);
  myMP3.stop();
}

void loop() {

  static byte lastbutton1state = HIGH; // last state of button 1
  static byte lastbutton2state = HIGH; // last state of button 2

  reading1 = ADCTouch.read(TOUCHPIN1, RESOLUTION);
  reading2 = ADCTouch.read(TOUCHPIN2, RESOLUTION);
  int val1, val2;

  // check if triggered
  if(reading1 > average1() * multiplier){
    // executes if the sensor is triggered
    //Serial.println("HIGH");
    val1 = HIGH;
    // don't use this reading for smoothing
  }else{
    // executes if the sensor is not triggered
    //Serial.println("LOW");
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
    //Serial.println("HIGH");
    val2 = HIGH;
    // don't use this reading for smoothing
  }else{
    // executes if the sensor is not triggered
    //Serial.println("LOW");
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
  
  //static stateEnum sysState = IDLE;
  static unsigned long delayMs; // used for delay states
  static unsigned long turnOffDelay; // used for delay states
  unsigned long currentMillis = millis();
  
  int button1Status = buttonState(val1, lastbutton1state);
  int button2Status = buttonState(val2, lastbutton2state);

  for (int n = 0; n < 7; n++){
    // Wing Light Pattern and Speed
    digitalWrite(latchPinWing, LOW);
    shiftOut(dataPinWing, clockPinWing, MSBFIRST, 0);
    digitalWrite(latchPinWing, HIGH);
  }

  /*** System Status ***/
  static enum {
    NO_TOUCH,
    BUTTON1_TOUCH,
    BUTTON1_MOVE,
    BUTTON2_TOUCH,
    BUTTON2_MOVE
  } state = NO_TOUCH;
  
  static enum {
    ON,
    OFF
  } buttonState = OFF;
  
  switch (state){ 
    case NO_TOUCH:
      if (button2Status == 1) {
        delayMs = currentMillis;
        state = BUTTON2_MOVE;
      } else if (button1Status == 1) {
        delayMs = currentMillis;
        state = BUTTON1_MOVE;
      } if(button1Status == 0 || button2Status == 0){
        if (moveWing.isRunning()) servo.write(moveWing.update());
        moveWing.go(closed, 500, LINEAR, ONCEFORWARD);
        Serial.println("NO_TOUCH");
        myMP3.disableLoop();
        myMP3.stop();
        buttonState = OFF;
      }
      break;
    case BUTTON1_MOVE:
      if (button1Status == 0) {
        state = NO_TOUCH;
      } else if (currentMillis - delayMs >= turnOnADelay) {
        myMP3.EQ(DFPLAYER_EQ_NORMAL);
        moveWing.go(fullopen, 500, LINEAR, ONCEFORWARD);
        state = BUTTON1_TOUCH;
      }
      break;
    case BUTTON1_TOUCH:
      if (button1Status == 0) {
        delayMs = currentMillis; // update the delay time when button was pushed
        turnOffDelay = turnOffADelay;
        moveWing.go(closed, 500, LINEAR, ONCEFORWARD);
        state = NO_TOUCH;
        myMP3.disableLoop();
        myMP3.stop();
      } else {
        if (moveWing.isRunning()) {
          servo.write(moveWing.update());
          if(!moveWing.isRunning() && servo.read() == fullopen) {
            play_in_progress = 1;
            PlayTrack(2);
            buttonState = ON;
          }
        }
      }
      break;
    case BUTTON2_MOVE:
      if (button2Status == 0) {
        state = NO_TOUCH;
        myMP3.disableLoop();
        myMP3.stop();
      } else if (currentMillis - delayMs >= turnOnADelay) {
        myMP3.EQ(DFPLAYER_EQ_NORMAL);
        moveWing.go(halfopen, 500, LINEAR, ONCEFORWARD);
        state = BUTTON2_TOUCH;
      }
      break;
    case BUTTON2_TOUCH:
      if (button2Status == 0) {
        delayMs = currentMillis; // update the delay time when button was pushed
        turnOffDelay = turnOffADelay;
        moveWing.go(closed, 500, LINEAR, ONCEFORWARD);
        state = NO_TOUCH;
        myMP3.disableLoop();
        myMP3.stop();
      } else {
        if (moveWing.isRunning()) {
          servo.write(moveWing.update());
          if(!moveWing.isRunning() && servo.read() == halfopen) {
            play_in_progress = 1;
            PlayTrack(1);
            buttonState = ON;
          }
        }
      }
      break;
  }

  switch (buttonState) {
    case ON:
      shiftRegister();
      break;
    case OFF:
      break;
  }



//  Serial.print(val1);
//  Serial.print("\t"); 
//  Serial.print(val2);
//  Serial.print("\t"); 
//  Serial.println(button1Status);
//  Serial.print("\t"); 
//  Serial.println(button2Status);
//  Serial.print("\t"); 
//  Serial.print(servo.read());
//  Serial.print("\t"); 
//  Serial.print(digitalRead(11));
//  Serial.print("\t");  
//  Serial.println(myMP3.readState());

}

int buttonState(byte buttonPin, byte& lastButtonState){

  byte currentButtonState;
Serial.println(buttonPin);
  if(buttonPin == HIGH) currentButtonState = HIGH;
  else currentButtonState = LOW;
  
  // Set the current button state.  This will be overwritten if a transition
  // occurred.
  buttonActionEnum buttonAction = (buttonActionEnum) currentButtonState;

  if (currentButtonState != lastButtonState){
    // Button state has changed!  Debounce then determine the transition
    // that happened.

    delay(90); // debounce
    lastButtonState = currentButtonState;

    // Determine the transition based on the current state
    if (currentButtonState == HIGH){
      // Button has transitioned to ON
      buttonAction = ON;
    } else { // currentButtonState == HIGH
      // Button has transitioned to off
      buttonAction = OFF;
      myMP3.disableLoop();
      myMP3.stop();
    }
  }

  return buttonAction;
}

long previousMillis = 0;
boolean timeHasCome = false;
void shiftRegister() {
  int n;
  // read pot data
  int potValue = analogRead(A7);
  // map the pot range and set a min and max range for the speed

  int lightSpeed = map(potValue, 0, 1020, 0, 150);
  Serial.println(lightSpeed);
  
  // put your main code here, to run repeatedly:
  int  timeHasCome = millis() - previousMillis >= lightSpeed;
  if (timeHasCome) {
    if (digitalRead(latchPinWing) == HIGH) {
      n = n + 1;
    } 
    if(n == 7){
      n = 0;
    }
      // Main Screen Speed (pattern in determined by long press on both buttons)
    digitalWrite(latchPinWing, LOW);
    shiftOut(dataPinWing, clockPinWing, MSBFIRST, wing[n]);
    digitalWrite(latchPinWing, HIGH);
    
    // Main Screen Speed (pattern in determined by long press on both buttons)
    //digitalWrite(dataPinMain, LOW);
    //shiftOut(latchPinMain, clockPinWing, MSBFIRST, mainScreen[n]);
    //digitalWrite(clockPinMain, HIGH);
    previousMillis = millis();
  }
}

int longPress(int button1Status, int button2Status) {
  int pressed;
  int longPress;
  int pressedTime;
  int releasedTime;

  if(button1Status == HIGH && button2Status == HIGH){
    pressed = 1;
    pressedTime = millis();
  } else {
    pressed = 0;
    releasedTime = millis();
  }

  // if(lastButtonState == HIGH && currentButtonState == LOW) {
  //   pressedTime = millis();
  // }

  // if(lastButtonState == HIGH && currentButtonState == LOW) { // button is pressed
  //   pressedTime = millis();
  // } else if (lastButtonState == LOW && currentButtonState == HIGH) { // button is released
  //   releasedTime = millis();
  // }
  long pressDuration = pressedTime;

  if (pressDuration > LONG_PRESS_TIME) {
    //Serial.println("A long press is detected");
  }
  return longPress;
}

void PlayTrack(int TrackToPlay){
  static int TrackPlaying = -1;

  if (TrackPlaying != TrackToPlay) { // Not playing the required track
    myMP3.stop(); // Stop what you're doing, if anything
    myMP3.volume(0);
    myMP3.volume(30);
    myMP3.loop(TrackToPlay);     // Play requested track, VOLUME 30 (OUT OF 30)
    TrackPlaying = TrackToPlay;             // Note the track now playing
  } else if (play_in_progress == 1) {      // Track number is right but is it still playing?
    myMP3.loop(TrackToPlay);     // No, so play it again, VOLUME 30 (OUT OF 30)
  } 
}
