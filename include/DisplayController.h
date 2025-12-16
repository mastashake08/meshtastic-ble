#ifndef DISPLAY_CONTROLLER_H
#define DISPLAY_CONTROLLER_H

#include <Arduino.h>
#include <U8g2lib.h>
#include "MessageHandler.h"

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
    
    // Sleep mode
    void sleep();
    void wake();
    bool isSleeping();
    
    // Clear display
    void clear();

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
    String currentStatus;
    bool displayEnabled;
    bool sleeping;
    
    void drawHeader();
    void drawMessage(int y, const String& sender, const String& text, bool isOwn);
    int wrapText(const String& text, int maxWidth, String* lines, int maxLines);
};

#endif // DISPLAY_CONTROLLER_H
