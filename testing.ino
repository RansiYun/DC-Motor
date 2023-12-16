#include <LiquidCrystal.h>
#include "arduinoFFT.h"
#include <string.h>
#include <Wire.h>
#include <RTClib.h>

#define SAMPLES 128            //SAMPLES-pt FFT. Must be a base 2 number. Max 128 for Arduino Uno.
#define SAMPLING_FREQUENCY 2100 //Ts = Based on Nyquist, must be 2 times the highest expected frequency.

RTC_DS1307 rtc;
LiquidCrystal lcd(39, 41, 31, 33, 35, 37);                 // LCD pin configuration

// input 1 & 2, enable pin
const int in1 = 13;
const int in2 = 12;
const int enA = 11;

// button configuration
const int bp1 = 2;                                    // button pin
const int bp2 = 3;
const int bp3 = 4;
const int bp4 = 5;
int but1;                                             // button state read variable
int but2;
int but3;
int but4;
int prevButtonState1 = LOW;                           // previous button state to have button stay pressed
int prevButtonState2 = LOW;                           // until another button is pressed
int prevButtonState3 = LOW;
int prevButtonState4 = LOW;

// math
float percent;                                        // power percentage 
float spd = 0;                                        // initial speed of fan
double peak;
String direction = "off";

arduinoFFT FFT = arduinoFFT();
 
unsigned int samplingPeriod;
unsigned long microSeconds;
 
double vReal[SAMPLES]; //create vector of size SAMPLES to hold real values
double vImag[SAMPLES]; //create vector of size SAMPLES to hold imaginary values

void setup() {
  Serial.begin(9600);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(bp1, INPUT_PULLUP);
  pinMode(bp2, INPUT_PULLUP);
  pinMode(bp3, INPUT_PULLUP);
  pinMode(bp4, INPUT_PULLUP);

  samplingPeriod = round(1000000*(1.0/SAMPLING_FREQUENCY)); //Period in microseconds 
  
  Wire.begin();
  rtc.begin();
  lcd.begin(16, 2);     
  
  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }                              // set the LCD dimensions (columns x rows)
  
}

void loop () {
  spinDirection();
  soundInput();
  speedControl();
  LCDdisplay();
  RTC();
  //info();
  Serial.println(direction);
}

////////////////////////////////////////////////////////////////////////SPIN DIRECTION ////////////////////////////////////////////////////////////
void spinDirection() {
  but1 = digitalRead(bp1);                            // read button state
  but2 = digitalRead(bp2);

  if (but1 == HIGH && prevButtonState1 == LOW) {      // button 1 (pin 2) config, C
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    direction = "C";
  }
  
  if (but2 == HIGH && prevButtonState2 == LOW) {      // button 2 (pin 3) config, CC
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    direction = "CC";
  }

  prevButtonState1 = but1;                            // holds previous state
  prevButtonState2 = but2;
}

///////////////////////////////////////////////////////////////////////// SPEED CONTROL ////////////////////////////////////////////////////////////
void speedControl() {
  but3 = digitalRead(bp3);                            // read button state
  but4 = digitalRead(bp4); 

  if (peak >= 257 && peak <= 267) {      // button 3 (pin 4) config, increase speed by 25%
    // Increment the value by 64
    spd = 255;
    if (spd > 255) {
      spd = 255;
    }
   
  }
  
   if (peak >= 431 && peak <= 448) {     // button 4 (pin 5) config, decrease speed by 25%
    // Increment the value by 64
    spd = 191.25;
    if (spd < 0) {
      spd = 0;
    }
   }

  analogWrite(enA, spd);                              // analog write speed to enable pin
  prevButtonState3 = but3;                            // holds previous state
  prevButtonState4 = but4;


                        // calculate power percentage input

}
////////////////////////////////////////////////////////////////////////// LCD DISPLAY /////////////////////////////////////////////////////////////
void LCDdisplay() {
 percent = (spd * 100) / 255;                        // calculate power percentage input
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Speed: ");
  lcd.print(percent);
  lcd.print(" ");
  lcd.print("%");
  lcd.setCursor(0,1);
  lcd.print("Dir: ");
  lcd.setCursor(5, 1);
  lcd.print(direction);

//  lcd.setCursor(0, )
}
///////////////////////////////////////////////////////////////////////// SOUND INPUT /////////////////////////////////////////////////////
void soundInput() {

  {
    for(int i=0; i<SAMPLES; i++)
    {
        microSeconds = micros();    //Returns the number of microseconds since the Arduino board began running the current script. 

        vReal[i] = analogRead(0); //Reads the value from analog pin 0 (A0), quantize it and save it as a real term.
        vImag[i] = 0; //Makes imaginary term 0 always

        //remaining wait time between samples if necessary/
        while(micros() < (microSeconds + samplingPeriod))
        {
          //do nothing
        }
    }
 
    //Perform FFT on samples/
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_BLACKMAN_HARRIS, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

    //Find peak frequency and print peak/
    peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);


    //Script stops here. Hardware reset required.*/

}
}  

void RTC() {
DateTime now = rtc.now();

  lcd.setCursor(8, 1);
  lcd.print(now.hour(), DEC);
  lcd.print(':');
  lcd.print(now.minute(), DEC);
  lcd.print(':');
  lcd.print(now.second(), DEC);

}

void info() {
 Serial.print("Fan Speed: ");
 Serial.print(percent);
 Serial.println("%");
 Serial.print("Frequency: ");
 Serial.println(peak);     //Print out the most dominant frequency.
 Serial.println("--------------------------------------");
 Serial.print("Direction: ");
 Serial.println(direction);
 delay (1000);
}  
  
