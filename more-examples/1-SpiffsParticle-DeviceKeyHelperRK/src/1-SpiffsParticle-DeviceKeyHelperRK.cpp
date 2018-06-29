#include "Particle.h"

// Make sure you include SpiffsParticleRK.h before DeviceKeyHelperRK.h, otherwise you won't have support for SpiffsParticle
#include "SpiffsParticleRK.h"

#include "DeviceKeyHelperRK.h"

// Note: You should use SEMI_AUTOMATIC mode so the check for valid keys can be done before connecting.
SYSTEM_MODE(SEMI_AUTOMATIC);

// Pick a debug level from one of these two:
SerialLogHandler logHandler;
// SerialLogHandler logHandler(LOG_LEVEL_TRACE);

// Chose a flash configuration:
// SpiFlashISSI spiFlash(SPI, A2); 		// ISSI flash on SPI (A pins)
// SpiFlashISSI spiFlash(SPI1, D5);		// ISSI flash on SPI1 (D pins)
// SpiFlashMacronix spiFlash(SPI1, D5);	// Macronix flash on SPI1 (D pins), typical config for E series
// SpiFlashWinbond spiFlash(SPI, A2);	// Winbond flash on SPI (A pins)
SpiFlashP1 spiFlash;					// P1 external flash inside the P1 module

// Create an object for the SPIFFS file system
SpiffsParticle fs(spiFlash);

void setup() {
	Serial.begin();

	// Not necessary, though provided here so you can see the serial log messages more easily
	delay(4000);

	// Initialize SPI flash with a volume size of 256K
	spiFlash.begin();
	fs.withPhysicalSize(256 * 1024);

	// Mount the SPIFFS file system
	s32_t res = fs.mountAndFormatIfNecessary();
	Log.info("mount res=%d", res);

	if (res == SPIFFS_OK) {
		// Check the device public and private keys against the file "keys" in the SPIFFS file
		// system.
		DeviceKeyHelperSpiffsParticle deviceKeyHelper(fs, "keys");
		deviceKeyHelper.check();
	}

	Particle.connect();
}

void loop() {
}
