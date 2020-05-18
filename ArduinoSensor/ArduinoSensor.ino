  
#include <Arduino_APDS9960.h>
#include <Wire.h>
#include "BlueDot_BME280.h"
#include <LiquidCrystal_I2C.h>
#include "SoftwareSerial.h"

#define WIFI_RX_PIN 6
#define WIFI_TX_PIN 7

#define SERIAL_SPEED 9600

// Your board may be on 0x77 mine appears to be a knock off sensor. If in doubt use an I2C Scanner
// e.g. https://github.com/RobTillaart/Arduino/tree/master/sketches/MultiSpeedI2CScanner
#define BME280_ID  0x76
#define LCD_ID  0x27

String host = "postman-echo.com";
String url = "post";

String ssid = "";
String password =  "";

BlueDot_BME280 bme; //BME280 Sensor
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

SoftwareSerial esp(WIFI_RX_PIN, WIFI_TX_PIN);

float temp;
float humidity;
float pressure;
String statusString = "000";

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

  Serial.println("Init temperature sensor");
  bme.parameter.communication = 0;
  bme.parameter.I2CAddress = BME280_ID;
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
  statusString = "Temp";
  updateDisplay();
  humidity  = bme.readHumidity();
  temp      = bme.readTempC();
  pressure  = bme.readPressure();
  updateDisplay();
}

void join() {

  statusString = "RST";
  updateDisplay();
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

  statusString = "STN";
  updateDisplay();
  Serial.println("Setting Station Mode");
  esp.println("AT+CWMODE=1");
  if(esp.find("OK")) {
    Serial.println("Station Mode Set to 1");
  }

  statusString = "JOIN";
  updateDisplay();
  Serial.println("Joining WiFi");
  String join = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";
  esp.println(join);
  if(esp.find("WIFI GOT IP")) {
    Serial.println("Joined OK");
  }
}

void sendData() {
  statusString = "STRT";
  updateDisplay();
  esp.println("Seting connection multiplex off");
  esp.println("AT+CIPMUX=0");
  if(esp.find("OK")) {
    Serial.println("Mux Set to 0");
  }

  statusString = "CONN";
  updateDisplay();
  Serial.println("Connecting to host " + host);
  esp.println("AT+CIPSTART=\"TCP\",\""+ host + "\",80");
  if(esp.find("OK")) {
    Serial.println("Connected to " + host);
  }

  statusString = "POST";
  updateDisplay();
  String body = "POST /POST?temperature="+ (String)temp +"&pressure=" + (String)pressure + "&humidity=" + humidity + " HTTP/1.1\r\nHost: " + host;
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

  //Need some error handling here. If no response then this will probably loop forever.
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

  int charCount = 0;
  static char response[15];
  while(charCount < 15)
  {
    c = esp.read();
    if(c != -1) {
      response[charCount] = c;
      charCount++;
    }
  }
  String respCode = (String)response[9] + (String)response[10] + (String)response[11];
  Serial.println("Response Code is '" + respCode + "'");
  if(respCode == "200")
  {
    statusString = "OK";
  }
  else
  {
    statusString = respCode;
  }
  updateDisplay();
  
  Serial.println("Closing connection");
  esp.println("AT+CIPCLOSE");
}

void updateDisplay() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);

  static char tempStr[15];
  dtostrf(temp,4, 1, tempStr);
  static char humidityStr[15];
  dtostrf(humidity,4, 1, humidityStr);
  static char pressureStr[15];
  dtostrf(pressure,4, 1, pressureStr);
  
  lcd.print("T:" + (String)tempStr + "C, H:"+ (String)humidityStr + "%");
  lcd.setCursor(0,1);
  lcd.print("P:" + (String)pressureStr + "MPa " + statusString);
}
void loop() {
  updateTemps();
  join();
  sendData(); 

  statusString = "Zzzz";
  updateDisplay();
  delay(1000);
}
