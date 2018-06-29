/**
 * Particle library for saving and restoring device private and public keys
 *
 * Location: https://github.com/rickkas7/DeviceKeyHelperRK
 * License: MIT
 */

#include "DeviceKeyHelperRK.h"

Logger log("app.devicekeys");

DeviceKeyHelper::DeviceKeyHelper(std::function<bool(DeviceKeyHelperSavedData *savedData)> load, std::function<bool(const DeviceKeyHelperSavedData *savedData)> save) :
	load(load), save(save) {
}

DeviceKeyHelper::~DeviceKeyHelper() {

}

void DeviceKeyHelper::check(bool forceSaveCurrentKey) {
	// On TCP devices, this is 1600 bytes so it's kind of large to allocate on the stack safely, plus we need two of them.
	// We deallocate it before exiting this function.
	uint8_t *onDevice = new uint8_t[DEVICE_KEYS_HELPER_SIZE];
	if (onDevice) {
		DeviceKeyHelperSavedData *saved = new DeviceKeyHelperSavedData();
		if (saved) {
			//
			dct_read_app_data_copy(DEVICE_KEYS_HELPER_OFFSET, onDevice, DEVICE_KEYS_HELPER_SIZE);

			bool saveKeys = false;

			if (forceSaveCurrentKey) {
				log.info("force save device keys");
				saveKeys = true;
			}
			else
			if (load(saved)) {
				// We were able to load some data, but make sure it's valid
				if (validateData(saved)) {
					// Looks valid

					if (memcmp(onDevice, saved->keys, DEVICE_KEYS_HELPER_SIZE) != 0) {
						// Changed
						int res = dct_write_app_data(saved->keys, DEVICE_KEYS_HELPER_OFFSET, DEVICE_KEYS_HELPER_SIZE);
						log.info("device keys changed! reverting offset=%u size=%u result=%d", DEVICE_KEYS_HELPER_OFFSET, DEVICE_KEYS_HELPER_SIZE, res);

						dct_read_app_data_copy(DEVICE_KEYS_HELPER_OFFSET, onDevice, DEVICE_KEYS_HELPER_SIZE);
						log.info("read data back: %s", memcmp(onDevice, saved->keys, DEVICE_KEYS_HELPER_SIZE) ? "did not match" : "verified");

						// This delay is here because it keys reset should be rare, but if something goes wrong we don't want to go
						// into a fast rolling reboot because we could end up wearing out the flash.
						delay(5000);
						System.reset();
					}
					else {
						// Same
						log.info("device keys unchanged");
					}
				}
				else {
					// Not valid, just save the current keys
					log.info("was able to load device keys, but data was not valid");
					saveKeys = true;
				}

			}
			else {
				// Did not successfully load, so save the current key instead
				log.info("was unable to load existing key data");
				saveKeys = true;
			}

			if (saveKeys) {
				// Save a header (with magic bytes, length, and checksum)
				log.info("saving keys");
				memcpy(saved->keys, onDevice, DEVICE_KEYS_HELPER_SIZE);

				saved->magic = DATA_HEADER_MAGIC;
				saved->size = DEVICE_KEYS_HELPER_SIZE;
				saved->sum = calculateChecksum(saved);

				save(saved);
			}

			delete saved;
		}

		delete[] onDevice;
	}
}


uint16_t DeviceKeyHelper::calculateChecksum(const DeviceKeyHelperSavedData *savedData) const {
	uint16_t sum = 0;

	for(size_t ii = 0; ii < DEVICE_KEYS_HELPER_SIZE; ii++) {
		sum += savedData->keys[ii];
	}
	return sum;
}

bool DeviceKeyHelper::validateData(const DeviceKeyHelperSavedData *savedData) const {

	if (savedData->magic != DATA_HEADER_MAGIC || savedData->size != DEVICE_KEYS_HELPER_SIZE) {
		log.info("bad magic bytes or size magic=%08lx size=%u", savedData->magic, savedData->size);
		return false;
	}

	if (savedData->sum != calculateChecksum(savedData)) {
		log.info("bad checksum");
		return false;
	}
	return true;
}

