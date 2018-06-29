# Device Key Helper

## Background

### About public key cryptography

Particle devices authenticate with the cloud using public key cryptography. Each side has a public key and a private key. The device private key is only stored on the device but the public key can be shared even over insecure channels. You can share it with everyone, even. The cloud knows every device's public key.

For the cloud side, the cloud private key is kept secret, but all devices know the cloud public key. It's publicly available on a web site and in the Particle CLI source. The public key is not a secret.

When a device handshakes, it encrypts some data using the device private key and the cloud public key. The cloud is able to decrypt this because it knows the cloud private key and the device public key (the opposite side). It then sends data back to the device encrypted with the cloud private key and the device public key.

That data can only be decrypted by the device because decrypting it requires the cloud public key an the device private key, and only the device knows its private keys. 

This process assures that the cloud is who it says it is, not a rogue cloud, because only the real cloud knows its private key. And the cloud knows the device is authentic, because only the device knows its private key.

And a third-party snooping on the process can't gain anything because the private keys never leave the respective sides.

This process requires a lot of computation, and is only used to authenticate both sides and set up a session key to encrypt the data for the connection using symmetric encryption like AES.

### The device key problem

The private device key is stored in the configuration flash on the device, and only there. But what happens if the flash is erased or corrupted?

In that case the device will generate a new private and public device key. They have to be generated in pairs because they're cryptographically connected, each private key has a specific public key that's able to decrypt its data.

When the device goes to connect with the cloud using its new device keys, however, there's a problem: the public key stored on the cloud doesn't match the current public key, and it can't decrypt the data. It assumes that some rogue device is trying to impersonate a different device and shuts down the connection.

### particle keys doctor

If you've experienced this, you probably know that you fix this in DFU mode (blinking yellow) by using using:

```
particle keys doctor YOUR_DEVICE_ID
```

What this does is upload your new device private key to the cloud, so they match again.

### Why can't the device just do it itself?

There are two reasons:

- The keys upload uses TLS/SSL encryption to the Particle API and the Photon and Electron don't support that very well.
- When you use the Particle CLI, you need to be logged into the account that has claimed the device.

The latter is the real problem. Quite intentionally, devices don't contain login credentials (password or access token) for the account that it is claimed to. If it did, you'd be able to steal a device and take over the account, and that would be bad. So there are no access tokens stored on the device, so there is no way to authenticate a change of key from the device itself.

### The cloud public key

The cloud public key can also be erased. This is fixed using:

```
particle keys server
```

System firmware 0.7.0 and later can repair the cloud public key by itself, because that's not a secret and it's the same for every device class. (Photons and Electrons have different cloud public keys, but every Photon has the same cloud public key.)


## Key recovery to the rescue

What if, instead of generating a new, incompatible device private key, you just restored the old one? That's what this library does.

### Why doesn't the device just do that itself?

The device private key is kind of large and specific to each device. While there's a demo of storing it in the virtual EEPROM, that's not the best location as the ideal location is not in the STM32 flash. Also, it takes over a good chunk of the EEPROM.

Ideally, this requires an external flash (SD card or SPI flash) or something like a FRAM. Since that's not standard equipment, it's not practical to include this in system firmware.


