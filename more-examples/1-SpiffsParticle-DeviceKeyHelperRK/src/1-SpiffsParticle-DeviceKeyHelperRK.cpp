#include "Particle.h"

// Make sure you include SpiffsParticleRK.h before DeviceKeyHelperRK.h, otherwise you won't have support for SpiffsParticle
#include "SpiffsParticleRK.h"

#include "DeviceKeyHelperRK.h"

SYSTEM_MODE(SEMI_AUTOMATIC);

// Set a reasonable logging level:
SerialLogHandler logHandler(LOG_LEVEL_WARN, { // Logging level for non-application messages
    { "app", LOG_LEVEL_INFO }, // Default logging level for all application messages
    { "app.spiffs", LOG_LEVEL_WARN } // Disable spiffs info and trace messages
});

// Chose a flash configuration:
// SpiFlashISSI spiFlash(SPI, A2); 		// ISSI flash on SPI (A pins)
// SpiFlashISSI spiFlash(SPI1, D5);		// ISSI flash on SPI1 (D pins)
SpiFlashMacronix spiFlash(SPI1, D5);	// Macronix flash on SPI1 (D pins), typical config for E series
// SpiFlashWinbond spiFlash(SPI, A2);	// Winbond flash on SPI (A pins)
// SpiFlashP1 spiFlash;					// P1 external flash inside the P1 module

// Create an object for the SPIFFS file system
SpiffsParticle fs(spiFlash);

DeviceKeyHelperSpiffsParticle deviceKeyHelper(fs, "keys");

void setup() {
	Serial.begin();

	// Initialize SPI flash with a volume size of 256K
	spiFlash.begin();
	fs.withPhysicalSize(256 * 1024);

	// Mount the SPIFFS file system
	s32_t res = fs.mountAndFormatIfNecessary();
	Log.info("mount res=%ld", res);

	if (res == SPIFFS_OK) {
		// If the file system was mounted, enable monitoring for keys errors
		deviceKeyHelper.startMonitor();
	}

	// You either need to use SYSTEM_THREAD(ENABLED) or SYSTEM_MODE(SEMI_AUTOMATIC) because
	// in thread disabled AUTOMATIC mode, setup() isn't called until cloud connected and the
	// code to monitor the connection would never be started via startMonitor().
	Particle.connect();
}

void loop() {
}
