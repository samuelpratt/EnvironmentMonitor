  
#include <Arduino_APDS9960.h>
#include <Wire.h>
#include "BlueDot_BME280.h"
#include <LiquidCrystal_I2C.h>


BlueDot_BME280 bme; //BME280 Sensor
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

int displayOnFor = 10000;
int displayTimer = 0;
int displayToShow = 0;

int sleepFor = 10;
int sleepUpdateIterations = 500;

float temp;
float humidity;
float pressure;

int buttonPin = 7;

void setup() {
  
  Serial.begin(9600);
  Serial.println("Starting...");

  lcd.init(); 
  lcd.init();


  lcd.backlight();
  lcd.setCursor(1,0);
  lcd.print("Starting...");
  lcd.noBacklight();
  delay(1000);

  pinMode(buttonPin, INPUT);

  Serial.println("Init temperature sensor");
  bme.parameter.communication = 0;
  bme.parameter.I2CAddress = 0x77;
  bme.parameter.sensorMode = 0b11; 
  bme.parameter.IIRfilter = 0b100;
  bme.parameter.humidOversampling = 0b101;
  bme.parameter.tempOversampling = 0b101;
  bme.parameter.pressOversampling = 0b101; 
  bme.parameter.pressureSeaLevel = 1013.25;
  bme.parameter.tempOutsideCelsius = 15;
  bme.parameter.tempOutsideFahrenheit = 59;
  if (bme.init() != 0x60)
  {
    Serial.println(F("BME280 Sensor not found!"));
  }

  Serial.println("Finished init");
  

}

void updateTemps() {
  Serial.println("Updating Temps");
  humidity  = bme.readHumidity();
  temp      = bme.readTempC();
  pressure  = bme.readPressure();
}

void showTemp(bool backlight) {
  updateDisplay("Temperature", (String)temp, "C", backlight);
}

void showPressure(bool backlight) {
  updateDisplay("Pressure", (String)pressure, "MPa", backlight);
}

void showHumidity(bool backlight) {
  updateDisplay("Humidity", (String)humidity, "%", backlight);
}

void checkButton() {
  
  if(digitalRead(buttonPin) == LOW)
  {
    Serial.println("Button pushed");
    displayToShow++;
    displayTimer = displayOnFor;
    int ds = displayToShow % 3;
    switch(ds) {
      case 0:
        showPressure(true);
        break;
      case 1:
        showHumidity(true);
        break;
      default:
        showTemp(true);
        break;
    }
        
  }
}

void updateDisplay(String label, String value, String unit, bool backlight) {
  lcd.init();
  lcd.clear();
  
  lcd.setCursor(0,0);
  lcd.print(label + ":");
  
  lcd.setCursor(0,1);
  lcd.print(value + " " + unit);

  if(backlight) {
    lcd.backlight();
  }
  else {
    lcd.noBacklight();
  } 
}

void upDateDisplay() {
  if(displayTimer > 0)
  {
    displayTimer -= sleepFor * sleepUpdateIterations;
  }
  else
  {
    showTemp(false);
  }
}

int loopCount = 0;
void loop() {
  loopCount++;
  checkButton();

  if(loopCount % 500 == 0)
  {
    updateTemps();
    upDateDisplay(); 
  }


  
 
  delay(10);
}
