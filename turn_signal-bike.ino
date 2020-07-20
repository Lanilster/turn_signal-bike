#include <FastLED.h>
//#include "Timer.h"
#include <EEPROM.h>
//Timer t, t2;

#define LED_PIN     7  //Pin led used to control led stripe
#define NUM_LEDS    30 //number of leds
//Colors 
#define BLACK   CRGB ( 0, 0, 0) //Black
#define ORANGE  CRGB ( 255, 69, 0) //Specific Orange for my LED stripe
#define RED     CRGB ( 255, 0, 0) //Specific Red for my LED stripe
CRGB leds[NUM_LEDS];

//PIN 
#define PIN_buttonWarning  18    //the number of the pushbutton pinn
#define PIN_buttonLeft      3    //the number of the pushbutton pin
#define PIN_buttonRight     2    //the number of the pushbutton pin
#define PIN_led_indicator_Right  51  //the number of the led indicator pin
#define PIN_led_indicator_Left   50  //the number of the led indicator pin

// variables will change:
volatile bool rightTurnState = false; //right Turn sate ON or OFF
volatile bool leftTurnState = false;  //left Turn sate ON or OFF

unsigned long interruptPreviousMilis = 0; //protection against button double tap
volatile bool ledsState = false;  //enable/disable blinking 
volatile bool doublePressLeft = false;
volatile bool doublePressRight = false;

volatile byte ledSetState = false;
volatile int ledCount = 10;

int blinkModeAddr = 0;            //EEPROM address to save mode
volatile int blinkMode = EEPROM.read(0) ;       //0=classic blink, 1= chase mode

void setup() {
  Serial.begin(9600);
  pinMode (PIN_led_indicator_Left, OUTPUT);
  pinMode (PIN_led_indicator_Right, OUTPUT);  
  noInterrupts();
  //TIMER1_SETUP
  TCCR1A = 0; //65536 for 16bit timer
  TCCR1B = 0;
  TCNT1  = 0;
  // Prescaler 1024
  bitSet (TCCR1B, CS10); // 
  bitClear (TCCR1B, CS11); //  
  bitSet (TCCR1B, CS12); //  
  OCR1A = 7813;            // compare match register 16MHz/256/2Hz
  TCCR1B |= (1 << WGM12);   // CTC mode  
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  //END TIMER1_SETUP

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);  
  pinMode (PIN_buttonLeft, INPUT_PULLUP);
  pinMode (PIN_buttonRight, INPUT_PULLUP);
  pinMode (PIN_buttonWarning, INPUT_PULLUP);  
  interrupts();
  attachInterrupt(digitalPinToInterrupt(PIN_buttonLeft), switchLeftButton, RISING );
  attachInterrupt(digitalPinToInterrupt(PIN_buttonRight), switchRightButton, RISING ); 
 
  resetLeds();
}

void loop() {
  //if blinking is enable
  if(ledsState){
    if(!ledSetState){
      if(leftTurnState){
        leftTurnLight();
      }else if (rightTurnState ){
        rightTurnLight();
      }
      ledSetState = true;
      ledCount -=1;
    }
  }else{
    resetLeds();
    ledSetState = false;
    if(ledCount==0){
      leftTurnState  = false;
      rightTurnState = false;
      ledCount = 10;
    }
  }
  // Recommence la séquence
}

ISR(TIMER1_COMPA_vect){
   ledsState = !ledsState;
}

/**
 *  This function is called when left button is released after been pressed
 */
void switchLeftButton(){
  //avoid buncing on the button
  if(millis() - interruptPreviousMilis >= 500){
    //verify if it's a double press
    if(digitalRead(PIN_buttonRight) == LOW){
      //double press detected, save that 
      doublePressRight = true;
      changeBlinkMode();      //DOUBLE PRESS ACTION
    }else{//no double press
      //protection against button release after double press
      if(!doublePressLeft){
        rightTurnState = false; 
        leftTurnState = !leftTurnState; //toggle own state
        ledCount =10;
        Serial.println("switchLeftButton;leftTurnState toggle (rightTurnState disable)"); 
      }else{
        //double press has been used, reset value
        doublePressLeft = false;
      }
    }
    //update bouncing limit
    interruptPreviousMilis = millis(); 
  }
}

/**
 *  This function is called when right button is released after been pressed
 */
void switchRightButton(){
  //avoid buncing on the button
  if(millis() - interruptPreviousMilis >= 500){
    //verify if it's a double press
    if(digitalRead(PIN_buttonLeft) == LOW){
      //double press detected, save that 
      doublePressLeft = true;
    }else{
      //protection against button release after double press
      if(!doublePressRight){        //if there is no previous press on the other
        //disable other state
        leftTurnState = false;  
        //toggle own state 
        rightTurnState = !rightTurnState;
        ledCount =10;
        Serial.println("switchRightButton;rightTurnState toggle (leftTurnState disable)"); 
      }else{
        //double press has been used, reset value
        doublePressRight = false; 
      }
    }
    //update bouncing limit
    interruptPreviousMilis = millis();
  }
}

/**
 * This function make light blink as left turn 
 */
void leftTurnLight(){
  //set indicator
  digitalWrite (PIN_led_indicator_Left, HIGH) ; 
  if(blinkMode == 0){
     for (int i = 7; i >= 0; i--) {
      leds[i] = ORANGE; //Orange 
    }
    FastLED.show();
  }else if (blinkMode == 1){
    for (int i = 7; i >= 0; i--) {
      leds[i] = ORANGE; //Orange 
      FastLED.show();
      delay(40);
    }
  }
}

/**
 * This function make light blink as right turn 
 */
void rightTurnLight(){
  //set indicator
  digitalWrite (PIN_led_indicator_Right, HIGH) ; 
  if(blinkMode == 0){
    for (int i = 22; i < NUM_LEDS; i++) {
      leds[i] = ORANGE; //Orange
    }
    FastLED.show();
  }else if (blinkMode == 1){
    for (int i = 22; i < NUM_LEDS; i++) {
      leds[i] = ORANGE; //Orange
      FastLED.show();
      delay(40);
    }
  }
}

/**
 * This function turn off all the Leds
 */
void resetLeds(){
  for (int i = 7; i >= 0; i--) {
      leds[i] = BLACK; 
  }
  for (int i = 22; i < NUM_LEDS; i++) {
     leds[i] = BLACK;
  }
  FastLED.show();
  //reset indicators
  digitalWrite (PIN_led_indicator_Left, LOW) ; 
  digitalWrite (PIN_led_indicator_Right, LOW) ; 
}

/**
 * This function change the type of blinking led
 */
void changeBlinkMode(){
  if(blinkMode == 1){
    blinkMode = 0;
    EEPROM.update(blinkModeAddr, blinkMode);
  }else{
    blinkMode++;
    EEPROM.update(blinkModeAddr, blinkMode);
  }
}
