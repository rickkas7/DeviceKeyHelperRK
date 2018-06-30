#include "Particle.h"

#include "DeviceKeyHelperRK.h"

// This is required, because we reset the keys during setup()
SYSTEM_MODE(SEMI_AUTOMATIC);

SerialLogHandler logHandler;
//SerialLogHandler logHandler(LOG_LEVEL_TRACE);

// Save and restore the device keys in EEPROM at offset 100 in the EEPROM
// The amount os space used at that offset depends on the device:
// - For Wi-Fi devices (Photon, P1): 1608 bytes
// - For cellular devices (Electron, E series): 328 bytes
DeviceKeyHelperEEPROM deviceKeyHelper(100);

void setup() {

	// You must call this from setup to start monitoring for keys errors
	deviceKeyHelper.startMonitor();

	// You either need to use SYSTEM_THREAD(ENABLED) or SYSTEM_MODE(SEMI_AUTOMATIC) because
	// in thread disabled AUTOMATIC mode, setup() isn't called until cloud connected and the
	// code to monitor the connection would never be started via startMonitor().
	Particle.connect();
}

void loop() {

}

