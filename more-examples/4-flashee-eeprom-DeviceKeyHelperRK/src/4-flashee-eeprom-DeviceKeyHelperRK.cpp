#include "Particle.h"

// Store keys as a file in a flashee-eeprom file system on a P1
// https://github.com/m-mcgowan/spark-flashee-eeprom/

// Make sure you include flashee-eeprom.h before DeviceKeyHelperRK.h, otherwise you won't have support for
// DeviceKeyHelperFlasheeFile
#include "flashee-eeprom.h"

#include "DeviceKeyHelperRK.h"

// Note: You should use SEMI_AUTOMATIC mode so the check for valid keys can be done before connecting.
SYSTEM_MODE(SEMI_AUTOMATIC);

// Pick a debug level from one of these two:
SerialLogHandler logHandler;
// SerialLogHandler logHandler(LOG_LEVEL_TRACE);

// The flashee FAT file system
FATFS fs;


void setup() {
	Serial.begin();

	// Not necessary, though provided here so you can see the serial log messages more easily
	delay(4000);

	FRESULT fResult =  Flashee::Devices::createFATRegion(0, 4096*256, &fs);
	if (fResult == FR_OK) {
		// Check the device public and private keys against the keys stored in the file "keys"
		DeviceKeyHelperFlasheeFile deviceKeyHelper("keys");
		deviceKeyHelper.check();
	}
	else {
		Log.info("failed to mount flashee file system %d", fResult);
	}


	Particle.connect();
}

void loop() {
}
