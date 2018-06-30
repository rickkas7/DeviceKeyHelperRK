#include "Particle.h"

// Make sure you include SdFat.h before DeviceKeyHelperRK.h, otherwise you won't have support for SdFat
#include "SdFat.h"

#include "DeviceKeyHelperRK.h"

// Note: You should use SEMI_AUTOMATIC mode so the check for valid keys can be done before connecting.
SYSTEM_MODE(SEMI_AUTOMATIC);

// Pick a debug level from one of these two:
SerialLogHandler logHandler;
// SerialLogHandler logHandler(LOG_LEVEL_TRACE);

// Primary SPI with DMA
// SCK => A3, MISO => A4, MOSI => A5, SS => A2 (default)
SdFat sd;
const uint8_t chipSelect = A2;

// Secondary SPI with DMA
// SCK => D4, MISO => D3, MOSI => D2, SS => D1
// SdFat sd(1);
// const uint8_t chipSelect = D1;

DeviceKeyHelperSdFat deviceKeyHelper("keys");

void setup() {
	Serial.begin();

	if (sd.begin(chipSelect, SPI_HALF_SPEED)) {
		// If the file system was mounted, enable monitoring for keys errors
		deviceKeyHelper.startMonitor();
	}
	else {
		Log.info("failed to initialize SD card");
	}

	// You either need to use SYSTEM_THREAD(ENABLED) or SYSTEM_MODE(SEMI_AUTOMATIC) because
	// in thread disabled AUTOMATIC mode, setup() isn't called until cloud connected and the
	// code to monitor the connection would never be started via startMonitor().
	Particle.connect();
}

void loop() {
}
