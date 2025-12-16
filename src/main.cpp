#include <Arduino.h>
#include "MeshtasticBLE.h"
#include "KeyManager.h"
#include "MessageHandler.h"
#include "DisplayController.h"

// Global objects
MeshtasticBLE bleClient;
KeyManager keyManager;
MessageHandler messageHandler;
DisplayController display;

// State management
enum AppState {
    STATE_INIT,
    STATE_KEY_CHECK,
    STATE_SCANNING,
    STATE_CONNECTING,
    STATE_CONNECTED,
    STATE_DISCONNECTED
};

AppState currentState = STATE_INIT;
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000; // Update display every second

// BLE callback for received data
void onBLEDataReceived(uint8_t* data, size_t length) {
    Serial.printf("Received %d bytes from BLE\n", length);
    
    // Process the received data
    if (messageHandler.processReceivedData(data, length)) {
        // New message received - update display
        display.showMessages(messageHandler);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== Meshtastic BLE Controller ===");
    
    // Initialize display
    if (!display.begin()) {
        Serial.println("Failed to initialize display!");
        return;
    }
    
    delay(2000); // Show startup screen
    
    // Initialize key manager
    if (!keyManager.begin()) {
        Serial.println("Failed to initialize key manager!");
        display.updateStatus("Error: Keys");
        return;
    }
    
    // Initialize message handler
    messageHandler.begin();
    
    // Initialize BLE
    if (!bleClient.begin()) {
        Serial.println("Failed to initialize BLE!");
        display.updateStatus("Error: BLE");
        return;
    }
    
    // Register BLE data callback
    bleClient.onDataReceived(onBLEDataReceived);
    
    // Check if keys are loaded
    currentState = STATE_KEY_CHECK;
    
    Serial.println("Setup complete!");
}

void loop() {
    unsigned long currentTime = millis();
    
    switch (currentState) {
        case STATE_KEY_CHECK:
            if (keyManager.hasKeys()) {
                Serial.println("Keys found!");
                Serial.printf("Private key: %s\n", keyManager.getPrivateKey().c_str());
                Serial.printf("Public key: %s\n", keyManager.getPublicKey().c_str());
                display.showKeyStatus(true);
                delay(2000);
                currentState = STATE_SCANNING;
            } else {
                Serial.println("No keys found! Please import your Meshtastic keys.");
                Serial.println("Add your keys via Serial commands:");
                Serial.println("  IMPORT_PRIVATE:<your_private_key>");
                Serial.println("  IMPORT_PUBLIC:<your_public_key>");
                display.showKeyStatus(false);
                
                // Wait for serial input
                if (Serial.available()) {
                    String cmd = Serial.readStringUntil('\n');
                    cmd.trim();
                    
                    if (cmd.startsWith("IMPORT_PRIVATE:")) {
                        String key = cmd.substring(15);
                        keyManager.importPrivateKey(key);
                    } else if (cmd.startsWith("IMPORT_PUBLIC:")) {
                        String key = cmd.substring(14);
                        keyManager.importPublicKey(key);
                    } else if (cmd == "SKIP_KEYS") {
                        // For testing without keys
                        Serial.println("Skipping key check...");
                        currentState = STATE_SCANNING;
                    }
                }
            }
            break;
            
        case STATE_SCANNING:
            Serial.println("Scanning for Meshtastic devices...");
            display.showScanning();
            
            if (bleClient.scanForDevices(5)) {
                Serial.printf("Found %d device(s)\n", bleClient.getDeviceCount());
                for (int i = 0; i < bleClient.getDeviceCount(); i++) {
                    Serial.printf("  [%d] %s - %s\n", 
                        i, 
                        bleClient.getDeviceName(i).c_str(),
                        bleClient.getDeviceAddress(i).toString().c_str());
                }
                currentState = STATE_CONNECTING;
            } else {
                Serial.println("No devices found, retrying...");
                delay(2000);
            }
            break;
            
        case STATE_CONNECTING:
            Serial.println("Connecting to first device...");
            display.showConnecting(bleClient.getDeviceName(0));
            
            if (bleClient.connectToFirst()) {
                Serial.println("Connected successfully!");
                display.showConnected(bleClient.getDeviceName(0));
                currentState = STATE_CONNECTED;
                delay(2000);
            } else {
                Serial.println("Connection failed, retrying scan...");
                delay(2000);
                currentState = STATE_SCANNING;
            }
            break;
            
        case STATE_CONNECTED:
            // Check connection status
            if (!bleClient.isConnected()) {
                Serial.println("Disconnected!");
                display.showDisconnected();
                currentState = STATE_DISCONNECTED;
                break;
            }
            
            // Update display periodically
            if (currentTime - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL) {
                display.showMessages(messageHandler);
                lastDisplayUpdate = currentTime;
            }
            
            // Check for serial commands to send messages
            if (Serial.available()) {
                String msg = Serial.readStringUntil('\n');
                msg.trim();
                
                if (msg.length() > 0) {
                    Serial.printf("Sending message: %s\n", msg.c_str());
                    
                    uint8_t buffer[256];
                    size_t length;
                    
                    if (messageHandler.createTextMessage(msg, buffer, &length, sizeof(buffer))) {
                        if (bleClient.sendToRadio(buffer, length)) {
                            Serial.println("Message sent!");
                            messageHandler.addSentMessage(msg);
                            display.showMessages(messageHandler);
                        } else {
                            Serial.println("Failed to send message");
                        }
                    }
                }
            }
            break;
            
        case STATE_DISCONNECTED:
            delay(3000);
            Serial.println("Attempting to reconnect...");
            currentState = STATE_SCANNING;
            break;
            
        default:
            break;
    }
    
    delay(10); // Small delay to prevent tight loops
}