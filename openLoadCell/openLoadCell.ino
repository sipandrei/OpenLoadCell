// The format of a line of the config file SHALL be: <nominal resistance>,<compensation factor>
// First line of config.txt SHALL be the header

#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <SPI.h>
#include <SD.h>
#include <CSV_Parser.h>

File deformationRecording, configFile;
LiquidCrystal_I2C lcd(0x27,20,4);
HX711 bridge;

float baseVoltage = 5.0;
float wheatstoneVoltage = 0;
float comp[4];
float res[4];

const int dataPinBridge = 5;
const int clockPinBridge = 6;
int calibrationValue = 0;

const int SDPin = 10;
const int buttonPin = 4;
Sd2Card card;
long long startTime=0;
CSV_Parser cp("ff");

char feedRowParser() {
  return configFile.read();
}
bool rowParserFinished() {
  return ((configFile.available()>0)?false:true);
}

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
  bridge.set_scale(calibrationValue); 
  
  pinMode(buttonPin, INPUT_PULLUP);
  
  if(!SD.begin(SDPin)){
    Serial.println("SD initialization failed");
    lcd.clear();
    lcd.print("SD initialization FAILED!");
    while(1);
  }
  Serial.println("SD initialized");
  lcd.print("SD initialized!");

  configFile = SD.open("/config.csv");
  if(!configFile){
    lcd.clear();
    lcd.print("Config Missing!");
    Serial.println("Config Missing!");
    while(1);
  }
  int i = 0;

  float *rez = (float*)cp[0];
  float *comps = (float*)cp[1];
  
  while(cp.parseRow()){
    if(i < 4){
      res[i] = rez[0];
      comp[i] = comps[0];
      Serial.print(res[i]);
      Serial.print(" ");
      Serial.println(comp[i]);
      i++;
    } 
  }
  for(int j = 0; j < 4; j++){
    if(res[j] == 0)
      {
        Serial.println(res[j]);
        lcd.clear();
        lcd.print("Config Error");
        lcd.print(" Wrong Res");
        while(1);
      }
  }
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
  addCsvLine(deformationRecording,String(currentDeformation), String(int(millis()-startTime)));
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
  file.print(readTime);
  file.print(",");
  file.println(deformation);
  }
