#ifndef MESHTASTIC_BLE_H
#define MESHTASTIC_BLE_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEClient.h>
#include <BLEScan.h>
#include <functional>

// Meshtastic BLE Service and Characteristic UUIDs
#define MESHTASTIC_SERVICE_UUID      "6ba1b218-15a8-461f-9fa8-5dcae273eafd"
#define TORADIO_UUID                 "f75c76d2-129e-4dad-a1dd-7866124401e7"
#define FROMRADIO_UUID               "2c55e69e-4993-11ed-b878-0242ac120002"
#define FROMNUM_UUID                 "ed9da18c-a800-4f66-a670-aa7547e34453"

class MeshtasticBLE {
public:
    MeshtasticBLE();
    ~MeshtasticBLE();

    // Initialize BLE
    bool begin();
    
    // Scan for Meshtastic devices
    bool scanForDevices(uint32_t scanDuration = 5);
    
    // Connect to a specific device
    bool connect(BLEAddress address);
    bool connectToFirst(); // Connect to first found device
    
    // Disconnect
    void disconnect();
    
    // Check connection status
    bool isConnected();
    
    // Send data to radio (ToRadio)
    bool sendToRadio(uint8_t* data, size_t length);
    
    // Register callback for received data (FromRadio)
    void onDataReceived(std::function<void(uint8_t*, size_t)> callback);
    
    // Get list of found devices
    int getDeviceCount();
    BLEAddress getDeviceAddress(int index);
    String getDeviceName(int index);

private:
    BLEClient* pClient;
    BLEScan* pScan;
    BLERemoteCharacteristic* pToRadioChar;
    BLERemoteCharacteristic* pFromRadioChar;
    BLERemoteCharacteristic* pFromNumChar;
    
    std::vector<BLEAdvertisedDevice> foundDevices;
    bool connected;
    
    std::function<void(uint8_t*, size_t)> dataCallback;
    
    static void notifyCallback(BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify);
    static MeshtasticBLE* instance; // For static callback
};

#endif // MESHTASTIC_BLE_H
