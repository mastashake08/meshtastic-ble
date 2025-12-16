# Meshtastic BLE Controller - AI Coding Agent Instructions

## Project Overview
This is an ESP32-based BLE controller for Meshtastic mesh networking devices. The project enables wireless control and monitoring of Meshtastic devices through Bluetooth Low Energy, supporting message transmission, device configuration, and status monitoring with multi-device key management.

**Target Hardware:** Heltec WiFi Kit 32 V3 (ESP32-based board with built-in display)
**Build System:** PlatformIO with Arduino framework

## Architecture

### Hardware Platform
- **Board:** Heltec WiFi Kit 32 V3
- **MCU:** ESP32 with integrated BLE
- **Display:** Built-in OLED (board-specific)
- **Framework:** Arduino for ESP32

### Core Components (To Be Implemented)
- **BLE Client:** Connects to Meshtastic devices using ESP32 BLE stack
- **Protocol Handler:** Meshtastic protocol implementation for BLE communication
- **Key Manager:** Stores and manages device encryption keys
- **UI Controller:** Display interface for device interaction
- **Message Queue:** Manages bidirectional message flow

## Development Workflow

### Building and Uploading
```bash
# Build the project
pio run

# Upload to connected ESP32
pio run --target upload

# Monitor serial output
pio device monitor

# Clean build artifacts
pio run --target clean
```

### PlatformIO Configuration
- Platform uses custom ESP32 build: `pioarduino/platform-espressif32` (stable release)
- All configuration in [platformio.ini](../platformio.ini)
- Libraries should be added to `lib_deps` in platformio.ini
- Board-specific pins and features defined by `heltec_wifi_kit_32_V3` board config

## Project Conventions

### Code Organization
- `src/main.cpp` - Main application entry point with setup() and loop()
- `include/` - Header files for custom modules
- `lib/` - Custom libraries (PlatformIO auto-links these)
- `test/` - Unit tests (PlatformIO Unity framework)

### Meshtastic Integration Requirements
When implementing BLE functionality:
- Use ESP32 BLE library (`BLEDevice.h`, `BLEClient.h`)
- Meshtastic uses standard BLE GATT services/characteristics
- Implement proper connection handling with reconnection logic
- Handle device keys for encrypted mesh communication
- Support scanning for multiple nearby Meshtastic devices

### Hardware-Specific Considerations
- Heltec WiFi Kit 32 V3 has built-in OLED display (use appropriate library)
- Onboard buttons for user input (check board pinout)
- Battery management capabilities built-in
- WiFi capabilities available but not primary focus (BLE first)

## Critical Patterns

### Arduino ESP32 Setup Pattern
```cpp
void setup() {
  Serial.begin(115200);
  // Initialize BLE, display, and peripherals
  // Connect to last known device or show scan UI
}

void loop() {
  // Handle BLE events
  // Update display
  // Process user input
  delay(10); // Avoid tight loops on ESP32
}
```

### ESP32 Memory Management
- Avoid large stack allocations (limited RAM)
- Use `PROGMEM` for constant strings/data
- Monitor heap usage: `ESP.getFreeHeap()`
- Prefer static allocation over dynamic where possible

## Dependencies to Add
When implementing features, add these to platformio.ini `lib_deps`:
- Meshtastic protocol library or implement manually
- BLE libraries (built into ESP32 Arduino core)
- Display driver for Heltec board (U8g2 or similar)
- Preferences library for persistent storage (ESP32 built-in)

## Testing & Debugging
- Use `Serial.printf()` for debug output
- PlatformIO Unified Debugger available for this board
- Test with actual Meshtastic devices (required for BLE protocol validation)
- Monitor heap and handle memory fragmentation

## Common Tasks
- **Adding BLE Service:** Scan for service UUIDs, connect, and subscribe to characteristics
- **Persistent Storage:** Use ESP32 Preferences library for storing device keys
- **Display Updates:** Update OLED in loop() but rate-limit to avoid flicker
- **Multi-device Support:** Store multiple device credentials in NVS (Non-Volatile Storage)
