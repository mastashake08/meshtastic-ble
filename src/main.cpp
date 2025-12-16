#include <Arduino.h>
#include "MeshtasticBLE.h"
#include "KeyManager.h"
#include "MessageHandler.h"
#include "DisplayController.h"

// PRG button (GPIO0 on ESP32)
#define PRG_BUTTON 0
#define LONG_PRESS_TIME 3000  // 3 seconds
#define MULTI_CLICK_TIMEOUT 500  // Max time between clicks (ms)
#define SHUTDOWN_CLICKS 5  // Number of clicks to shutdown

// Battery monitoring (Heltec WiFi LoRa 32 V3)
#define BATTERY_PIN 1  // ADC1_CH0 for battery voltage
#define VBUS_PIN 37    // GPIO37 for USB power detection
#define ADC_SAMPLES 10
const float ADC_VOLTAGE_DIVIDER = 2.0;  // Voltage divider ratio
const float ADC_REF_VOLTAGE = 3.3;
const int ADC_MAX_VALUE = 4095;  // 12-bit ADC
const float BATTERY_MIN_VOLTAGE = 3.0;  // Min voltage for 0%
const float BATTERY_MAX_VOLTAGE = 4.2;  // Max voltage for 100%

// Global objects
MeshtasticBLE bleServer;
KeyManager keyManager;
MessageHandler messageHandler;
DisplayController display;

// Button state
unsigned long buttonPressTime = 0;
bool buttonWasPressed = false;
bool sleepMode = false;
int clickCount = 0;
unsigned long lastClickTime = 0;

// Battery state
uint8_t batteryLevel = 100;
unsigned long lastBatteryUpdate = 0;
const unsigned long BATTERY_UPDATE_INTERVAL = 30000;  // Update every 30 seconds

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

// Read battery level (0-100%)
uint8_t readBatteryLevel() {
    // Take multiple samples for accuracy
    int adcSum = 0;
    for (int i = 0; i < ADC_SAMPLES; i++) {
        adcSum += analogRead(BATTERY_PIN);
        delay(10);
    }
    float adcAverage = adcSum / (float)ADC_SAMPLES;
    
    // Convert ADC reading to voltage
    float voltage = (adcAverage / ADC_MAX_VALUE) * ADC_REF_VOLTAGE * ADC_VOLTAGE_DIVIDER;
    
    // Debug output
    Serial.printf("Battery ADC: %.0f, Voltage: %.2fV\n", adcAverage, voltage);
    
    // Convert voltage to percentage
    float percentage = ((voltage - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE)) * 100.0;
    
    // Clamp to 0-100%
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;
    
    return (uint8_t)percentage;
}

void shutdownDevice() {
    Serial.println("\n=== SHUTDOWN SEQUENCE ===");
    Serial.println("5 clicks detected - shutting down...");
    
    // Show shutdown message on display
    display.clear();
    display.updateStatus("Shutdown");
    display.showStartup();
    
    delay(2000);
    
    // Turn off display
    display.sleep();
    
    // Stop BLE
    bleServer.stopAdvertising();
    
    Serial.println("Entering deep sleep...");
    delay(100);
    
    // Enter deep sleep (wake on button press)
    esp_sleep_enable_ext0_wakeup((gpio_num_t)PRG_BUTTON, 0);  // Wake on LOW (button press)
    esp_deep_sleep_start();
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Initialize PRG button
    pinMode(PRG_BUTTON, INPUT_PULLUP);
    
    // Initialize battery ADC
    analogReadResolution(12);  // 12-bit resolution
    analogSetAttenuation(ADC_11db);  // For 0-3.3V range
    
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
    
    // Register BLE key command callback
    bleServer.onKeyCommand([](const String& cmd) {
        Serial.printf("BLE Key Command: %s\n", cmd.c_str());
        
        if (cmd.startsWith("IMPORT_PRIVATE:")) {
            String key = cmd.substring(15);
            key.trim();
            if (keyManager.importPrivateKey(key)) {
                Serial.println("✓ Private key imported via BLE");
            } else {
                Serial.println("✗ Failed to import private key via BLE");
            }
        } else if (cmd.startsWith("IMPORT_PUBLIC:")) {
            String key = cmd.substring(14);
            key.trim();
            if (keyManager.importPublicKey(key)) {
                Serial.println("✓ Public key imported via BLE");
            } else {
                Serial.println("✗ Failed to import public key via BLE");
            }
        } else if (cmd == "SKIP_KEYS") {
            Serial.println("Skipping key check via BLE...");
            currentState = STATE_ADVERTISING;
        } else if (cmd == "STATUS") {
            // Send key status back
            if (keyManager.hasKeys()) {
                Serial.println("Keys are loaded");
            } else {
                Serial.println("No keys loaded");
            }
        } else {
            Serial.println("Unknown BLE key command: " + cmd);
        }
    });
    
    // Check if keys are loaded
    currentState = STATE_KEY_CHECK;
    
    Serial.println("Setup complete!");
}

void loop() {
    unsigned long currentTime = millis();
    static bool keyCheckMessageShown = false;
    
    // Handle PRG button for sleep mode toggle
    bool buttonPressed = (digitalRead(PRG_BUTTON) == LOW);
    
    // Update battery level periodically
    if (currentTime - lastBatteryUpdate > BATTERY_UPDATE_INTERVAL) {
        batteryLevel = readBatteryLevel();
        bool charging = (digitalRead(VBUS_PIN) == HIGH);  // HIGH when USB connected
        
        bleServer.updateBatteryLevel(batteryLevel);
        display.updateBatteryLevel(batteryLevel);
        display.updateChargingStatus(charging);
        
        Serial.printf("Battery: %d%% %s\n", batteryLevel, charging ? "(Charging)" : "");
        lastBatteryUpdate = currentTime;
    }
    
    if (buttonPressed && !buttonWasPressed) {
        // Button just pressed
        buttonPressTime = currentTime;
        buttonWasPressed = true;
    } else if (!buttonPressed && buttonWasPressed) {
        // Button just released
        unsigned long pressDuration = currentTime - buttonPressTime;
        
        if (pressDuration >= LONG_PRESS_TIME) {
            // Long press detected - toggle sleep mode
            sleepMode = !sleepMode;
            clickCount = 0;  // Reset click counter
            
            if (sleepMode) {
                Serial.println("Entering sleep mode...");
                display.sleep();
            } else {
                Serial.println("Waking from sleep mode...");
                display.wake();
                // Refresh display based on current state
                switch (currentState) {
                    case STATE_KEY_CHECK:
                        display.showKeyStatus(false);
                        break;
                    case STATE_ADVERTISING:
                        display.showScanning();
                        break;
                    case STATE_CONNECTED:
                        display.showConnected(bleServer.getDeviceName());
                        break;
                    default:
                        break;
                }
            }
        } else {
            // Short press - count clicks
            if (currentTime - lastClickTime < MULTI_CLICK_TIMEOUT) {
                clickCount++;
                Serial.printf("Click %d/%d\n", clickCount, SHUTDOWN_CLICKS);
            } else {
                clickCount = 1;
                Serial.println("Click 1/5");
            }
            lastClickTime = currentTime;
            
            // Check for shutdown sequence
            if (clickCount >= SHUTDOWN_CLICKS) {
                shutdownDevice();
            }
        }
        buttonWasPressed = false;
    }
    
    // Reset click counter if timeout exceeded
    if (clickCount > 0 && currentTime - lastClickTime > MULTI_CLICK_TIMEOUT) {
        clickCount = 0;
    }
    
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