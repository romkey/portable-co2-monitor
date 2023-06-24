#include <Arduino.h>
#include <Wire.h>
#include <SPIFFS.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SparkFun_SCD4x_Arduino_Library.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define SCREEN_ADDRESS 0x3D

#define FILENAME "co2.csv"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT);
SCD4x co2;

typedef struct reading {
  uint16_t co2;
  uint16_t temperature;
  uint16_t humidity;
} reading_t;

#define MAX_SIZE 4000
#define MAX_READINGS MAX_SIZE/sizeof(reading_t)

RTC_DATA_ATTR uint16_t current_reading = 0;
RTC_DATA_ATTR reading_t readings[MAX_READINGS];

void show_current();
void show_graph();
void sync_to_flash();

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("hello world");

  Serial.print("can store ");
  Serial.print(MAX_READINGS);
  Serial.println(" readings before syncing to flash");

  Wire.begin(8, 9);

  //  co2.enableDebugging();

  if(!co2.begin()) {
    Serial.println(F("Sensor not detected. Please check wiring. Freezing..."));
    while (1)
      delay(1);
  }

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 init failed");
  }

  display.clearDisplay();
  display.display();
}

void loop()  {
  if(co2.readMeasurement()) {
    Serial.print("idx:");
    Serial.println(current_reading);

    Serial.print("CO2(ppm):");
    Serial.print(co2.getCO2());

    Serial.print("\tTemperature(C):");
    Serial.print(co2.getTemperature(), 1);

    Serial.print("\tHumidity(%RH):");
    Serial.print(co2.getHumidity(), 1);

    Serial.println();

    readings[current_reading].co2 = co2.getCO2();
    readings[current_reading].temperature = co2.getTemperature();
    readings[current_reading].humidity = co2.getHumidity();

    show_current();
    show_graph();

    current_reading++;

    if(current_reading == MAX_READINGS) {
      sync_to_flash();
      current_reading = 0;
    }

    esp_sleep_enable_timer_wakeup(50*1000*1000);
    esp_deep_sleep_start();
  } else
    Serial.print(".");

  delay(6000);
}


void show_current() {
  display.clearDisplay();

  display.setCursor(0, 0);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.print("CO2");

  display.setTextSize(4);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(10, 30);     // Start at top-left corner

  display.print(readings[current_reading].co2);

  display.display();

  delay(3000);
}

void show_graph() {
  uint16_t high = 0;
  uint16_t low = 0xFFFF;

#ifdef TEST
  Serial.println("seeding values");

  for(uint16_t i = 0; i < MAX_READINGS; i++) {
    readings[i].co2 = 400 + i;
    readings[i].temperature = 100 + random(180);
    readings[i].humidity = 200 + random(800);
  }

  current_reading = MAX_READINGS - 40;
#endif


  uint16_t start = (current_reading - SCREEN_WIDTH) % MAX_READINGS;

  for(uint16_t i = 0; i < SCREEN_WIDTH; i++) {
    high = max(high, readings[(start + i) % MAX_READINGS].co2);
    low = min(low, readings[(start + i) % MAX_READINGS].co2);
  }

#ifdef TEST
  Serial.print("high ");
  Serial.println(high);
  Serial.print("low ");
  Serial.println(low);
#endif

  display.clearDisplay();

  for(uint16_t i = 0; i < SCREEN_WIDTH; i++) {
#ifdef TEST
    Serial.print(i);
    Serial.print(", ");
    Serial.println(map(readings[start + i].co2, low, high, 0, 63));
#endif

    display.drawPixel(i, 63 - map(readings[(start + i) % MAX_READINGS].co2, low, high, 0, 63), SSD1306_WHITE);
  }

  display.display();
  delay(3000);
}

void sync_to_flash() {
#if 0
  for(uint16_t i = 0; i < MAX_READINGS; i++) {
    readings[i].co2 = 400 + i;
    readings[i].temperature = 100 + random(180);
    readings[i].humidity = 200 + random(800);
  }
#endif

  if(!SPIFFS.begin(true)) {
    Serial.println("SPIFFS failed");
    display.clearDisplay();

    display.setTextSize(4);      // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 10);     // Start at top-left corner
    display.print("SPIFFS :(");

    delay(30*1000);
    return;
  }

  File f = SPIFFS.open(FILENAME, FILE_APPEND);
  for(uint16_t i = 0; i < MAX_READINGS; i++)
    f.printf("%d,%d, %d\n", readings[i].co2, readings[i].temperature, readings[i].humidity);

  f.close();
  Serial.println("SPIFFS good");
}
