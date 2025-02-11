#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <SPI.h>
#include <SD.h>

File deformationRecording;
LiquidCrystal_I2C lcd(0x27,20,4);
HX711 bridge;

const int dataPinBridge = 5;
const int clockPinBridge = 6;
int calibrationValue = 0;

const int SDPin = 10;
const int buttonPin = 4;
Sd2Card card;
long long startTime=0;

int buttonState = 1;
int debounce = 50;
int lastDebounce = 0;
bool isRecording = false;

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(3,0);
  
  bridge.begin(dataPinBridge, clockPinBridge);
  bridge.setScale(calibrationValue); 
  
  pinMode(buttonPin, INPUT_PULLUP);
  
  if(!SD.begin(SDPin)){
    Serial.println("SD initialization failed");
    lcd.clear();
    lcd.print("SD initialization FAILED!");
    while(1);
  }
  Serial.println("SD initialized");
  lcd.print("SD initialized!");
}

void loop() {
  if(isRecording == false){
    bridge.power_down();
    if(deformationRecording)
      deformationRecording.close();
    lcd.clear();
    lcd.print("Waiting for start   command...");
    while(isRecording == false){
      buttonDebounce(digitalRead(buttonPin));
    }

    lcd.clear();
    lcd.print("Initializing test...");
    Serial.print("Initializing test...");
    deformationRecording= SD.open(nameMaker(), FILE_WRITE);
    addCsvLine(deformationRecording, "Current Deformation", "Read Time");
    bridge.power_up();
    startTime = millis();
  }
  int currentDeformation;
  addCsvLine(deformationRecording,String(currentDeformation), String(millis()-startTime));
  buttonDebounce(digitalRead(buttonPin)); //verify test stop
}

void buttonDebounce(int buttonReading){ 
  if(buttonReading != buttonState and lastDebounce == 0)  
    lastDebounce = millis();
  if((millis()-lastDebounce) > debounce){
    if(buttonReading == 1 and buttonState == 0)
      isRecording = !isRecording;
    if(buttonReading != buttonState)
      buttonState = buttonReading;
      lastDebounce = 0;
    }
}

String nameMaker(){
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

void addCsvLine(File file, String deformation, String readTime){
  file.println(readTime);
  file.print(",");
  file.print(deformation);
  }
