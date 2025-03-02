// The format of a line of the config file SHALL be: <nominal resistance>,<compensation factor>
// First line of config.txt SHALL be the header
// First data line SHALL contain information about the poisson ratio, εs, calibration factor and bridge setup,
// after the resistance and compensation factor, IN THIS ORDER!!!

#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <SPI.h>
#include <SD.h>
#include <CSV_Parser.h>

File deformationRecording, configFile;
LiquidCrystal_I2C lcd(0x27,20,4);
HX711 bridge;

int bridgeSetup = 1;
float baseVoltage = 5.0;
float poisson = 0;
float statDef = 0;
float comp[4];
//float res[4];
const int dataPinBridge = 5;
const int clockPinBridge = 6;
int calibrationValue = 0;

const int SDPin = 10;
const int buttonPin = 4;
Sd2Card card;
long long startTime=0;
CSV_Parser cp("fffdd");

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

  float *comps = (float*)cp[0];
  float *filePoisson = (float*)cp[1];
  float *fileStatDef = (float*)cp[2];
  int *fileCalFact = (int*)cp[3];
  int *configSetup = (int*)cp[4];
  
  while(cp.parseRow()){
    if(i == 0){
      poisson = filePoisson[0];
      statDef = fileStatDef[0];
      calibrationValue = fileCalFact[0];
      bridgeSetup = configSetup[0];
      
      Serial.println((String)poisson+" "+(String)statDef+" "+(String)calibrationValue+" "+(String)bridgeSetup);
    }
    
    if(i < 4){
      comp[i] = comps[0];
      Serial.println(comp[i]);
      i++;
    } 
  }
  for(int j = 0; j < 4; j++){
    if(comp[j] == 0)
      {
        lcd.clear();
        lcd.print("Config Error");
        lcd.print(" ZERO Res or Comp");
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
    deformationRecording= SD.open(nameMaker(), FILE_WRITE);
    addCsvLine(deformationRecording, "Raw Reading", "Read Time", "Voltage Reading", "Deformation Value");
    bridge.power_up();
    startTime = millis();
    bridge.tare();
  }
  
  int rawReading = bridge.read();
  float voltageReading = rawReading * baseVoltage / 16777216;
  float deformationReading = deformationParsing(voltageReading);
  addCsvLine(deformationRecording,String(rawReading), String(int(millis()-startTime)), String(voltageReading), String(deformationReading));
  recordingDisplay((String)rawReading, (String)voltageReading, (String)deformationReading);
  buttonDebounce(digitalRead(buttonPin)); //verify test stop
  delay(10);
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

void addCsvLine(File file, String rawValue, String readTime, String readVoltage, String deformation){
  file.print(readTime);
  file.print(",");
  file.print(rawValue);
  file.print(",");
  file.print(readVoltage);
  file.print(",");
  file.println(deformation);
  }

float deformationParsing(float voltage){
  switch(bridgeSetup){
    case 1:
      return 4 * voltage / comp[0] / baseVoltage - statDef;
    break;
    
    case 2:
      return 4 * voltage / comp[0] / baseVoltage;
    break;

    case 3:
      return 4 * voltage / (1 + poisson) / comp[0] / baseVoltage;
    break;
    
    case 4:
      return 2 * voltage / comp[0] / baseVoltage;
    break;
        
    case 5:
      return 2 * voltage / comp[0] / baseVoltage - statDef;
    break;
        
    case 6:
      return 2 * voltage / (1 + poisson) / comp[0] / baseVoltage; 
    break;
        
    case 7:
      return voltage / comp[0] / baseVoltage;
    break;
        
    case 8:
      return 2 * voltage / (1 - poisson) / comp[0] / baseVoltage;
    break;
  
  }
}

void recordingDisplay(String r, String v, String e){
  lcd.clear();
  lcd.print("r:" + r + " v:" + v + " e:" + e);
}
