#ifndef KEY_MANAGER_H
#define KEY_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>

class KeyManager {
public:
    KeyManager();
    ~KeyManager();

    // Initialize key storage
    bool begin();
    
    // Import and store keys (Base64 or hex encoded)
    bool importPrivateKey(const String& privateKey);
    bool importPublicKey(const String& publicKey);
    
    // Import both keys at once
    bool importKeys(const String& privateKey, const String& publicKey);
    
    // Retrieve stored keys
    String getPrivateKey();
    String getPublicKey();
    
    // Check if keys are stored
    bool hasPrivateKey();
    bool hasPublicKey();
    bool hasKeys();
    
    // Clear stored keys
    void clearKeys();
    
    // Get raw key data (for encryption operations)
    bool getPrivateKeyRaw(uint8_t* buffer, size_t maxLen);
    bool getPublicKeyRaw(uint8_t* buffer, size_t maxLen);

private:
    Preferences preferences;
    
    // Key storage keys
    static const char* NAMESPACE;
    static const char* PRIVATE_KEY;
    static const char* PUBLIC_KEY;
};

#endif // KEY_MANAGER_H
