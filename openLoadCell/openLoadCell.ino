#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <SPI.h>
#include <SD.h>

File deformationRecording;
LiquidCrystal_I2C lcd(0x27,20,4);
HX711 bridge;

const int dataPinBridge = 5;
const int clockPinBridge = 6;

const int SDPin = 10;
const int buttonPin = 4;

int startState = LOW;
int debounce = 3;
int lastDebounce = 0;

void setup() {
  Serial.begin(11500);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(3,0);
  

  bridge.begin(dataPinBridge, clockPinBridge);

  pinMode(buttonPin, INPUT_PULLUP);
  
  if(!SD.begin(SDPin)){
    Serial.println("SD initialization failed");
    lcd.clear();
    lcd.print("SD initialization FAILED!");
    while(1);
  }
  Serial.println("SD initialized");
  lcd.print("SD initialized!");
  
  lcd.clear();
  lcd.print("Waiting for start command");
  while(startState = LOW){
    buttonDebounce(digitalRead(buttonPin), startState);
  }

  lcd.clear();
  lcd.print("Initializing test...");

  deformationRecording.open(nameMaker(), FILE_WRITE);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void buttonDebounce(int buttonReading, int &oldState){
  if(buttonReading != oldState)
    lastDebounce = millis();
  if(millis()-lastDebounce > debounce){
    oldState = buttonReading;
    }
}

int nameMaker(){
  String filename;
  int testNumber = 0;
  String prefix = "test";
  String suffix = ".csv";
  filename = prefix + testNumber + suffix;
  while(SD.exists(filename)){
    testNumber++;
    filename = prefix + testNumber + suffix;  
  }
  return filename;
}
