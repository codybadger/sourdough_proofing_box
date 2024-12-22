#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_SSD1306.h>
#include <splash.h>
#include <Adafruit_GFX.h>

#define DHTPIN 5
#define DHTTYPE DHT21

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_I2C_ADDRESS 0x3C
#define OLED_RESET_PIN -1

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 screen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET_PIN);
int TEMP_SETPOINT = 78;
int TEMP_TOLERANCE = 1;
int BUTTON_UP_PIN = 2;
int BUTTON_DOWN_PIN = 3;
int HEATER_PIN = 4;
unsigned long startMillis = millis();
unsigned long currentMillis;
unsigned long oldMillis = millis();
char time[9];
const unsigned long tempRefreshMillis = 1000;
const unsigned long debounceDelay = 100;
unsigned long lastDebounceTime = 0;
int buttonUpState;
int buttonDownState;
int lastButtonUpState = HIGH;
int lastButtonDownState = HIGH;
bool newButtonUpPush;
bool newButtonDownPush;
int HEATER_STATUS; // 1 belowON, 2 aboveOFF, 3 betweenOFF, 4 betweenON

void setup() {
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(HEATER_PIN, OUTPUT);
  dht.begin();
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("Time\tSetpoint\tT\tH\tHEATER STATE");
  screen.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDRESS);
  bool newButtonUpPush = true;
  bool newButtonDownPush = true;
}

void loop() {
  currentMillis = millis();     // get the current "time" (milliseconds since program started)
  int seconds = currentMillis/1000;
  int minutes = currentMillis/60000;
  int hours = currentMillis/3600000;
  seconds %= 60;
  minutes %= 60;
  hours %= 24;
  sprintf(time,"%02d:%02d:%02d",hours,minutes,seconds);
  
  // read button inputs to change temperature setpoint
  int buttonUpReading = digitalRead(2);
  int buttonDownReading = digitalRead(3);
  
  if (buttonUpReading != lastButtonUpState || buttonDownReading != lastButtonDownState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonUpReading != buttonUpState) {
      buttonUpState = buttonUpReading;
      if (buttonUpState == HIGH) {
        TEMP_SETPOINT++;
      }
    }
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonDownReading != buttonDownState) {
      buttonDownState = buttonDownReading;
      if (buttonDownState == HIGH) {
        TEMP_SETPOINT--;
      }
    }
  }

  lastButtonUpState = buttonUpReading;
  lastButtonDownState = buttonDownReading;
  
  // read temperature and humidity
  if (currentMillis - oldMillis >= tempRefreshMillis)
  {
    oldMillis = millis();
    float t = dht.readTemperature(true);
    float h = dht.readHumidity();

    // Temperature control
    if (t >= TEMP_SETPOINT + TEMP_TOLERANCE) {
      digitalWrite(HEATER_PIN, LOW);
      HEATER_STATUS = 2;
    }
    else if (t <= TEMP_SETPOINT - TEMP_TOLERANCE) {
      digitalWrite(HEATER_PIN, HIGH);
      HEATER_STATUS = 1;
    }
    else if (HEATER_STATUS == "above_OFF" && t < TEMP_SETPOINT + TEMP_TOLERANCE) {
      HEATER_STATUS = 3;
    }
    else if (HEATER_STATUS == "below_ON" && t > TEMP_SETPOINT - TEMP_TOLERANCE) {
      digitalWrite(HEATER_PIN, HIGH);
      HEATER_STATUS = 4;
    }

    // Display Data
    Serial.print(time);
    Serial.print(",\t");
    Serial.print(TEMP_SETPOINT);
    Serial.print(",\t");
    Serial.print(t, 1);
    Serial.print(",\t");
    Serial.print(h, 1);
    Serial.print(",\t");
    Serial.println(HEATER_STATUS);
    
    screen.clearDisplay();
    screen.setTextSize(2);
    screen.setTextColor(WHITE);
    screen.setCursor(0, 0);
    screen.print("Temp:");
    screen.setCursor(74, 0);
    screen.print("Hum:");
    
    screen.setTextSize(1);
    screen.setCursor(0,56);
    screen.print("T:");
    screen.setCursor(16, 56);
    screen.print(TEMP_SETPOINT);
    screen.setCursor(34, 56);
    screen.print("F");
    
    screen.setTextSize(1);
    screen.setCursor(56,56);
    screen.print("t:");
    screen.setCursor(72, 56);
    screen.print(time);
/*    screen.print(hours);
    screen.setCursor(80, 56);
    screen.print(":");
    screen.setCursor(88, 56);
    screen.print(minutes);
    screen.setCursor(104, 56);
    screen.print(":");
    screen.setCursor(112, 56);
    screen.print(seconds);
*/
    if(isnan(h) || isnan(t)){
      screen.setCursor(0, 32);
      screen.print("Failed to read from DHT sensor");
      screen.display();
      return;
    }

    screen.setCursor(0, 24);
    screen.setTextSize(3);
    screen.print(t, 0);
    screen.print("F");

    screen.setCursor(74, 24);
    screen.print(h, 0);
    screen.print("%");

    screen.display();
  }


}
