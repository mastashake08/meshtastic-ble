#include <Arduino.h>
#include "MeshtasticBLE.h"
#include "KeyManager.h"
#include "MessageHandler.h"
#include "DisplayController.h"

// Global objects
MeshtasticBLE bleServer;
KeyManager keyManager;
MessageHandler messageHandler;
DisplayController display;

// State management
enum AppState {
    STATE_INIT,
    STATE_KEY_CHECK,
    STATE_ADVERTISING,
    STATE_CONNECTED
};

AppState currentState = STATE_INIT;
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000; // Update display every second

// BLE callback for received data (from connected client)
void onBLEDataReceived(uint8_t* data, size_t length) {
    Serial.printf("Received %d bytes from client\n", length);
    
    // Process the received data
    if (messageHandler.processReceivedData(data, length)) {
        // New message received - update display
        display.showMessages(messageHandler);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== Meshtastic BLE Server ===");
    
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
    
    // Initialize BLE Server
    if (!bleServer.begin("Meshtastic-ESP32")) {
        Serial.println("Failed to initialize BLE Server!");
        display.updateStatus("Error: BLE");
        return;
    }
    
    // Register BLE data callback
    bleServer.onDataReceived(onBLEDataReceived);
    
    // Check if keys are loaded
    currentState = STATE_KEY_CHECK;
    
    Serial.println("Setup complete!");
}

void loop() {
    unsigned long currentTime = millis();
    static bool keyCheckMessageShown = false;
    
    switch (currentState) {
        case STATE_KEY_CHECK:
            if (keyManager.hasKeys()) {
                Serial.println("Keys found!");
                Serial.printf("Private key: %s\n", keyManager.getPrivateKey().c_str());
                Serial.printf("Public key: %s\n", keyManager.getPublicKey().c_str());
                display.showKeyStatus(true);
                delay(2000);
                currentState = STATE_ADVERTISING;
                keyCheckMessageShown = false;
            } else {
                // Only print instructions once
                if (!keyCheckMessageShown) {
                    Serial.println("\n=== WAITING FOR KEYS ===");
                    Serial.println("No keys found! Please import your Meshtastic keys.");
                    Serial.println("Add your keys via Serial commands:");
                    Serial.println("  IMPORT_PRIVATE:<your_private_key>");
                    Serial.println("  IMPORT_PUBLIC:<your_public_key>");
                    Serial.println("Or type SKIP_KEYS to continue without keys\n");
                    display.showKeyStatus(false);
                    keyCheckMessageShown = true;
                }
                
                // Wait for serial input
                if (Serial.available()) {
                    String cmd = Serial.readStringUntil('\n');
                    cmd.trim();
                    
                    if (cmd.startsWith("IMPORT_PRIVATE:")) {
                        String key = cmd.substring(15);
                        key.trim();
                        if (keyManager.importPrivateKey(key)) {
                            Serial.println("✓ Private key imported");
                        }
                    } else if (cmd.startsWith("IMPORT_PUBLIC:")) {
                        String key = cmd.substring(14);
                        key.trim();
                        if (keyManager.importPublicKey(key)) {
                            Serial.println("✓ Public key imported");
                        }
                    } else if (cmd == "SKIP_KEYS") {
                        Serial.println("Skipping key check...");
                        currentState = STATE_ADVERTISING;
                        keyCheckMessageShown = false;
                    } else {
                        Serial.println("Unknown command: " + cmd);
                    }
                }
                
                // Small delay to prevent tight loop
                delay(100);
            }
            break;
            
        case STATE_ADVERTISING:
            {
                static bool advMessageShown = false;
                if (!advMessageShown) {
                    Serial.println("\n=== BLE SERVER ADVERTISING ===");
                    Serial.println("Waiting for Meshtastic client to connect...");
                    display.showScanning();
                    display.updateStatus("Advertising");
                    advMessageShown = true;
                }
                
                // Check for connection
                if (bleServer.isConnected()) {
                    Serial.println("\n=== CLIENT CONNECTED ===");
                    display.showConnected(bleServer.getDeviceName());
                    currentState = STATE_CONNECTED;
                    advMessageShown = false;
                    delay(1000);
                }
                
                delay(100);
            }
            break;
            
        case STATE_CONNECTED:
            // Check connection status
            if (!bleServer.isConnected()) {
                Serial.println("Client disconnected!");
                display.showDisconnected();
                delay(2000);
                currentState = STATE_ADVERTISING;
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
                        if (bleServer.sendFromRadio(buffer, length)) {
                            Serial.println("Message sent to client!");
                            messageHandler.addSentMessage(msg);
                            display.showMessages(messageHandler);
                        } else {
                            Serial.println("Failed to send message");
                        }
                    }
                }
            }
            break;
            
        default:
            break;
    }
    
    delay(10); // Small delay to prevent tight loops
}