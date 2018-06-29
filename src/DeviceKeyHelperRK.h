/**
 * Particle library for saving and restoring device private and public keys
 *
 * Location: https://github.com/rickkas7/DeviceKeyHelperRK
 * License: MIT
 */

#ifndef __DEVICEKEYHELPERRK_H
#define __DEVICEKEYHELPERRK_H


#include "Particle.h"
#include "dct.h"

// The size of the public and private keys depends on whether the device uses UDP (cellular devices, typically)
// which use the ALT key slot, or TCP (Wi-Fi devices) which use the main key slot
#if HAL_PLATFORM_CLOUD_UDP
const size_t DEVICE_KEYS_HELPER_SIZE = DCT_ALT_DEVICE_PUBLIC_KEY_SIZE + DCT_ALT_DEVICE_PRIVATE_KEY_SIZE;
const size_t DEVICE_KEYS_HELPER_OFFSET = DCT_ALT_DEVICE_PUBLIC_KEY_OFFSET;
#else
const size_t DEVICE_KEYS_HELPER_SIZE = DCT_DEVICE_PUBLIC_KEY_SIZE + DCT_DEVICE_PRIVATE_KEY_SIZE;
const size_t DEVICE_KEYS_HELPER_OFFSET = DCT_DEVICE_PUBLIC_KEY_OFFSET;
#endif

/**
 * @brief Structure for holding saved keys, including magic bytes, size, checksum, and the actual keys.
 *
 * This is what is saved in EEPROM, SPI Flash, FRAM, etc.
 */
typedef struct {
	uint32_t	magic;  // DATA_HEADER_MAGIC = 0x75a65c63
	uint16_t	size;	// size of the keys field only, DEVICE_KEYS_HELPER_SIZE not the size of the structure!
	uint16_t	sum; 	// Checksum of the keys field only. Straight sum of uint8_t bytes, 16 bits wide.
	uint8_t 	keys[DEVICE_KEYS_HELPER_SIZE];
} DeviceKeyHelperSavedData;

/**
 * @brief Base class for saving and restoring data
 *
 * Normally you'll use a class for a specific storage medium like DeviceKeyHelperEEPROM or
 * DeviceKeyHelperSpiffsParticle however you can use this directly or subclass it if you have
 * a different storage method you want to use.
 */
class DeviceKeyHelper {
public:
	/**
	 * @brief Constructor
	 *
	 * @param load
	 *
	 * @param save
	 */
	DeviceKeyHelper(std::function<bool(DeviceKeyHelperSavedData *savedData)> load, std::function<bool(const DeviceKeyHelperSavedData *savedData)> save);
	virtual ~DeviceKeyHelper();

	/**
	 * @brief Call to check the keys
	 *
	 * NOTE: If the keys are swapped back to the saved key, this function will pause, then restart.
	 *
	 * @param forceSaveCurrentKey (optional, default: false) If true, the current key will be saved always. This
	 * is useful if you just changed the device keys (particle keys load or particle keys doctor).
	 */
	void check(bool forceSaveCurrentKey = false);


protected:
	/**
	 * @brief Calculate the 16-bit checksum of the keys array in savedData
	 */
	uint16_t calculateChecksum(const DeviceKeyHelperSavedData *savedData) const;

	/**
	 * @brief Validate savedData, making sure magic, size, and the checksum are correct
	 *
	 * @returns true if valid, false if not
	 */
	bool validateData(const DeviceKeyHelperSavedData *savedData) const;

	static const uint32_t DATA_HEADER_MAGIC = 0x75a65c63;

	std::function<bool(DeviceKeyHelperSavedData *savedData)> load;
	std::function<bool(const DeviceKeyHelperSavedData *savedData)> save;
};

/**
 * @brief Class to save the keys in the emulated EEPROM
 *
 *
 */
class DeviceKeyHelperEEPROM : public DeviceKeyHelper {
public:
	/**
	 * @brief Store data in the onboard emulate EEPROM
	 *
	 * @param offset The offset to write to.
	 *
	 * The amount of space you need depends on your platform:
	 * For Wi-Fi devices (Photon, P1): 1608 bytes
	 * For cellular devices (Electron, E series): 328 bytes
	 *
	 * The emulated EEPROM on the Photon, P1, and Electron is 2047 bytes so storing Wi-Fi device
	 * keys will se most of it.
	 */
	inline DeviceKeyHelperEEPROM(size_t offset) :
		DeviceKeyHelper([offset](DeviceKeyHelperSavedData *savedData) {
			// Log.info("getting %u bytes at %u", sizeof(*savedData), offset);
			EEPROM.get(offset, *savedData);
			return true;
		},
		[offset](const DeviceKeyHelperSavedData *savedData) {
			// Log.info("saving %u bytes at %u", sizeof(*savedData), offset);
			EEPROM.put(offset, *savedData);
			return true;
		}) {
	};
};

