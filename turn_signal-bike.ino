#include <FastLED.h>
#include "Timer.h"
Timer t;

#define LED_PIN     7
#define NUM_LEDS    30
CRGB leds[NUM_LEDS];

const int buttonWarningPin = 18; //the number of the pushbutton pin
const int buttonLeftPin = 3;    //the number of the pushbutton pin
const int buttonRightPin = 2;   //the number of the pushbutton pin
const int led_indicator_Right = 51;
const int led_indicator_Left = 50;


// variables will change:
volatile bool rightTurnState = false; //right Turn sate ON or OFF
volatile bool leftTurnState = false;  //left Turn sate ON or OFF
volatile bool warningState = false;  //warning sate ON or OFF
unsigned long interruptPreviousMilis = 0; //protection against button double tap
volatile bool ledsState = false;  //enable/disable blinking 
volatile int blinkMode = 0;       //0=classic blink, 1= chase mode
volatile bool doublePressLeft = false;
volatile bool doublePressRight = false;

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);  
  t.every(500, halfSecond ); 
  
  pinMode (led_indicator_Left, OUTPUT);
  pinMode (led_indicator_Right, OUTPUT);  
  pinMode (buttonLeftPin, INPUT_PULLUP);
  pinMode (buttonRightPin, INPUT_PULLUP);
  pinMode (buttonWarningPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonLeftPin), switchLeftButton, RISING );
  attachInterrupt(digitalPinToInterrupt(buttonRightPin), switchRightButton, RISING ); 
  attachInterrupt(digitalPinToInterrupt(buttonWarningPin), switchWarningButton, RISING ); 
  resetLeds();
}

void loop() {
   //if blinking is enable
  if(leftTurnState || rightTurnState || warningState){
    t.update();    
  }else{
    //blinking disable ; turn off led
    resetLeds();
    //prepare for next call
    ledsState = true; 
  }
  // Recommence la séquence
}

/**
 *  This function is called every 500ms = half seconds
 */
void halfSecond(){
  //if led can be turned on
  if(ledsState){
    //Serial.println("");
    //Serial.println("");
    //verify which led to turn on
    if(leftTurnState){
      //Serial.println("leftTurnState");
      leftTurnLight();
    }else if(rightTurnState){
      //Serial.println("rightTurnState");
      rightTurnLight();
    }else if(warningState){
      warningLight();
    }
  }else{
    //turn off leds
    //Serial.println("resetLeds");
    resetLeds();
  }
  //toggle led state to create blink
  ledsState = !ledsState;
}

/**
 *  This function is called when left button is released after been pressed
 */
void switchLeftButton(){
  //avoid buncing on the button
  if(millis() - interruptPreviousMilis >= 120){
    //verify if it's a double press
    if(digitalRead(buttonRightPin) == LOW){
      //double press detected, save that 
      doublePressRight = true;
      changeBlinkMode();      //DOUBLE PRESS ACTION
    }else{//no double press
      //protection against button release after double press
      if(!doublePressLeft){
        if(warningState){
          warningState = false;
          //Serial.println("switchLeftButton;warningState disable");
        }else{
        //disable other state      
          rightTurnState = false; 
          leftTurnState = !leftTurnState; //toggle own state
          //Serial.println("switchLeftButton;leftTurnState toggle (rightTurnState disable)");          
        }
      }else{
        //double press has been used, reset value
        //Serial.println("switchLeftButton;doublePressLeft disable");
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
  if(millis() - interruptPreviousMilis >= 120){
    //verify if it's a double press
    if(digitalRead(buttonLeftPin) == LOW){
      //double press detected, save that 
      doublePressLeft = true;
      //enable warning
      //enableWarningLight();      //DOUBLE PRESS ACTION
    }else{
      //protection against button release after double press
      if(!doublePressRight){        //if there is no previous press on the other 
        if(warningState){
          warningState = false;
        }else{
          //disable other state
          leftTurnState = false;  
          //toggle own state 
          rightTurnState = !rightTurnState;         
        }
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
 *  This function is called when warning button is released after been pressed
 */
void switchWarningButton(){
   //avoid buncing on the button
  if(millis() - interruptPreviousMilis >= 120){
    //if warningState is already enable, disable it
    if(warningState){
      warningState = false;
    }else{
       //enable warning
       enableWarningLight();
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
  digitalWrite (led_indicator_Left, HIGH) ; 
  if(blinkMode == 0){
     for (int i = 5; i >= 0; i--) {
      leds[i] = CRGB ( 255,69,0); //Orange 
    }
    FastLED.show();
  }else if (blinkMode == 1){
    for (int i = 5; i >= 0; i--) {
      leds[i] = CRGB ( 255,69,0); //Orange 
      FastLED.show();
      delay(60);
    }
  }
}

/**
 * This function make light blink as right turn 
 */
void rightTurnLight(){
  //set indicator
  digitalWrite (led_indicator_Right, HIGH) ; 
  if(blinkMode == 0){
    for (int i = 24; i < NUM_LEDS; i++) {
      leds[i] = CRGB ( 255, 69, 0); //Orange
    }
    FastLED.show();
  }else if (blinkMode == 1){
    for (int i = 24; i < NUM_LEDS; i++) {
      leds[i] = CRGB ( 255, 69, 0); //Orange
      FastLED.show();
      delay(60);
    }
  }
}

/**
 * This function make light blink as warning 
 */
void warningLight(){
  //set indicators
  digitalWrite (led_indicator_Left, HIGH) ; 
  digitalWrite (led_indicator_Right, HIGH) ; 
  //left 
  for (int i = 5; i >= 0; i--) {
    leds[i] = CRGB ( 255,69,0); //Orange 
  }
  //right 
  for (int i = 24; i < NUM_LEDS; i++) {
    leds[i] = CRGB ( 255, 69, 0); //Orange
  }
  FastLED.show();  
}

/**
 * This function turn off all the Leds
 */
void resetLeds(){
  for (int led = 0; led < NUM_LEDS; led++) {
    leds[led] = CRGB::Black; //off
  }
  FastLED.show();
  //reset indicators
  digitalWrite (led_indicator_Left, LOW) ; 
  digitalWrite (led_indicator_Right, LOW) ; 
}

/**
 * This function change the type of blinking led
 */
void changeBlinkMode(){
  if(blinkMode == 1){
    blinkMode = 0;
  }else{
    blinkMode++;
  }
}

/**
 * This function disable turn light, enable warning light
 */
void enableWarningLight(){
  //disable others light
  rightTurnState = false; 
  leftTurnState = false;  
  //enable warning state
  warningState = true;
}
