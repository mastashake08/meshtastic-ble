# Meshtastic BLE Controller

ESP32-based BLE server for Meshtastic mesh networking devices. This project enables wireless control and monitoring of Meshtastic devices through Bluetooth Low Energy, supporting message transmission, device configuration, and status monitoring with multi-device key management.

**Target Hardware:** Heltec WiFi Kit 32 V3 (ESP32-S3 with built-in OLED display)

## Features

- ✅ **BLE Server Mode** - Acts as a Meshtastic-compatible BLE peripheral
- ✅ **Official Meshtastic Protobufs** - Full protocol compatibility using official definitions
- ✅ **Text Messaging** - Send and receive text messages over the mesh
- ✅ **Encryption Support** - Import and use your existing Meshtastic device keys
- ✅ **OLED Display** - View messages and connection status on built-in screen
- ✅ **Multi-Device Support** - Store and manage multiple device keys

## Hardware Requirements

- **Heltec WiFi Kit 32 V3** (ESP32-S3 based board)
  - Built-in SSD1306 OLED display (128x64)
  - Integrated BLE capabilities
  - USB-C connection for programming

## Software Prerequisites

### Required Tools

1. **PlatformIO** - Install via VS Code extension or CLI:
   ```bash
   # Install PlatformIO Core CLI
   pip install platformio
   ```

2. **Python 3.10-3.13** - PlatformIO requires a compatible Python version:
   ```bash
   # Check your Python version
   python3 --version
   
   # Install Python 3.13 on macOS (if needed)
   brew install python@3.13
   ```

3. **Protocol Buffers Compiler** (already included in setup):
   ```bash
   # On macOS
   brew install protobuf nanopb
   ```

## Installation

1. **Clone the Repository**
   ```bash
   git clone https://github.com/mastashake08/meshtastic-ble.git
   cd meshtastic-ble
   ```

2. **Verify Protobuf Files**
   
   The official Meshtastic protobuf files are already generated in `include/proto/meshtastic/`. If you need to regenerate them:
   ```bash
   cd /tmp
   git clone https://github.com/meshtastic/protobufs.git
   cd protobufs
   protoc --nanopb_opt=--cpp-descriptors --nanopb_out=/path/to/meshtastic-ble/include/proto -I. meshtastic/*.proto
   
   # Rename .pb.c to .pb.cpp for C++ compilation
   cd /path/to/meshtastic-ble/include/proto/meshtastic
   for f in *.pb.c; do mv "$f" "${f%.c}.cpp"; done
   ```

## Building the Project

### Using PlatformIO CLI

```bash
# Build the project
pio run

# Or if you have Python version issues, use the direct path:
~/.platformio/penv/bin/platformio run
```

### Using PlatformIO in VS Code

1. Open the project folder in VS Code
2. Install the PlatformIO IDE extension
3. Click the **Build** button (✓) in the PlatformIO toolbar

### Build Output

Successful build shows:
```
RAM:   [=         ]  10.8% (used 35340 bytes from 327680 bytes)
Flash: [==        ]  21.4% (used 714735 bytes from 3342336 bytes)
```

## Uploading to Heltec WiFi Kit 32 V3

1. **Connect the Board** via USB-C cable

2. **Upload the Firmware**
   ```bash
   # Upload using PlatformIO
   pio run --target upload
   
   # Or with direct path:
   ~/.platformio/penv/bin/platformio run --target upload
   ```

3. **Monitor Serial Output**
   ```bash
   pio device monitor
   # Or use the Serial Monitor in VS Code (PlatformIO toolbar)
   ```

## Configuration & Usage

### 1. Import Your Meshtastic Keys

On first boot, the device will wait for you to import your existing Meshtastic encryption keys.

**Via Serial Monitor (115200 baud):**

```
IMPORT_PRIVATE:<your-32-byte-hex-private-key>
IMPORT_PUBLIC:<your-32-byte-hex-public-key>
```

Example:
```
IMPORT_PRIVATE:0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF
IMPORT_PUBLIC:FEDCBA9876543210FEDCBA9876543210FEDCBA9876543210FEDCBA9876543210
```

**To skip key import (for testing):**
```
SKIP_KEYS
```

### 2. Connect from a Meshtastic Device

