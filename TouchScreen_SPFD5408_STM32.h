// Touch screen library with X Y and Z (pressure) readings as well
// as oversampling to avoid 'bouncing'
// (c) ladyada / adafruit
// Code under MIT License

// Optimised for SPFD5408 on SMT32 by MadeByJK

#ifndef _ADAFRUIT_TOUCHSCREEN_STM_H_
#define _ADAFRUIT_TOUCHSCREEN_STM_H_

#include <stdint.h>

typedef void(*CalibrationStateCallback) (const int16_t x, const int16_t y);

#define NUMSAMPLES 5
#define TS_WIDTH 320
#define TS_HEIGHT 240
#define TS_MIN_PRESSURE 100
#define TS_MAX_PRESSURE 375

class TSPoint {
public:
	TSPoint();
	TSPoint(int16_t x, int16_t y, int16_t z);

	bool operator==(TSPoint);
	bool operator!=(TSPoint);

	int16_t x, y, z;
	bool isValid;
};
class TSCalibration {
public:
	TSCalibration();
	TSCalibration(int16_t x0, int16_t y0, int16_t x1, int16_t y1);

	int16_t x0, y0, x1, y1;
};
class TouchScreen {
public:
	TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym);
	TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym, uint16_t rx);

	TSPoint getPoint();
	bool calibrate(CalibrationStateCallback prePointCalibration, CalibrationStateCallback postPointCalibration);

	TSCalibration calibration;
	bool useCalibration;

private:
	uint8_t _yp, _ym, _xm, _xp;
	uint16_t _rxplate;

	int16_t calculetePoint(int16_t s, int16_t p0, int16_t p1, int16_t p);
	void calibratePoint(CalibrationStateCallback prePointCalibration, CalibrationStateCallback postPointCalibration, int16_t xp, int16_t yp, int16_t &x, int16_t &y);
};

#endif
