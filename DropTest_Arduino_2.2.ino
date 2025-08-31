/*
Version started August 24
Main Changes:
Addition of drop counter 
*/

#include <dht.h> //Import library for DHT11
#include <SPI.h> //Import library for SD Card
#include <SD.h>
#include <LiquidCrystal.h> //Import library for LCD screen

//Define pins
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);
#define trigPin 8
#define echoPin 9
#define DHT11_PIN 7
#define DHTTYPE DHT11
const int buttonPin = 6;
const int chipSelect = 4; //For SD Card pin 
dht DHT;

//Setup custom characters
byte pauseChar[8] = {
	0b10001,
	0b10001,
	0b10001,
	0b10001,
	0b10001,
	0b10001,
	0b10001,
	0b00000
};

byte playChar[8] = {
	0b11000,
	0b11100,
	0b11110,
	0b11111,
	0b11110,
	0b11100,
	0b11000,
	0b00000
};

byte errorChar[8] = {
	0b00100,
	0b00100,
	0b00100,
	0b00100,
	0b00100,
	0b00000,
	0b00100,
	0b00000
};

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;

File myFile; 

//Initialize global variables
bool recording = false; 
float minDistance = 500.0; 
float maxDistance = 0.0;
int dropCount = 0;
float previousDistance = 0; 
float distanceChange = 0;
float threshold = 15; //the distance change in mm that should be recognized as a drop
float loopTime = 75; //the time it takes for a single distance check in milliseconds 

void setup() {
  Serial.begin(9600); // Initialize serial communication
  lcd.begin(16,2);

  // Initialize ultrasonic sensor pins
  pinMode(trigPin, OUTPUT); // Set Arduino to send the signal to start the ultrasonic pulse
  pinMode(echoPin, INPUT);  // Set Ultrasonic range finder to send the information about the duration of the trip taken by the ultrasonic pulse to the Arduino

      
  // Initialize SD card 
    
    //Serial.print(F("\nInitializing SD card..."));

    // Check if SD card is working properly
    if (!card.init(SPI_HALF_SPEED, chipSelect)) {
      /*
      Serial.println(F("Initialization failed. Things to check:"));
      Serial.println(F("* is a card inserted?"));
      Serial.println(F("* is your wiring correct?"));
      Serial.println(F("* did you change the chipSelect pin to match your shield or module?"));
      while (1);
    } else {
      Serial.println(F("Wiring is correct and a card is present."));
      */
    }
    
    // Check if the SD library initialization is successful
    if (!SD.begin(4)) {
    /*
      Serial.println(F("Initialization failed!"));
      while (1);
    }
    Serial.println(F("Initialization done."));

    // Print the type of card
    Serial.println();
    Serial.print(F("Card type:         "));
    switch (card.type()) {
      case SD_CARD_TYPE_SD1:
        Serial.println(F("SD1"));
        break;
      case SD_CARD_TYPE_SD2:
        Serial.println(F("SD2"));
        break;
      case SD_CARD_TYPE_SDHC:
        Serial.println(F("SDHC"));
        break;
      default:
        Serial.println(F("Unknown"));
      */
    }

    // Try to open the 'volume'/'partition' - it should be FAT16 or FAT32
    /*
    if (!volume.init(card)) {
      Serial.println(F("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card"));
      while (1);
    }

    // Print some information about the SD card
    Serial.print(F("Clusters:          "));
    Serial.println(volume.clusterCount());
    Serial.print(F("Blocks x Cluster:  "));
    Serial.println(volume.blocksPerCluster());

    Serial.print(F("Total Blocks:      "));
    Serial.println(volume.blocksPerCluster() * volume.clusterCount());
    Serial.println();

    // Print the type and size of the first FAT-type volume
    uint32_t volumesize;
    Serial.print(F("Volume type is:    FAT"));
    Serial.println(volume.fatType(), DEC);

    volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
    volumesize *= volume.clusterCount();       // we'll have a lot of clusters
    volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
    Serial.print(F("Volume size (Kb):  "));
    Serial.println(volumesize);
    Serial.print(F("Volume size (Mb):  "));
    volumesize /= 1024;
    Serial.println(volumesize);
    Serial.print(F("Volume size (Gb):  "));
    Serial.println((float)volumesize / 1024.0);

    // Print information about files found on the card (name, date, and size in bytes)
    Serial.println(F("\nFiles found on the card (name, date, and size in bytes): "));
    root.openRoot(volume);

    // List all files in the card with date and size
    root.ls(LS_R | LS_DATE | LS_SIZE);
    */

  // Initialize the button pin with a pull-up resistor
  pinMode(buttonPin, INPUT_PULLUP);

  // create custom characters
  lcd.createChar(0, pauseChar); 
  lcd.createChar(1, playChar);
  lcd.createChar(2, errorChar);

   //Top left section of LCD
  lcd.setCursor(0,0);
  lcd.print(F("N:"));

  //Top right section of LCD
  lcd.setCursor(9,0);
  lcd.print(F("stat:"));
}

