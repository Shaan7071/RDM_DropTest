#include <LiquidCrystal.h> //Import external library for LCD screen
#include <math.h> //Import extrnal library for thermistor

#define trigPin 10
#define echoPin 13
LiquidCrystal lcd(12, 11, 5, 4, 3, 2); //Assign pin numbers to the LCD module (4-bit mode)

//Steinhart-Hart equation  used to convert the thermistor resistance values to temperature
double Thermistor(int RawADC) {
 double Temp;
 Temp = log(10000.0*((1024.0/RawADC-1))); 
 Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
 Temp = Temp - 273.15;          
 return Temp;
}

void setup() {
  // put your setup code here, to run once:
Serial.begin (9600);
  pinMode(trigPin, OUTPUT); //Set Arduino to send the signal to start the ultrasonic pulse
  pinMode(echoPin, INPUT); //Set Ultrasonic range finder to send the information about the duration of the trip taken by the ultrasonic pulse to the Arduino
}

void loop() {
  // put your main code here, to run repeatedly:
  
  int val;                
  double temp;            
  val=analogRead(0);      
  temp=Thermistor(val);

 float duration, distance; //set these variables as floating points (decimals)
  digitalWrite(trigPin, LOW); //Ensures the pin is turned off (low)
  delayMicroseconds(2);
 
  digitalWrite(trigPin, HIGH); //Turn on the pin to initiate the ultrasonic pulses 
  delayMicroseconds(10); //Remain on for 10 microseconds (8 pulses sent)
  digitalWrite(trigPin, LOW); //Turn off 
  
  duration = pulseIn(echoPin, HIGH); //Records time it takes for pulse to return (tuning on echoPin)
  spdSnd = 331.4 + (0.606 * temp) + (0.0124 * 50); //Calculate the speed of sound (using 50% RH)
  distance = (duration / 2) * (spdSnd / 10000); //Only want to measure distance there, not back. Multiply by speed of sound. 
  
  //If distance is out of range
  if (distance >= 400 || distance <= 2){
    lcd.print("Out of range"); //Print "Out of range" on the LCD screen
    delay(500); //Display message on the LCD screen for 500 milliseconds (0.5 seconds)
  }
  else {
    lcd.print(distance);//Print the distance on the LCD screen
    lcd.print(" cm");
    delay(500);; //Display message on the LCD screen for 500 milliseconds (0.5 seconds)
  }

  delay(500); //Wait 500 milliseconds before checking the distance again
  lcd.clear();
}
