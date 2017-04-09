// Touch screen library with X Y and Z (pressure) readings as well
// as oversampling to avoid 'bouncing'
// (c) ladyada / adafruit
// Code under MIT License

// Optimised for SPFD5408 on SMT32 by MadeByJK

#include "Arduino.h"
#include "pins_arduino.h"
#include "wiring_private.h"
#include <avr/pgmspace.h>
#include <WProgram.h>

#include "TouchScreen_SPFD5408_STM32.h"

//******************************* Others *******************************\\

#if (NUMSAMPLES > 2)
static void insert_sort(int array[], uint8_t size) {
	uint8_t j;
	int save;

	for (int i = 1; i < size; i++) {
		save = array[i];
		for (j = i; j >= 1 && save < array[j - 1]; j--)
			array[j] = array[j - 1];
		array[j] = save;
	}
}
#endif

//******************************* TSPoint *******************************\\

TSPoint::TSPoint() {
	x = y = 0;
}

TSPoint::TSPoint(int16_t x0, int16_t y0, int16_t z0) {
	x = x0;
	y = y0;
	z = z0;
	
	isValid = z0 > TS_MIN_PRESSURE && z0 < TS_MAX_PRESSURE && x0 > 0 && y0 > 0;
}

bool TSPoint::operator==(TSPoint p1) {
	return  ((p1.x == x) && (p1.y == y) && (p1.z == z));
}

bool TSPoint::operator!=(TSPoint p1) {
	return  ((p1.x != x) || (p1.y != y) || (p1.z != z));
}

//******************************* TouchScreen *******************************\\

TouchScreen::TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym) {
	_yp = yp;
	_xm = xm;
	_ym = ym;
	_xp = xp;
	_rxplate = 0;
}

TouchScreen::TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym, uint16_t rxplate) {
	_yp = yp;
	_xm = xm;
	_ym = ym;
	_xp = xp;
	_rxplate = rxplate;
}

TSPoint TouchScreen::getPoint() {
	int x, y, z;
	int samples[NUMSAMPLES];
	uint8_t i, valid;

	valid = 1;
	pinMode(_yp, INPUT);
	pinMode(_ym, INPUT);

	digitalWrite(_yp, LOW);
	digitalWrite(_ym, LOW);

	pinMode(_xp, OUTPUT);
	pinMode(_xm, OUTPUT);

	digitalWrite(_xp, HIGH);
	digitalWrite(_xm, LOW);

	for (i = 0; i < NUMSAMPLES; i++) {
		samples[i] = analogRead(_yp);	
	}
#if NUMSAMPLES > 2
	insert_sort(samples, NUMSAMPLES);
#endif

#if NUMSAMPLES == 2
	if (samples[0] != samples[1]) { valid = 0; }
#endif

	x = (4095 - samples[NUMSAMPLES / 2]);

	pinMode(_xp, INPUT);
	pinMode(_xm, INPUT);
	digitalWrite(_xp, LOW);
	pinMode(_yp, OUTPUT);
	digitalWrite(_yp, HIGH);
	pinMode(_ym, OUTPUT);

	for (i = 0; i < NUMSAMPLES; i++) {
		samples[i] = analogRead(_xm);
	}

#if NUMSAMPLES > 2
	insert_sort(samples, NUMSAMPLES);
#endif
#if NUMSAMPLES == 2
	if (samples[0] != samples[1]) { valid = 0; }
#endif

	y = (4095 - samples[NUMSAMPLES / 2]);

	pinMode(_xp, OUTPUT);

	// Set X+ to ground
	digitalWrite(_xp, LOW);

	// Set Y- to VCC
	digitalWrite(_ym, HIGH);

	// Hi-Z X- and Y+
	digitalWrite(_yp, LOW);

	//digitalWrite(_yp, LOW);
	pinMode(_yp, INPUT);

	int z1 = analogRead(_xm);
	int z2 = analogRead(_yp);

	if (_rxplate != 0) {
		// now read the x
		float rtouch;
		rtouch = z2;
		rtouch /= z1;
		rtouch -= 1;
		rtouch *= x;
		rtouch *= _rxplate;
		rtouch /= 4095;

		z = rtouch;
	}
	else {
		z = (4095 - (z2 - z1));
	}
		
	pinMode(_yp, OUTPUT);
	pinMode(_ym, OUTPUT);
	pinMode(_xp, OUTPUT);
	pinMode(_xm, OUTPUT);

	if (!valid) {
		return TSPoint(0, 0, 0);
	}
	
	#ifdef toggleXY
	return TSPoint(calculetePoint(TS_HEIGHT, calibration.y0, calibration.y1, y), calculetePoint(TS_WIDTH, calibration.x0, calibration.x1, x), z);
	#else
	return TSPoint(calculetePoint(TS_WIDTH, calibration.x0, calibration.x1, x), calculetePoint(TS_HEIGHT, calibration.y0, calibration.y1, y), z);
	#endif
}

bool TouchScreen::calibrate(CalibrationStateCallback prePointCalibration, CalibrationStateCallback postPointCalibration) {
	for(int i = 0; i < 2; i++, getPoint());	
	calibratePoint(prePointCalibration, postPointCalibration, 0, 0, calibration.x0, calibration.y0);
	calibratePoint(prePointCalibration, postPointCalibration, TS_WIDTH - 1, TS_HEIGHT - 1, calibration.x1, calibration.y1);
	useCalibration = true;	

	return true;
}

int16_t TouchScreen::calculetePoint(int16_t s, int16_t p0, int16_t p1, int16_t p) {
	if (useCalibration) {
		p = s - ((p1 - p) / (float)(p1 - p0))*s;	
	}

	return p;
}

void TouchScreen::calibratePoint(CalibrationStateCallback prePointCalibration, CalibrationStateCallback postPointCalibration, int16_t xp, int16_t yp, int16_t &x, int16_t &y) {
	TSPoint point;

	prePointCalibration(xp, yp);

	do
	{
		delay(20);
		point = getPoint();		
	} while (!point.isValid);

	x = point.x;
	y = point.y;	
	
	postPointCalibration(xp, yp);
}

//******************************* TSCalibration *******************************\\

TSCalibration::TSCalibration() {
	x0 = y0 = x1 = y1 = 0;
}

TSCalibration::TSCalibration(int16_t _x0, int16_t _y0, int16_t _x1, int16_t _y1) {
	x0 = _x0;
	y0 = _y0;
	x1 = _x1;
	y1 = _y1;
}