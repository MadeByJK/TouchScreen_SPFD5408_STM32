#include <Adafruit_GFX.h>
#include <Adafruit_SPFD5408_8bit_STM32.h>
#include <TouchScreen_SPFD5408_STM32.h>
#include "Images\Buttons.h"
#include "Images\Symbols.h"

#include <OneWireSTM.h>
#include <DallasTemperature.h>

#define BACKGROUND   0x0000
#define INACTIVE   0x6666
#define FOREGROUND   0xFFFF

#define YP PA1  // must be an analog pin, use "An" notation! WR
#define XM PA2  // must be an analog pin, use "An" notation! RS
#define YM PB15   // can be a digital pin d7
#define XP PB14   // can be a digital pin d6

#define TEMPERATURE_PRECISION 9
#define THERMOMETERS_GROUND_COUNT 1
#define THERMOMETERS_AIR_COUNT 1
#define THERMOMETERS_GROUND_BUS PB0
#define THERMOMETERS_AIR_BUS PB1

#define USE_THERMOMETERS (THERMOMETERS_AIR_COUNT > 0 || THERMOMETERS_GROUND_COUNT > 0)

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
Adafruit_SPFD5408_8bit_STM32 tft;
uint8_t selected = BTN_HEATING;
float groundTemperatures[THERMOMETERS_GROUND_COUNT];
float airTemperatures[THERMOMETERS_AIR_COUNT];

#if THERMOMETERS_GROUND_COUNT

OneWire groundOneWire(THERMOMETERS_GROUND_BUS);
DallasTemperature groundSensors(&groundOneWire);

#endif

#if THERMOMETERS_AIR_COUNT

OneWire airOneWire(THERMOMETERS_AIR_BUS);
DallasTemperature airSensors(&airOneWire);

#endif

void setup()
{
  Serial.begin(115200);
  delay(1000);
  initScreen();
  ts.calibrate(preCalibration, postCalibration);

#if USE_THERMOMETERS
  initThermometers();
#endif

  drawScreen();
  drawTab();
}

void preCalibration(const int16_t x, const int16_t y)
{
  tft.fillCircle(x, y, 5, FOREGROUND);
}

void postCalibration(const int16_t x, const int16_t y)
{
  for (int i = 0; i < 5; i++) {
    tft.setCursor(50, 50);
    tft.println(5 - i);
    delay(1000);
    tft.fillScreen(BACKGROUND);
  }
}

void initScreen()
{
  tft.reset();
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(BACKGROUND);
  tft.setTextColor(FOREGROUND);
  tft.setTextSize(5);
}

#if USE_THERMOMETERS
void initThermometers()
{
#if THERMOMETERS_GROUND_COUNT
  groundSensors.begin();
#endif

#if THERMOMETERS_AIR_COUNT
  airSensors.begin();
#endif
}
#endif

void loop()
{

}

void drawScreenLines()
{
  tft.drawFastHLine(0, 32, 320, FOREGROUND);
  tft.drawFastVLine(240, 32, 208, FOREGROUND);
}

void drawScreen()
{
  tft.fillScreen(BACKGROUND);
  drawScreenLines();

  for (int i = 0; i < 4; i++)
  {
    tft.drawBitmap((32 * i) + 4, 4, symbols[i], 24, 24, INACTIVE);
  }

  for (int i = 0; i < 5; i++)
  {
    tft.drawRect(240, 32 + 40 * i, 80, 40, FOREGROUND);
    tft.drawBitmap(264, 36 + (40 * i), buttons[i], 32, 32, FOREGROUND);
  }
}

void drawTab()
{
  tft.fillRect(240, 32 + 40 * selected, 80, 40, FOREGROUND);
  tft.drawBitmap(264, 36 + (40 * selected), buttons[selected], 32, 32, BACKGROUND);

  switch (selected)
  {
    case BTN_HEATING:
      drawHeating();
      break;

    case BTN_ROLLERS:
      break;
    case BTN_SECURITY:
      break;
    case BTN_SETTINGS:
      break;
    case BTN_WARNING:
      break;
  }
}

void drawHeating()
{

}

float readTemperature(DallasTemperature sensor, uint8_t count)
{
  float result = 0.0;
  sensors.requestTemperatures();

  for (int i = 0; i < count; i++)
  {
    result +=sensor.getTempCByIndex(i);
  }

  return result/count;
}

