/*
August 15 2023
Main change: incorportation of SD shield 
*/

#include <LiquidCrystal.h> //Import external library for LCD screen
#include <dht.h> //Import library for DHT11
#include <SPI.h> //Import library for SD Card
#include <SD.h>

#define trigPin 8
#define echoPin 9
#define DHT11_PIN 7
#define DHTTYPE DHT11
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
const int buttonPin = 6;  // Button pin

dht DHT;

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;

const int chipSelect = 4; //For SD Card pin 

File myFile; 
bool recording = false;  // Flag to indicate if recording is active
bool increasing = true; //Flag to indicate if distance is increasing or decreasing (part of drop cycle)
float minDistance = 0.0;
float maxDistance = 0.0;    
float previousDistance = 0.0;

void setup() {
  // put your setup code here, to run once:
Serial.begin (9600);

  pinMode(trigPin, OUTPUT); //Set Arduino to send the signal to start the ultrasonic pulse
  pinMode(echoPin, INPUT); //Set Ultrasonic range finder to send the information about the duration of the trip taken by the ultrasonic pulse to the Arduino
  lcd.begin(16, 2, LCD_5x8DOTS);

Serial.print("\nInitializing SD card...");

  // Check if SD card is working properly
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    while (1);
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }
  
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  // print the type of card
  Serial.println();
  Serial.print("Card type:         ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // Try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    while (1);
  }

  Serial.print("Clusters:          ");
  Serial.println(volume.clusterCount());
  Serial.print("Blocks x Cluster:  ");
  Serial.println(volume.blocksPerCluster());

  Serial.print("Total Blocks:      ");
  Serial.println(volume.blocksPerCluster() * volume.clusterCount());
  Serial.println();

  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("Volume type is:    FAT");
  Serial.println(volume.fatType(), DEC);

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
  Serial.print("Volume size (Kb):  ");
  Serial.println(volumesize);
  Serial.print("Volume size (Mb):  ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Gb):  ");
  Serial.println((float)volumesize / 1024.0);

  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);

 //myFile = SD.open("test.txt", FILE_WRITE);

pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  float duration, distance; //set these variables as floating points (decimals)
  float spdSnd;
  float temperature;
  int chk = DHT.read11(DHT11_PIN);

  // Button is pressed, start or stop recording
  if (digitalRead(buttonPin) == HIGH) {
    if (recording) {
      recording = false;
      myFile.close();
      Serial.println("Recording stopped.");
    } 
    else {
      Serial.print("HERE");
      recording = true;
      myFile = SD.open("test.txt", FILE_WRITE);
      Serial.println("Recording started.");
    }
    }

  digitalWrite(trigPin, LOW); //Ensures the pin is turned off (low)
  delayMicroseconds(2);
 
  digitalWrite(trigPin, HIGH); //Turn on the pin to initiate the ultrasonic pulses 
  delayMicroseconds(10); //Remain on for 10 microseconds (8 pulses sent)
  digitalWrite(trigPin, LOW); //Turn off 
  
  duration = pulseIn(echoPin, HIGH); //Records time it takes for pulse to return (tuning on echoPin)
  spdSnd = 331.4 + (0.606 * DHT.temperature) + (0.0124 * DHT.humidity) ; //Calculate the speed of sound 
  distance = (duration / 2) * (spdSnd / 10000); //Only want to measure distance there, not back. Multiply by speed of sound. 

  //lcd.setCursor(0, 1);
  //lcd.print(DHT.temperature);
  //lcd.print(" C");
  //lcd.print(" ");
  //lcd.print(DHT.humidity);
  //lcd.print("%RH");

  //lcd.setCursor(0, 0);
  if (distance >= 400 || distance <= 2){
    Serial.println("Out of range");
    delay(500);
  }
  else {
    Serial.print(distance);
    Serial.println(" cm");
    delay(500);
  }
  delay(500);
  lcd.clear();

  if (recording) {
    // Write data to file (if the file opened okay, write to it)
    if (myFile) {
      Serial.println("Writing to test.txt...");
      myFile.println(distance);
      Serial.println("done.");
    } 
    else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }
    if (increasing && distance < previousDistance) {
        maxDistance = previousDistance;
        increasing = false;
    }
    if (increasing==false && distance > previousDistance) {
      minDistance = previousDistance;
      increasing = true;
    }
  }
  else {
    minDistance = distance;
    Serial.println("Press button to start recording");
  }
  // Update the previous distance
  previousDistance = distance;
  Serial.print("min: ");
  Serial.println(minDistance);
  Serial.print("max: ");
  Serial.println(maxDistance);
  int DropDistance = (maxDistance - minDistance);
  Serial.println(DropDistance);
  }
  


