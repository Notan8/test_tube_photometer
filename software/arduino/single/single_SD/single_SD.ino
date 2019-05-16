/* This code has a unique optical cell. It can blank and take stand alone measures, but also can be run overnight and
store one sample every given delay and store the reading on an SD card*/
//includes all libraries necessary for this project
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>

//#define CSPIN PA4 // To change depending on Arduino
//SD.begin(4); // To change depending on Arduino

// There are a couple of variables (filename and payload) that is up to you to define!
String payload = "";
int filename; // For now the code is automatically generating different filenames increased by one number whenever you turn on the device
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2; //pin setup for SD card
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); //pin setup for LCD screen
int sensor = A5;  // output pin of OPT101 attached to Analog 5
//pin setup for the sensors/output
int LED = 8;
int buttonBlank = 7;
int buttonMeasure = 6;
int buttonExp = 9;
//variable setup
int blank = 0;
float blankValue = 0;
bool blanked = false;
bool expRunning = false;
int measure = 0;
int startExp = 0;
float OD = 0;
float opt_signal;
File myFile;

void setup() {
  // this will set the starting conditions and run once when the arduino is plugged in
  //start serial + lcd display of optical values.
    Serial.begin(9600);
    delay(100);
    lcd.begin(16,2);
    
    // initialise SD card
    Serial.println(F("Initialising SD card..."));
    if (!SD.begin(4)) {
        Serial.println(F("SD initialisation failed!"));
    }
    else {
        Serial.println(F("SD initialisation done."));
    }

    // open the file. Note that only one file can be open at a time,so you have to close this one before opening another.
    filename = getNextName();
    myFile = SD.open(String(filename) + ".csv", FILE_WRITE);
    
    // if the file opened okay, write to it:
    if (myFile) {
        myFile.println(F("Optical density measurement for E.coli culture"));
        myFile.print(","); myFile.print(filename);
        
        // close the file:
        myFile.close();
    } else {
        // if the file didn't open, print an error:
        Serial.println(F("Error while writing SD"));
    }
    
    // Turn on the blacklight and print a message on LCD.
    lcd.print("   Test  Tube    ");
    lcd.setCursor(0, 1);
    lcd.print("Photometer  v1.1");
    pinMode (LED, OUTPUT); //for LED
    pinMode(buttonBlank, INPUT);
    pinMode(buttonMeasure, INPUT);
}

void loop() {
  measure = digitalRead(buttonMeasure);
  blank = digitalRead(buttonBlank);
  startExp = digitalRead(buttonExp);

  if(blank == HIGH){
   // lcd.clear();
    //lcd.print("Measuring Blank ...");
    //Serial.print("Measure was pressed"); //serial double check
    //Serial.print("\n");
    //Serial.print("Measuring Blank ...");
    //Serial.print("\n");
    //lcd.setCursor(0, 1);
    //lcd.print("Please wait");
    /* ~~~~~~~  those have been blanked because the current experiment does not require either a serial
                 nor an hardware display of the data since the whole thing is running alone over night ~~~~~~*/
    digitalWrite(LED, HIGH);
    delay(500);
    measureBlank(10);
    //lcd.setCursor(0, 1);
    //lcd.print("                ");
    //lcd.setCursor(0, 1);
    //lcd.print("Blanked");
    //Serial.print("Blanked"); //serial double check
    //Serial.print("\n");
    /* ~~~~~~~  those have been blanked because the current experiment does not require either a serial
                 nor an hardware display of the data since the whole thing is running alone over night ~~~~~~*/
  }
  
  if(measure == HIGH){
    lcd.clear();
    lcd.print("Measuring OD ...");
  //  Serial.print("Measuring OD"); //serial double check
   // Serial.print("\n");
    lcd.setCursor(0, 1);
    lcd.print("Please wait");
    digitalWrite(LED, HIGH);
    delay(500);
    measureOD(10);
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(OD);
    Serial.print(OD); //serial double check
    Serial.print("\n");

    if(blanked == false){
      lcd.print("raw value");
      Serial.print("raw value"); //serial double check
      Serial.print("\n");

    }
  }

  if(startExp == HIGH){
    expRunning = !expRunning; //inverts the status of expRunning. for some reason it only works to turn the device on and I can't figure out a way to turn it off
    }
     
  while(expRunning == true){
    //Serial.print("Measuring OD"); //serial double check
    //Serial.print("\n");
    digitalWrite(LED, HIGH);
    delay(500);
    measureOD(10);
    digitalWrite(LED, LOW);
    //Serial.print(OD); //serial double check
    //Serial.print("\n");
    myFile = SD.open(String(filename) + ".csv", FILE_WRITE);
      if (myFile) {
        myFile.println(OD);
        myFile.close();
            }
    delay(300000);
  }
 }
  
void measureOD(int multiplier) {

  opt_signal = 0;

  for (int dummy = 0; dummy < multiplier; dummy++)
  {
    delay(100);
    opt_signal = (opt_signal + analogRead(sensor));
    delay(100);
  }

  if(blanked == true){
    OD = -log10(opt_signal/blankValue);
  }
  else{
    OD = opt_signal/multiplier;
  }
}

void measureBlank(int multiplier) {

  blankValue = 0;
  
  for (int dummy = 0; dummy < multiplier; dummy++)
  {
    delay(100);
    blankValue = (blankValue + analogRead(sensor));
    delay(100);
  }
  blanked = true;

}

// check filenames on SD card and increment by one
int getNextName() {
  File root = SD.open("/");
  int out = 10000;
  while (true) {
    // check file
    File entry = root.openNextFile();

    // if opening the file doesn't return a correct file, we are done
    if (!entry) {
      break;
    }

    // check if the filename is in the correct range. If it is, increment the counter
    String temp = entry.name();
    temp = temp.substring(0 ,5);
    int num = temp.toInt();
    if (num < 99999 && num >= out) {
      out = num + 1;
    }

    // close the file
    entry.close();
  }
  return out;
}