void loop() {
  // Main code to run repeatedly

  // Variables for ultrasonic sensor and recording
  float duration, distance; // Set these variables as floating points (decimals)
  float spdSnd;
  float temperature;
  
  int chk = DHT.read11(DHT11_PIN);
  

  // Button is pressed, start or stop recording
  if (digitalRead(buttonPin) == LOW) {
    if (recording) {
      recording = false;
      myFile.close();
      //Serial.println(F("Recording stopped."));
    } 
    else {
      recording = true;
      myFile = SD.open("test.txt", FILE_WRITE);
      //Serial.println(F("Recording started."));
    }
  }

  // Ultrasonic sensor measurements
  digitalWrite(trigPin, LOW); // Ensures the pin is turned off (low)
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH); // Turn on the pin to initiate the ultrasonic pulses 
  delayMicroseconds(10); // Remain on for 10 microseconds (8 pulses sent)
  digitalWrite(trigPin, LOW); // Turn off 
  
  duration = pulseIn(echoPin, HIGH); // Records time it takes for pulse to return (tuning on echoPin)
  spdSnd = 331.4 + (0.606 * DHT.temperature) + (0.0124 * DHT.humidity); // Calculate the speed of sound 
  distance = (((duration / 2) * (spdSnd / 10000)) / 0.9959 + 0.5748)*10; // Only want to measure distance there, not back. Multiply by speed of sound. Convert to mm. 

   // Update max and min distances if needed 
    if (distance > maxDistance) {
        //Serial.print(F("Distance is greater, new max"));
        maxDistance = distance;
    }
    if (distance < minDistance) {
      //Serial.print(F("Distance is less, new min"));
      minDistance = distance;
    }

  if (recording) {
    lcd.setCursor(15,0);
    lcd.write((byte)1); //Playing symbol
    //Show total height dropped
    lcd.setCursor(9,1);
    lcd.print(F("         ")); //Clear section of screen
    lcd.setCursor(9,1);
    lcd.print("h:");
    int DropDistance = (maxDistance - minDistance);
    lcd.print(DropDistance);
    lcd.print("mm");
    // Write data to file (if the file opened okay, write to it)
    if (myFile) {
      //Serial.println(F("Writing to test.txt..."));
      myFile.println(distance,0);
      //Serial.print("saved to SD");
      //Serial.println(F("done."));
    } 
    else {
      // If the file didn't open, print an error:
      lcd.setCursor(15,0);
      lcd.write((byte)2); //show error symbol
      //Serial.println(F("error opening test.txt"));
    }

   
  //Check if a drop has occured (look for a large, fast change in distance)
  distanceChange = (distance - previousDistance);
  if (distanceChange >= threshold) { //Adjustable based on the speed of the drop and frequency of distance checks
    dropCount = dropCount + 1;
    Serial.print(F("------------------------------------------------------------------------------------------------------------------"));
  }
  }
  else {
    //Serial.println(F("Press button to start recording"));
    lcd.setCursor(15,0);
    lcd.write((byte)0); // Show pause symbol
    lcd.setCursor(9,1);
    lcd.print(F("        "));
  }
 
  //Serial.print(F("min: "));
  //Serial.println(minDistance);
  //Serial.print(F("max: "));
  //Serial.println(maxDistance)

  // Display distance measured on lcd screen
  lcd.setCursor(0,1);
  if (distance >= 4000 || distance <= 20){
    lcd.print(F("Out of range"));
  }
  else {
    lcd.setCursor(0,1);
    lcd.print(F("         ")); //Clear section of screen (fixes "mmm" when change from 3-digit to 2-digit)
    lcd.setCursor(0,1);
    lcd.print("d:");
    lcd.print(distance,0);
    lcd.print("mm");
  }

  // Display number of drops on lcd screen
  lcd.setCursor(2,0);
  lcd.print(dropCount);

  Serial.print("Distance: ");
  Serial.println(distance);
  Serial.print("Previous Distance: ");
  Serial.println(previousDistance);
  previousDistance = distance; 
  delay(loopTime); //Adjustable based on how often a distance check is desired (milliseconds)
  //lcd.clear();
}

  


