
#include <Adafruit_Sensor.h>
#include <RTClib.h>
#include <Adafruit_MMA8451.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>

#define LOG_INTERVAL  1000 //mills between entries
#define REDLED 7
#define GREENLED 6
#define CHIPSELECT 10
#define SWITCHIN 5

//onboard LEDs
//const int redLED = 7;
//const int greenLED = 6;

//switch for inputs
//const int switchIn = 5;
//const int chipSelect = 10;

//initialize stuff
RTC_DS1307 RTC;
Adafruit_MMA8451 mma = Adafruit_MMA8451();
SoftwareSerial mySerial(3, 2);
Adafruit_GPS gps(&mySerial);

//Data variables
int logMillis = 0;

File newLog() {

  logMillis = millis();

  //code to create a unique filename. starts at 00, and loops incrementing that number until a filename which has not been used has been found
  char filename[] = "LOG00.TXT";
  for (uint8_t i = 0; i < 100; i++) {
    filename[3] = i / 10 + '0';
    filename[4] = i % 10 + '0';

    if (! SD.exists(filename)) {
      break;
    }
  }

  File logfile = SD.open(filename, FILE_WRITE);

  Serial.print(F("Logging to: "));
  Serial.println(filename);

  DateTime now;
  now = RTC.now();

  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  logfile.print(" ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.println(now.second(), DEC);

  logfile.println(F("millis,speed,lat,lon,alt,accx,accy,accz"));

  return logfile;

}

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
  Serial.println();

  pinMode(GREENLED, OUTPUT);
  pinMode(REDLED, OUTPUT);
  pinMode(SWITCHIN, INPUT_PULLUP);
  pinMode(CHIPSELECT, OUTPUT);

  useInterrupt();
  gps.begin(9600);
  gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  gps.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);

  Serial.println(F("Initializing SD card"));

  SD.begin(CHIPSELECT);

  Serial.println(F("card initialized"));

  Wire.begin();
  if (!RTC.begin()) {
    Serial.println(F("RTC failed"));
  }
}

SIGNAL(TIMER0_COMPA_vect) {
  gps.read();
}

void useInterrupt() {
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}


void loop() {
  if (digitalRead(SWITCHIN)) {
    digitalWrite(REDLED, LOW);
    File logfile = newLog();

    while (digitalRead(SWITCHIN)) {
      digitalWrite(GREENLED, HIGH);

      gps.parse(gps.lastNMEA());

      //String data = new String(logMillis + "'" + gps.speed + "'" + gps.latitude + "'" + gps.longitude + "'" + gps.altitude + "'")

      logfile.print(logMillis);
      logfile.print(F(" , "));
      logfile.print(gps.speed);
      logfile.print(F(" , "));
      logfile.print(gps.latitude);
      logfile.print(F(" , "));
      logfile.print(gps.longitude);
      logfile.print(F(" , "));
      logfile.print(gps.altitude);
      logfile.print(F(" , "));

      delay(100);
      digitalWrite(GREENLED, LOW);
      delay(LOG_INTERVAL - 100);
    }

    logfile.close();
    digitalWrite(REDLED, HIGH);
  }
}