Once keys are imported, your ESP32 will:
- Start advertising as a BLE Meshtastic peripheral
- Display "Advertising..." on the OLED screen
- Accept connections from Meshtastic apps or devices

### 3. View Messages

Messages appear on the built-in OLED display showing:
- Connection status
- Sender information
- Message content
- Timestamp

### Device States

The device operates in the following states:

| State | Description | OLED Display |
|-------|-------------|--------------|
| `STATE_INIT` | Initializing hardware | "Initializing..." |
| `STATE_KEY_CHECK` | Waiting for key import | "Import keys via serial" |
| `STATE_ADVERTISING` | Waiting for connection | "Advertising..." |
| `STATE_CONNECTED` | Active BLE connection | "Connected" + messages |

## Project Structure

```
meshtastic-ble/
├── include/
│   ├── proto/
│   │   ├── meshtastic/          # Official Meshtastic protobuf files
│   │   │   ├── mesh.pb.cpp/h    # Core mesh packet definitions
│   │   │   ├── portnums.pb.cpp/h
│   │   │   └── ... (23 files total)
│   │   └── meshtastic_protocol.h # Protocol wrapper
│   ├── MeshtasticBLE.h          # BLE GATT server
│   ├── KeyManager.h             # NVS key storage
│   ├── MessageHandler.h         # Protobuf encoding/decoding
│   └── DisplayController.h      # OLED display management
├── src/
│   ├── main.cpp                 # Main application
│   ├── MeshtasticBLE.cpp
│   ├── KeyManager.cpp
│   ├── MessageHandler.cpp
│   ├── DisplayController.cpp
│   └── meshtastic_protocol.cpp
├── platformio.ini               # Build configuration
└── README.MD
```

## Troubleshooting

### Python Version Error
```
ERROR: Python version must be between 3.10 and 3.13
```
**Solution:** Use the PlatformIO Python environment directly:
```bash
~/.platformio/penv/bin/platformio run
```

### Build Errors with Protobuf Files
Ensure all `.pb.c` files are renamed to `.pb.cpp`:
```bash
cd include/proto/meshtastic
for f in *.pb.c; do mv "$f" "${f%.c}.cpp"; done
```

### Cannot Connect to Device
1. Verify keys are imported correctly (check serial output)
2. Ensure Bluetooth is enabled on client device
3. Check that device is in `STATE_ADVERTISING` mode
4. Restart the ESP32 if needed

### Display Not Working
The Heltec WiFi Kit 32 V3 OLED uses specific pins:
- SDA: GPIO 17
- SCL: GPIO 18
- RST: GPIO 21

These are configured in `DisplayController.cpp`.

## Serial Commands Reference

| Command | Description | Example |
|---------|-------------|---------|
| `IMPORT_PRIVATE:<key>` | Import 32-byte hex private key | `IMPORT_PRIVATE:0123...CDEF` |
| `IMPORT_PUBLIC:<key>` | Import 32-byte hex public key | `IMPORT_PUBLIC:FEDC...3210` |
| `SKIP_KEYS` | Skip key import (testing only) | `SKIP_KEYS` |

## Development

### Adding New Features

The project uses the **official Meshtastic protobuf definitions**, which support:
- Text messages (implemented)
- Position updates (ready to implement)
- Telemetry data (ready to implement)
- Admin commands (ready to implement)
- And many more...

See `include/proto/meshtastic/portnums.pb.h` for all available message types.

### Regenerating Protobufs

If Meshtastic updates their protocol:
```bash
cd /tmp
git clone https://github.com/meshtastic/protobufs.git
cd protobufs
protoc --nanopb_opt=--cpp-descriptors --nanopb_out=/path/to/project/include/proto -I. meshtastic/*.proto
```

## License

This project uses official Meshtastic protobuf definitions licensed under GPL v3.

## Contributing

Pull requests welcome! Please ensure:
- Code compiles without warnings
- Serial output is clear and helpful
- OLED display updates are tested
- Memory usage remains reasonable (< 50% RAM/Flash)

## Acknowledgments

- [Meshtastic Project](https://meshtastic.org) - Official protobuf definitions
- [Nanopb](https://jpa.kapsi.fi/nanopb/) - Protobuf implementation for embedded systems
- [U8g2](https://github.com/olikraus/u8g2) - OLED display library
