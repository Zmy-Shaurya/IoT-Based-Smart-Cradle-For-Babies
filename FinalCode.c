#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// -------- PINS --------
#define DHTPIN 4
#define DHTTYPE DHT11
#define SOUND_PIN 34
#define RELAY 25
#define BUZZER 27
#define IN3 18
#define IN4 19

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

// -------- SETTINGS --------
float tempThreshold = 31.0;
float humidityThreshold = 70.0;
int soundThreshold = 1400;

// -------- VARIABLES --------
int soundCount = 0;
bool crying = false;

unsigned long motorStartTime = 0;
bool motorRunning = false;

void setup() {
  Serial.begin(115200);

  // prevent reset issues
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  dht.begin();

  pinMode(RELAY, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  digitalWrite(RELAY, HIGH); // OFF
  digitalWrite(BUZZER, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  lcd.print("Smart Cradle");
  delay(2000);
  lcd.clear();
}

void loop() {

  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();
  int soundValue = analogRead(SOUND_PIN);

  Serial.print("T:");
  Serial.print(temp);
  Serial.print(" H:");
  Serial.print(hum);
  Serial.print(" S:");
  Serial.println(soundValue);

  // -------- FAN --------
  bool fanOn = (!isnan(temp) && temp > tempThreshold);
  digitalWrite(RELAY, fanOn ? LOW : HIGH);

  // -------- BUZZER --------
  digitalWrite(BUZZER, (hum > humidityThreshold) ? HIGH : LOW);

  // -------- CRY DETECTION --------
  if (soundValue > soundThreshold) {
    soundCount++;
  } else {
    soundCount = 0;
  }

  crying = (soundCount > 1);

  // -------- MOTOR TIMER --------
  if (crying) {
    motorRunning = true;
    motorStartTime = millis();
  }

  bool cryMotorOn = motorRunning && (millis() - motorStartTime < 30000);

  if (cryMotorOn) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  } else {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    motorRunning = false;
  }

  // -------- LCD --------
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temp);
  lcd.print(" H:");
  lcd.print(hum);
  lcd.print("   ");

  lcd.setCursor(0, 1);

  if (fanOn && cryMotorOn) {
    lcd.print("FAN+CRY ACTIVE ");
  }
  else if (cryMotorOn) {
    lcd.print("CRY MOTOR ON   ");
  }
  else if (fanOn) {
    lcd.print("FAN ON         ");
  }
  else {
    lcd.print("MONITORING     ");
  }

  delay(200);
}
