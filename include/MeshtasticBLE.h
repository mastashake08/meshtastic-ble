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
    
    // Get device name
    String getDeviceName();

private:
    BLEServer* pServer;
    BLEService* pService;
    BLECharacteristic* pToRadioChar;
    BLECharacteristic* pFromRadioChar;
    BLECharacteristic* pFromNumChar;
    
    String deviceName;
    bool connected;
    uint32_t fromNum;
    
    std::function<void(uint8_t*, size_t)> dataCallback;
    
    class ServerCallbacks;
    class ToRadioCallbacks;
    
    friend class ServerCallbacks;
    friend class ToRadioCallbacks;
};

#endif // MESHTASTIC_BLE_H
