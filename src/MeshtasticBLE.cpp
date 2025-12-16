#include "MeshtasticBLE.h"

MeshtasticBLE* MeshtasticBLE::instance = nullptr;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    std::vector<BLEAdvertisedDevice>* devices;
public:
    MyAdvertisedDeviceCallbacks(std::vector<BLEAdvertisedDevice>* devs) : devices(devs) {}
    
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        // Check if device advertises Meshtastic service
        if (advertisedDevice.haveServiceUUID() && 
            advertisedDevice.isAdvertisingService(BLEUUID(MESHTASTIC_SERVICE_UUID))) {
            Serial.printf("Found Meshtastic device: %s\n", advertisedDevice.toString().c_str());
            devices->push_back(advertisedDevice);
        }
    }
};

MeshtasticBLE::MeshtasticBLE() 
    : pClient(nullptr)
    , pScan(nullptr)
    , pToRadioChar(nullptr)
    , pFromRadioChar(nullptr)
    , pFromNumChar(nullptr)
    , connected(false) {
    instance = this;
}

MeshtasticBLE::~MeshtasticBLE() {
    disconnect();
    instance = nullptr;
}

bool MeshtasticBLE::begin() {
    Serial.println("Initializing BLE...");
    BLEDevice::init("Meshtastic-Controller");
    
    pScan = BLEDevice::getScan();
    pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(&foundDevices));
    pScan->setActiveScan(true);
    pScan->setInterval(100);
    pScan->setWindow(99);
    
    return true;
}

bool MeshtasticBLE::scanForDevices(uint32_t scanDuration) {
    foundDevices.clear();
    Serial.println("Scanning for Meshtastic devices...");
    
    pScan->start(scanDuration, false);
    pScan->clearResults();
    
    Serial.printf("Found %d Meshtastic device(s)\n", foundDevices.size());
    return foundDevices.size() > 0;
}

bool MeshtasticBLE::connect(BLEAddress address) {
    if (connected) {
        disconnect();
    }
    
    Serial.printf("Connecting to %s...\n", address.toString().c_str());
    
    pClient = BLEDevice::createClient();
    
    if (!pClient->connect(address)) {
        Serial.println("Failed to connect");
        return false;
    }
    
    Serial.println("Connected! Getting service...");
    
    BLERemoteService* pService = pClient->getService(MESHTASTIC_SERVICE_UUID);
    if (pService == nullptr) {
        Serial.println("Failed to find Meshtastic service");
        pClient->disconnect();
        return false;
    }
    
    // Get characteristics
    pToRadioChar = pService->getCharacteristic(TORADIO_UUID);
    pFromRadioChar = pService->getCharacteristic(FROMRADIO_UUID);
    pFromNumChar = pService->getCharacteristic(FROMNUM_UUID);
    
    if (pToRadioChar == nullptr || pFromRadioChar == nullptr) {
        Serial.println("Failed to find required characteristics");
        pClient->disconnect();
        return false;
    }
    
    // Register for notifications on FromRadio
    if (pFromRadioChar->canNotify()) {
        pFromRadioChar->registerForNotify(notifyCallback);
    }
    
    connected = true;
    Serial.println("Successfully connected to Meshtastic device!");
    return true;
}

bool MeshtasticBLE::connectToFirst() {
    if (foundDevices.size() == 0) {
        Serial.println("No devices found. Run scan first.");
        return false;
    }
    
    return connect(foundDevices[0].getAddress());
}

void MeshtasticBLE::disconnect() {
    if (pClient != nullptr && connected) {
        pClient->disconnect();
        connected = false;
        Serial.println("Disconnected from device");
    }
}

bool MeshtasticBLE::isConnected() {
    return connected && pClient != nullptr && pClient->isConnected();
}

bool MeshtasticBLE::sendToRadio(uint8_t* data, size_t length) {
    if (!isConnected() || pToRadioChar == nullptr) {
        Serial.println("Not connected");
        return false;
    }
    
    pToRadioChar->writeValue(data, length);
    return true;
}

void MeshtasticBLE::onDataReceived(std::function<void(uint8_t*, size_t)> callback) {
    dataCallback = callback;
}

void MeshtasticBLE::notifyCallback(BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
    if (instance != nullptr && instance->dataCallback) {
        instance->dataCallback(pData, length);
    }
}

int MeshtasticBLE::getDeviceCount() {
    return foundDevices.size();
}

BLEAddress MeshtasticBLE::getDeviceAddress(int index) {
    if (index >= 0 && index < foundDevices.size()) {
        return foundDevices[index].getAddress();
    }
    return BLEAddress("");
}

String MeshtasticBLE::getDeviceName(int index) {
    if (index >= 0 && index < foundDevices.size()) {
        return foundDevices[index].getName().c_str();
    }
    return "";
}
