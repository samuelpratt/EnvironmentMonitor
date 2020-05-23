  
#include <Wire.h>
#include "BlueDot_BME280.h"
#include "SoftwareSerial.h"
#include <U8x8lib.h>

#define WIFI_RX_PIN 6
#define WIFI_TX_PIN 7

#define SERIAL_SPEED 9600

// Your board may be on 0x77 mine appears to be a knock off sensor. If in doubt use an I2C Scanner
// e.g. https://github.com/RobTillaart/Arduino/tree/master/sketches/MultiSpeedI2CScanner
#define BME280_ID  0x76

String host = "postman-echo.com";
String url = "post";

String ssid = "";
String password =  "";

BlueDot_BME280 bme; //BME280 Sensor
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE); // SD1306 Screen
SoftwareSerial esp(WIFI_RX_PIN, WIFI_TX_PIN); //WiFi module

float temp;
float humidity;
float pressure;
String statusString = "000";

void setup() {
  
  Serial.begin(SERIAL_SPEED);
  Serial.println("Starting...");

  esp.begin(SERIAL_SPEED);
  esp.setTimeout(10000);

  u8x8.begin();
  u8x8.setFont(u8x8_font_px437wyse700a_2x2_r);
  u8x8.drawString(0,0,"Starting");
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
    Serial.println(F("BME280 not found"));
  }

  Serial.println("Finished init");
  

}

void updateTemps() {
  statusString = "Temp";
  updateStatusDisplay();
  humidity  = bme.readHumidity();
  temp      = bme.readTempC();
  pressure  = bme.readPressure();
  updateTempDisplay();
}

void join() {

  statusString = "RST";
  updateStatusDisplay();
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
  updateStatusDisplay();
  Serial.println("Setting Station Mode");
  esp.println("AT+CWMODE=1");
  if(esp.find("OK")) {
    Serial.println("Station Mode Set to 1");
  }

  statusString = "JOIN";
  updateStatusDisplay();
  Serial.println("Joining WiFi");
  String join = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";
  esp.println(join);
  if(esp.find("WIFI GOT IP")) {
    Serial.println("Joined OK");
  }
}

void sendData() {
  statusString = "STRT";
  updateStatusDisplay();
  esp.println("Seting conn multiplex off");
  esp.println("AT+CIPMUX=0");
  if(esp.find("OK")) {
    Serial.println("Mux Set to 0");
  }

  statusString = "CONN";
  updateStatusDisplay();
  Serial.println("Connecting to " + host);
  esp.println("AT+CIPSTART=\"TCP\",\""+ host + "\",80");
  if(esp.find("OK")) {
    Serial.println("Connected to " + host);
  }

  statusString = "POST";
  updateStatusDisplay();
  String body = "POST /POST?temperature="+ (String)temp +"&pressure=" + (String)pressure + "&humidity=" + humidity + " HTTP/1.1\r\nHost: " + host;
  int bodyLength = body.length() + 4;

  
  esp.println("AT+CIPSEND=" + (String)bodyLength);
  if(esp.find("OK")) {
    Serial.println("OK to snd");
  }
  delay(1000);
  esp.println(body);
  esp.println("");
  if(esp.find("SEND OK")) {
    Serial.println("Sent req OK");
  }



  //Need some error handling here. If no response then this will probably loop forever.
  delay(1000);
  Serial.println("Waiting");

  int c = -1;
  while(true) {
    c = esp.read();
    //Looking for the ',' at the end of "+IDP,"
    if(c == ',') {
      break;
    }
  }
  
  //esp.find("+IPD,");
  Serial.print("Got '");

  //int c = 0;
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
  updateStatusDisplay();
  
  Serial.println("Closing connection");
  esp.println("AT+CIPCLOSE");
}

void updateTempDisplay() {
  static char buffer[20];
  u8x8.setFont(u8x8_font_px437wyse700a_2x2_r);
  
  dtostrf(temp,4, 1, buffer);
  ("T:" + (String)buffer + "C").toCharArray(buffer, 20);
  u8x8.clearLine(0);
  u8x8.clearLine(1);
  u8x8.drawString(0,0, buffer);

  dtostrf(humidity,4, 1, buffer);
  ("H:" + (String)buffer + "%").toCharArray(buffer, 20);
  u8x8.clearLine(2);
  u8x8.clearLine(3);
  u8x8.drawString(0,2, buffer);

  u8x8.clearLine(4);
  u8x8.clearLine(5);
  dtostrf(pressure,4, 1, buffer);
  ("P:" + (String)buffer).toCharArray(buffer, 20);
  u8x8.drawString(0,4, buffer);
}

void updateStatusDisplay() {  
  static char buffer[20];
  u8x8.setFont(u8x8_font_px437wyse700a_2x2_r);
  statusString.toCharArray(buffer, 20);
  u8x8.clearLine(6);
  u8x8.clearLine(7);
  u8x8.drawString(0,6, buffer);
}

void loop() {
  updateTemps();
  join();
  sendData(); 

  Serial.println("Speeping");
  statusString = "";
  updateStatusDisplay();
  delay(20000);
}
