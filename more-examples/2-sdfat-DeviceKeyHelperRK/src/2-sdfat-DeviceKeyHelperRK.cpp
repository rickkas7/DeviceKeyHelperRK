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

void setup() {
	Serial.begin();

	// Not necessary, though provided here so you can see the serial log messages more easily
	delay(4000);

	if (sd.begin(chipSelect, SPI_HALF_SPEED)) {
		// Check the device public and private keys against the file "keys" in the SD card file system.
		DeviceKeyHelperSdFat deviceKeyHelper("keys");
		deviceKeyHelper.check();
	}
	else {
		Log.info("failed to initialize SD card");
	}

	Particle.connect();
}

void loop() {
}
