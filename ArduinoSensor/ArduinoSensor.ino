  
#include <Arduino_APDS9960.h>
#include <Wire.h>
#include "BlueDot_BME280.h"
#include <LiquidCrystal_I2C.h>
#include "SoftwareSerial.h"

#define WIFI_RX_PIN 6
#define WIFI_TX_PIN 7

#define SERIAL_SPEED 9600

String host = "postman-echo.com";
String url = "post";

String ssid = "****";
String password =  "****";

BlueDot_BME280 bme; //BME280 Sensor
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

SoftwareSerial esp(WIFI_RX_PIN, WIFI_TX_PIN);

int displayOnFor = 10000;
int displayTimer = 0;
int displayToShow = 0;

int sleepFor = 10; // Sleep 10ms between loops
int sleepUpdateIterations = 500; //Update the sensors every 500 iterations

float temp;
float humidity;
float pressure;

int buttonPin = 8;

void setup() {
  
  Serial.begin(SERIAL_SPEED);
  Serial.println("Starting...");

  esp.begin(SERIAL_SPEED);
  esp.setTimeout(10000);

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
    Serial.println("Updating Temps");
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

void join() {

  Serial.println("Reseting WiFi");
  esp.println("AT+RST");
  if(esp.find("ready")) {
    Serial.println("Module Reset OK");
  }
  //Let the WiFi come up if it's configured
  Serial.print("Waiting for WiFi to start-up");
  for(int i=0; i<20; i++) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");


  Serial.println("Setting Station Mode");
  esp.println("AT+CWMODE=1");
  if(esp.find("OK")) {
    Serial.println("Station Mode Set to 1");
  }
  
  Serial.println("Joining WiFi");
  String join = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";
  esp.println(join);
  if(esp.find("WIFI GOT IP")) {
    Serial.println("Joined OK");
  }
}

void sendData() {
  esp.println("Seting connection multiplex off");
  esp.println("AT+CIPMUX=0");
  if(esp.find("OK")) {
    Serial.println("Mux Set to 0");
  }

  Serial.println("Connecting to host " + host);
  esp.println("AT+CIPSTART=\"TCP\",\""+ host + "\",80");
  if(esp.find("OK")) {
    Serial.println("Connected to " + host);
  }

  String body = "POST /POST?temperature="+ (String)temp +"&pressure=" + (String)pressure + "&humidity=" + humidity + " HTTP/1.1\r\nHost: " + host;
  Serial.println(body);
  int bodyLength = body.length() + 4;
  
  esp.println("AT+CIPSEND=" + (String)bodyLength);
  if(esp.find("OK")) {
    Serial.println("OK to send");
  }
  delay(1000);
  esp.println(body);
  esp.println("");
  if(esp.find("SEND OK")) {
    Serial.println("Sent request OK");
  }
  esp.find("+IPD,");

  int c = 0;
  Serial.print("Got '");
  while(true) {
    c = esp.read();
    if((char)c == ':') {
      break;
    }
    if(c != -1) {
      Serial.print((char)c);
    }
  }
  Serial.println("' bytes");

  for(int i=0; i<20000; i++) {
    c = esp.read();
    if(c != -1) {
      Serial.print((char)c);
    }
  }
  Serial.println("");
  
  Serial.println("Closing connection");
  esp.println("AT+CIPCLOSE");
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
    join();
    sendData(); 
  }


  
 
  delay(10);
}
