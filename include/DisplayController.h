#ifndef DISPLAY_CONTROLLER_H
#define DISPLAY_CONTROLLER_H

#include <Arduino.h>
#include <U8g2lib.h>
#include "MessageHandler.h"

// Heltec WiFi Kit 32 V3 display pins
#define DISPLAY_SDA 17
#define DISPLAY_SCL 18
#define DISPLAY_RST 21

class DisplayController {
public:
    DisplayController();
    
    // Initialize display
    bool begin();
    
    // Display states
    void showStartup();
    void showScanning();
    void showConnecting(const String& deviceName);
    void showConnected(const String& deviceName);
    void showDisconnected();
    void showKeyStatus(bool hasKeys);
    
    // Message display
    void showMessages(MessageHandler& messageHandler);
    void showMessage(const Message& msg);
    void showLatestMessage(const Message& msg);
    
    // Status line (top of screen)
    void updateStatus(const String& status);
    
    // Clear display
    void clear();

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
    String currentStatus;
    
    void drawHeader();
    void drawMessage(int y, const String& sender, const String& text, bool isOwn);
    int wrapText(const String& text, int maxWidth, String* lines, int maxLines);
};

#endif // DISPLAY_CONTROLLER_H
