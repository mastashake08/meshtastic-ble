#include "KeyManager.h"

const char* KeyManager::NAMESPACE = "meshtastic";
const char* KeyManager::PRIVATE_KEY = "priv_key";
const char* KeyManager::PUBLIC_KEY = "pub_key";

KeyManager::KeyManager() {
}

KeyManager::~KeyManager() {
    preferences.end();
}

bool KeyManager::begin() {
    return preferences.begin(NAMESPACE, false); // Read/write mode
}

bool KeyManager::importPrivateKey(const String& privateKey) {
    if (privateKey.length() == 0) {
        Serial.println("Error: Empty private key");
        return false;
    }
    
    size_t written = preferences.putString(PRIVATE_KEY, privateKey);
    if (written > 0) {
        Serial.println("Private key imported successfully");
        return true;
    }
    
    Serial.println("Failed to import private key");
    return false;
}

bool KeyManager::importPublicKey(const String& publicKey) {
    if (publicKey.length() == 0) {
        Serial.println("Error: Empty public key");
        return false;
    }
    
    size_t written = preferences.putString(PUBLIC_KEY, publicKey);
    if (written > 0) {
        Serial.println("Public key imported successfully");
        return true;
    }
    
    Serial.println("Failed to import public key");
    return false;
}

bool KeyManager::importKeys(const String& privateKey, const String& publicKey) {
    bool privSuccess = importPrivateKey(privateKey);
    bool pubSuccess = importPublicKey(publicKey);
    
    if (privSuccess && pubSuccess) {
        Serial.println("Keys imported successfully");
        return true;
    }
    
    return false;
}

String KeyManager::getPrivateKey() {
    return preferences.getString(PRIVATE_KEY, "");
}

String KeyManager::getPublicKey() {
    return preferences.getString(PUBLIC_KEY, "");
}

bool KeyManager::hasPrivateKey() {
    return preferences.isKey(PRIVATE_KEY);
}

bool KeyManager::hasPublicKey() {
    return preferences.isKey(PUBLIC_KEY);
}

bool KeyManager::hasKeys() {
    return hasPrivateKey() && hasPublicKey();
}

void KeyManager::clearKeys() {
    preferences.remove(PRIVATE_KEY);
    preferences.remove(PUBLIC_KEY);
    Serial.println("Keys cleared");
}

bool KeyManager::getPrivateKeyRaw(uint8_t* buffer, size_t maxLen) {
    String key = getPrivateKey();
    if (key.length() == 0) {
        return false;
    }
    
    // Store the key bytes (assumes hex or base64 string)
    // This is a simplified version - you may need to add base64 decoding
    size_t len = min((size_t)key.length(), maxLen);
    memcpy(buffer, key.c_str(), len);
    return true;
}

bool KeyManager::getPublicKeyRaw(uint8_t* buffer, size_t maxLen) {
    String key = getPublicKey();
    if (key.length() == 0) {
        return false;
    }
    
    // Store the key bytes (assumes hex or base64 string)
    size_t len = min((size_t)key.length(), maxLen);
    memcpy(buffer, key.c_str(), len);
    return true;
}
