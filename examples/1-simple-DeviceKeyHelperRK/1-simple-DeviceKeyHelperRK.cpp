#include "Particle.h"

#include "DeviceKeyHelperRK.h"

// This is required, because we reset the keys during setup()
SYSTEM_MODE(SEMI_AUTOMATIC);

SerialLogHandler logHandler;
//SerialLogHandler logHandler(LOG_LEVEL_TRACE);

void setup() {

	delay(4000);

	{
		// Save and restore the device keys in EEPROM at offset 100 in the EEPROM
		DeviceKeyHelperEEPROM keyHelper(100);
		keyHelper.check();
	}

	// Wait until the keys have possibly been restored before trying to connect.
	// In SEMI_AUTOMATIC mode, connection retries and everything like that will
	// work just like AUTOMATIC after you do the Particle.connect once, no need
	// to manually manage the connection.
	Particle.connect();
}

void loop() {

}