#ifdef __SPIFFSPARTICLERK_H
/**
 * @brief Version that uses SpiffsParticleRK to save to SPI flash
 *
 * This includes many standalone SPI flash chips, the unpopulated set of pads on the E series module
 * and the external flash on the P1.
 *
 * https://github.com/rickkas7/SpiffsParticleRK
 */
class DeviceKeyHelperSpiffsParticle : public DeviceKeyHelper {
public:
	/**
	 * @brief Store data a SPIFFS file system
	 *
	 * @param fs The SpiffsParticle file system object to store in. The file system must be mounted.
	 *
	 * @param filename The filename to store the keys in. Filenames are limited to 32 character and there
	 * are no subdirectories in SPIFFS
	 */
	inline DeviceKeyHelperSpiffsParticle(SpiffsParticle &fs, const char *filename) :
		DeviceKeyHelper([&fs, filename](DeviceKeyHelperSavedData *savedData) {
			bool result = false;

			SpiffsParticleFile f = fs.openFile(filename, SPIFFS_O_RDONLY);
			if (f.isValid()) {
				if (f.length() == sizeof(DeviceKeyHelperSavedData)) {
					size_t len = f.readBytes((char *)savedData, sizeof(DeviceKeyHelperSavedData));
					result = (len == sizeof(DeviceKeyHelperSavedData));
				}
				f.close();
			}
			return result;
		},
		[&fs, filename](const DeviceKeyHelperSavedData *savedData) {
			bool result = false;

			SpiffsParticleFile f = fs.openFile(filename, SPIFFS_O_CREAT|SPIFFS_O_RDWR|SPIFFS_O_TRUNC);
			if (f.isValid()) {
				size_t len = f.write((const uint8_t *)savedData, sizeof(DeviceKeyHelperSavedData));
				result = (len == sizeof(DeviceKeyHelperSavedData));
				f.close();
			}
			return result;
		}) {
	};
};
#endif /* __SPIFFSPARTICLERK_H */

#ifdef SdFat_h
class DeviceKeyHelperSdFat : public DeviceKeyHelper {
public:
	/**
	 * @brief Store data a SdFat SD card file system
	 *
	 * @param filename The filename to store the keys in
	 */
	inline DeviceKeyHelperSdFat(const char *filename) :
		DeviceKeyHelper([filename](DeviceKeyHelperSavedData *savedData) {
			bool result = false;

			File f;
			if (f.open(filename, O_READ)) {
				if (f.size() == sizeof(DeviceKeyHelperSavedData)) {
					size_t len = f.read((char *)savedData, sizeof(DeviceKeyHelperSavedData));
					result = (len == sizeof(DeviceKeyHelperSavedData));
				}

				f.close();
			}
			return result;
		},
		[filename](const DeviceKeyHelperSavedData *savedData) {
			bool result = false;

			File f;
			if (f.open(filename, O_CREAT | O_RDWR | O_TRUNC)) {
				size_t len = f.write((const uint8_t *)savedData, sizeof(DeviceKeyHelperSavedData));
				result = (len == sizeof(DeviceKeyHelperSavedData));
				f.close();
			}
			return result;
		}) {
	};
};
#endif /* SdFat_h */

#ifdef __MB85RC256V_FRAM_RK
/**
 * @brief Interface to store device keys in a 32K I2C FRAM (MB85RC256V)
 *
 * Hardware:
 * https://www.adafruit.com/products/1895
 *
 * Library:
 * https://github.com/rickkas7/MB85RC256V-FRAM-RK
 */
class DeviceKeyHelperFRAM : public DeviceKeyHelper {
public:
	/**
	 * @brief Store data in a MB85RC256V FRAM (non-volatile ferro-electric RAM)
	 *
	 * @param fram The MB85RC256V object, typically a global variable See the library
	 * https://github.com/rickkas7/MB85RC256V-FRAM-RK
	 *
	 * @param offset The offset to write to.
	 *
	 * The amount of space you need depends on your platform:
	 * For Wi-Fi devices (Photon, P1): 1608 bytes
	 * For cellular devices (Electron, E series): 328 bytes
	 */
	inline DeviceKeyHelperFRAM(MB85RC256V &fram, size_t offset) :
		DeviceKeyHelper([&fram, offset](DeviceKeyHelperSavedData *savedData) {
			// Log.info("getting %u bytes at %u", sizeof(*savedData), offset);
			fram.get(offset, *savedData);
			return true;
		},
		[&fram, offset](const DeviceKeyHelperSavedData *savedData) {
			// Log.info("saving %u bytes at %u", sizeof(*savedData), offset);
			fram.put(offset, *savedData);
			return true;
		}) {
	};
};
#endif /* __MB85RC256V_FRAM_RK */



#endif /* __DEVICEKEYHELPERRK_H */
