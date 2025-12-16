# Meshtastic BLE Controller - Quick Start

## Importing Your Keys

After uploading the firmware to your Heltec WiFi Kit 32 V3, you'll need to import your existing Meshtastic keys via the Serial Monitor.

### Steps:

1. **Open Serial Monitor** (115200 baud)
   ```bash
   pio device monitor
   ```

2. **Import Your Private Key**
   ```
   IMPORT_PRIVATE:<your_private_key_here>
   ```

3. **Import Your Public Key**
   ```
   IMPORT_PUBLIC:<your_public_key_here>
   ```

4. **Restart the Device**
   - The device will automatically start scanning for Meshtastic devices
   - It will connect to the first device found
   - Messages will appear on the OLED display

### Finding Your Keys

Your Meshtastic keys are typically stored in your existing Meshtastic device settings:
- **Android App:** Settings → Security → Channel Keys
- **iOS App:** Settings → Advanced → Channel Details
- **Python CLI:** `meshtastic --info` shows channel details
- **Web Interface:** Configuration → Channels

The keys are usually Base64 or hex-encoded strings.

## Sending Messages

Once connected, you can send messages via Serial Monitor:
```
Hello mesh network!
```

Just type your message and press Enter. It will be sent to your Meshtastic network and appear on the display.

## Building and Uploading

```bash
# Build
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

## Display Information

The OLED screen shows:
- **Top line:** Current status (Connected, Scanning, etc.)
- **Main area:** Last 2-3 messages with sender names
- Messages scroll automatically as new ones arrive

## Troubleshooting

**No devices found:**
- Ensure your Meshtastic device has Bluetooth enabled
- Make sure the device is powered on and within range
- Try increasing scan duration in the code

**Connection fails:**
- Verify your keys are correct
- Check that Bluetooth pairing isn't required on your device
- Restart both the ESP32 and Meshtastic device

**Messages not appearing:**
- Verify you're on the same Meshtastic channel
- Check that encryption keys match your network
- Monitor Serial output for decoding errors

**For testing without keys:**
Send `SKIP_KEYS` via Serial Monitor to bypass key checking (messages won't decrypt properly but BLE connection will work).
