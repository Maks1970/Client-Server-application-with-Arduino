#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 8

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

DeviceAddress insideThermometer;

//int data = DEVICE_DISCONNECTED_C;
int t1820 = DEVICE_DISCONNECTED_C;
int tserver = DEVICE_DISCONNECTED_C;
int comRead = DEVICE_DISCONNECTED_C;

float tempC;
bool running = true;
bool first_run = true;

void setup() {
  Serial.begin(9600);
  sensors.begin();
  sensors.getAddress(insideThermometer, 0);
  sensors.setResolution(insideThermometer, 9);

  pinMode(9, OUTPUT);
  //Pinmode();
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("Work!");

}

char m;
void loop() {
  if (first_run){
    sensors.requestTemperatures(); // Send the command to get temperatures
    sensors.getTempC(insideThermometer);
    first_run = false;
  }

  //-- read T
  sensors.requestTemperatures(); // Send the command to get temperatures
  tempC = sensors.getTempC(insideThermometer);
  if (tempC != DEVICE_DISCONNECTED_C)
  {
    t1820 = tempC;
      if (running){
        lcd.setCursor(0, 0);
        lcd.print("  Temperature: ");
        lcd.setCursor(5, 1);
        lcd.print(t1820);
        Serial.write(t1820);
        delay(1000);
      }
  }


  if (Serial.available()) {
    //get byte from serial
      comRead = Serial.read();
      if (comRead < 128){
        tserver = comRead;
      }
    //if data from DS18B20 and data from serial is correct
    if ( (t1820 != DEVICE_DISCONNECTED_C) && (tserver != DEVICE_DISCONNECTED_C) ) {
      //set up ten pin
      if (t1820 <= tserver) {
        digitalWrite(9, HIGH);
      } else {
        digitalWrite(9, LOW);
      }
      //
      if (comRead == 0xCC){
       running = false; 
       lcd.clear(); 
       lcd.setCursor(5, 1);
       lcd.print("STOP");
       digitalWrite(9, LOW);
      }
      else if (comRead == 0xEE ){
        lcd.clear(); 
        if (t1820 <= tserver) {
          digitalWrite(9, HIGH);
        } else {
          digitalWrite(9, LOW);
        }
        running = true;
      }
    }
  }
}
