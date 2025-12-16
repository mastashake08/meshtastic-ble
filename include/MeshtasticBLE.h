#ifndef MESHTASTIC_BLE_H
#define MESHTASTIC_BLE_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <functional>

// Meshtastic BLE Service and Characteristic UUIDs
#define MESHTASTIC_SERVICE_UUID      "6ba1b218-15a8-461f-9fa8-5dcae273eafd"
#define TORADIO_UUID                 "f75c76d2-129e-4dad-a1dd-7866124401e7"
#define FROMRADIO_UUID               "2c55e69e-4993-11ed-b878-0242ac120002"
#define FROMNUM_UUID                 "ed9da18c-a800-4f66-a670-aa7547e34453"
#define KEY_CONTROL_UUID             "a1b2c3d4-e5f6-7890-abcd-ef1234567890"

// Standard Battery Service UUID
#define BATTERY_SERVICE_UUID         "0000180F-0000-1000-8000-00805f9b34fb"
#define BATTERY_LEVEL_UUID           "00002A19-0000-1000-8000-00805f9b34fb"

class MeshtasticBLE {
public:
    MeshtasticBLE();
    ~MeshtasticBLE();

    // Initialize BLE Server
    bool begin(const String& deviceName = "Meshtastic-ESP32");
    
    // Start advertising
    void startAdvertising();
    void stopAdvertising();
    
    // Check connection status
    bool isConnected();
    
    // Send data from radio (notify clients)
    bool sendFromRadio(uint8_t* data, size_t length);
    
    // Register callback for received data (ToRadio writes)
    void onDataReceived(std::function<void(uint8_t*, size_t)> callback);
    
    // Register callback for key control commands
    void onKeyCommand(std::function<void(const String&)> callback);
    
    // Update battery level (0-100%)
    void updateBatteryLevel(uint8_t level);
    
    // Get device name
    String getDeviceName();

private:
    BLEServer* pServer;
    BLEService* pService;
    BLECharacteristic* pToRadioChar;
    BLECharacteristic* pFromRadioChar;
    BLECharacteristic* pFromNumChar;
    BLECharacteristic* pKeyControlChar;
    
    BLEService* pBatteryService;
    BLECharacteristic* pBatteryLevelChar;
    
    String deviceName;
    bool connected;
    uint32_t fromNum;
    
    std::function<void(uint8_t*, size_t)> dataCallback;
    std::function<void(const String&)> keyCallback;
    
    class ServerCallbacks;
    class ToRadioCallbacks;
    class KeyControlCallbacks;
    
    friend class ServerCallbacks;
    friend class ToRadioCallbacks;
    friend class KeyControlCallbacks;
};

#endif // MESHTASTIC_BLE_H
