#include "MeshtasticBLE.h"

// Server connection callbacks
class MeshtasticBLE::ServerCallbacks: public BLEServerCallbacks {
    MeshtasticBLE* parent;
public:
    ServerCallbacks(MeshtasticBLE* p) : parent(p) {}
    
    void onConnect(BLEServer* pServer) {
        parent->connected = true;
        Serial.println("Client connected!");
    }
    
    void onDisconnect(BLEServer* pServer) {
        parent->connected = false;
        Serial.println("Client disconnected!");
        // Restart advertising
        delay(500);
        parent->startAdvertising();
    }
};

// ToRadio characteristic write callbacks
class MeshtasticBLE::ToRadioCallbacks: public BLECharacteristicCallbacks {
    MeshtasticBLE* parent;
public:
    ToRadioCallbacks(MeshtasticBLE* p) : parent(p) {}
    
    void onWrite(BLECharacteristic* pCharacteristic) {
        String value = pCharacteristic->getValue();
        
        if (value.length() > 0) {
            Serial.printf("Received %d bytes on ToRadio\n", value.length());
            
            // Call the registered callback
            if (parent->dataCallback) {
                parent->dataCallback((uint8_t*)value.c_str(), value.length());
            }
        }
    }
};

MeshtasticBLE::MeshtasticBLE() 
    : pServer(nullptr)
    , pService(nullptr)
    , pToRadioChar(nullptr)
    , pFromRadioChar(nullptr)
    , pFromNumChar(nullptr)
    , connected(false)
    , fromNum(0) {
}

MeshtasticBLE::~MeshtasticBLE() {
    if (pServer) {
        pServer->getAdvertising()->stop();
    }
}

bool MeshtasticBLE::begin(const String& name) {
    deviceName = name;
    
    Serial.println("Initializing BLE Server...");
    BLEDevice::init(deviceName.c_str());
    
    // Create BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks(this));
    
    // Create Meshtastic Service
    pService = pServer->createService(MESHTASTIC_SERVICE_UUID);
    
    // Create ToRadio characteristic (writable - receives data from client)
    pToRadioChar = pService->createCharacteristic(
        TORADIO_UUID,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
    );
    pToRadioChar->setCallbacks(new ToRadioCallbacks(this));
    
    // Create FromRadio characteristic (readable + notify - sends data to client)
    pFromRadioChar = pService->createCharacteristic(
        FROMRADIO_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    // BLE2902 descriptor automatically added by NimBLE for notify characteristic
    
    // Create FromNum characteristic (notify - indicates new data available)
    pFromNumChar = pService->createCharacteristic(
        FROMNUM_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    // BLE2902 descriptor automatically added by NimBLE for notify characteristic
    
    // Start the service
    pService->start();
    
    // Start advertising
    startAdvertising();
    
    Serial.println("BLE Server started!");
    Serial.printf("Device name: %s\n", deviceName.c_str());
    Serial.println("Waiting for connections...");
    
    return true;
}

void MeshtasticBLE::startAdvertising() {
    BLEAdvertising* pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(MESHTASTIC_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // Functions for iPhone connection optimization
    pAdvertising->setMaxPreferred(0x12);
    pAdvertising->start();
    Serial.println("BLE advertising started");
}

void MeshtasticBLE::stopAdvertising() {
    pServer->getAdvertising()->stop();
    Serial.println("BLE advertising stopped");
}

bool MeshtasticBLE::isConnected() {
    return connected;
}

bool MeshtasticBLE::sendFromRadio(uint8_t* data, size_t length) {
    if (pFromRadioChar == nullptr) {
        Serial.println("FromRadio characteristic not initialized");
        return false;
    }
    
    // Update the characteristic value
    pFromRadioChar->setValue(data, length);
    
    // Notify connected clients if any
    if (connected) {
        pFromRadioChar->notify();
        
        // Update FromNum to indicate new data
        fromNum++;
        pFromNumChar->setValue(fromNum);
        pFromNumChar->notify();
        
        Serial.printf("Sent %d bytes via FromRadio (packet #%d)\n", length, fromNum);
    }
    
    return true;
}

void MeshtasticBLE::onDataReceived(std::function<void(uint8_t*, size_t)> callback) {
    dataCallback = callback;
}

String MeshtasticBLE::getDeviceName() {
    return deviceName;
}
