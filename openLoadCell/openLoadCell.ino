#include <LiquidCrystal_I2C.h>
#include <HX711.h>

LiquidCrystal_I2C lcd(0x27,20,4);

void setup() {
  Serial.begin(11500);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(3,0);
  lcd.print("Waiting for start command");
  
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
