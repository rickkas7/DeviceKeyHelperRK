#include "Particle.h"

// Make sure you include MB85RC256V-FRAM-RK.h before DeviceKeyHelperRK.h, otherwise you won't have support for MB85RC256V
#include "MB85RC256V-FRAM-RK.h"


#include "DeviceKeyHelperRK.h"

// Note: You should use SEMI_AUTOMATIC mode so the check for valid keys can be done before connecting.
SYSTEM_MODE(SEMI_AUTOMATIC);

// Pick a debug level from one of these two:
SerialLogHandler logHandler;
// SerialLogHandler logHandler(LOG_LEVEL_TRACE);

// MB85RC256V FRAM on Wire (D0/D1) with default address (A0-A2 not connected, which have pull-downs
// on the Adafruit breakout board)
// - VCC to 3V3 (can also use VIN for a 5V I2C bus)
// - GND to GND
// - WP not connected (connect to VCC to prevent writes to the memory)
// - SCL connect to D1 (SCL) (blue in the picture)
// - SDA connect to D0 (SDA) (green in the picture)
// - A2 not connected. Connect to VCC to change the I2C address.
// - A1 not connected. Connect to VCC to change the I2C address.
// - A0 not connected. Connect to VCC to change the I2C address.
MB85RC256V fram(Wire, 0);

void setup() {
	Serial.begin();

	// Not necessary, though provided here so you can see the serial log messages more easily
	delay(4000);

	fram.begin();
	// fram.erase();

	// Check the device public and private keys against the keys stored in FRAM
	// at offset 1000
	DeviceKeyHelperFRAM deviceKeyHelper(fram, 1000);
	deviceKeyHelper.check();

	Particle.connect();
}

void loop() {
}
