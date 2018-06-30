#include "Particle.h"

// Make sure you include MB85RC256V-FRAM-RK.h before DeviceKeyHelperRK.h, otherwise you won't have support for MB85RC256V
#include "MB85RC256V-FRAM-RK.h"


#include "DeviceKeyHelperRK.h"

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

// Store the device keys starting at address 1000 in the FRAM
DeviceKeyHelperFRAM deviceKeyHelper(fram, 1000);


void setup() {
	Serial.begin();

	fram.begin();
	// fram.erase();

	// Start monitoring for connection failures
	deviceKeyHelper.startMonitor();

	// You either need to use SYSTEM_THREAD(ENABLED) or SYSTEM_MODE(SEMI_AUTOMATIC) because
	// in thread disabled AUTOMATIC mode, setup() isn't called until cloud connected and the
	// code to monitor the connection would never be started via startMonitor().
	Particle.connect();
}

void loop() {
}
