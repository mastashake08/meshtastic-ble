#include "DisplayController.h"

DisplayController::DisplayController() 
    : u8g2(U8G2_R0, DISPLAY_RST, DISPLAY_SCL, DISPLAY_SDA) {
    currentStatus = "Starting...";
}

bool DisplayController::begin() {
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setFontRefHeightExtendedText();
    u8g2.setDrawColor(1);
    u8g2.setFontPosTop();
    u8g2.setFontDirection(0);
    
    showStartup();
    Serial.println("Display initialized");
    return true;
}

void DisplayController::drawHeader() {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 0, currentStatus.c_str());
    u8g2.drawLine(0, 10, 128, 10);
}

void DisplayController::clear() {
    u8g2.clearBuffer();
    u8g2.sendBuffer();
}

void DisplayController::showStartup() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB10_tr);
    
    int w = u8g2.getStrWidth("Meshtastic");
    u8g2.drawStr((128 - w) / 2, 20, "Meshtastic");
    
    u8g2.setFont(u8g2_font_6x10_tf);
    w = u8g2.getStrWidth("BLE Controller");
    u8g2.drawStr((128 - w) / 2, 40, "BLE Controller");
    
    u8g2.sendBuffer();
}

void DisplayController::showScanning() {
    currentStatus = "Scanning...";
    u8g2.clearBuffer();
    drawHeader();
    
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(10, 30, "Searching for");
    u8g2.drawStr(10, 42, "Meshtastic devices");
    
    u8g2.sendBuffer();
}

void DisplayController::showConnecting(const String& deviceName) {
    currentStatus = "Connecting...";
    u8g2.clearBuffer();
    drawHeader();
    
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(10, 20, "Connecting to:");
    u8g2.drawStr(10, 32, deviceName.c_str());
    
    u8g2.sendBuffer();
}

void DisplayController::showConnected(const String& deviceName) {
    currentStatus = "Connected";
    u8g2.clearBuffer();
    drawHeader();
    
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(10, 20, "Connected!");
    u8g2.drawStr(10, 32, deviceName.c_str());
    u8g2.drawStr(10, 50, "Waiting for msgs...");
    
    u8g2.sendBuffer();
}

void DisplayController::showDisconnected() {
    currentStatus = "Disconnected";
    u8g2.clearBuffer();
    drawHeader();
    
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(10, 30, "Disconnected");
    u8g2.drawStr(10, 42, "Retrying...");
    
    u8g2.sendBuffer();
}

void DisplayController::showKeyStatus(bool hasKeys) {
    u8g2.clearBuffer();
    drawHeader();
    
    u8g2.setFont(u8g2_font_6x10_tf);
    if (hasKeys) {
        u8g2.drawStr(10, 20, "Keys: Loaded");
        u8g2.drawStr(10, 32, "Ready to connect");
    } else {
        u8g2.drawStr(10, 20, "Keys: Not loaded");
        u8g2.drawStr(10, 32, "Import keys first");
    }
    
    u8g2.sendBuffer();
}

void DisplayController::drawMessage(int y, const String& sender, const String& text, bool isOwn) {
    u8g2.setFont(u8g2_font_5x7_tf);
    
    // Draw sender
    String prefix = isOwn ? "You: " : sender + ": ";
    u8g2.drawStr(2, y, prefix.c_str());
    
    // Draw message text (truncate if needed)
    String displayText = text;
    if (displayText.length() > 20) {
        displayText = displayText.substring(0, 17) + "...";
    }
    u8g2.drawStr(2, y + 9, displayText.c_str());
}

void DisplayController::showMessages(MessageHandler& messageHandler) {
    u8g2.clearBuffer();
    drawHeader();
    
    int msgCount = messageHandler.getMessageCount();
    if (msgCount == 0) {
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(10, 30, "No messages yet");
        u8g2.sendBuffer();
        return;
    }
    
    // Show last 2-3 messages
    int y = 14;
    int startIdx = max(0, msgCount - 2);
    
    for (int i = startIdx; i < msgCount && y < 60; i++) {
        Message msg = messageHandler.getMessage(i);
        drawMessage(y, msg.sender, msg.text, msg.isOwn);
        y += 20;
    }
    
    u8g2.sendBuffer();
}

void DisplayController::showMessage(const Message& msg) {
    u8g2.clearBuffer();
    drawHeader();
    drawMessage(14, msg.sender, msg.text, msg.isOwn);
    u8g2.sendBuffer();
}

void DisplayController::showLatestMessage(const Message& msg) {
    // Show latest message briefly then return to message list
    showMessage(msg);
}

void DisplayController::updateStatus(const String& status) {
    currentStatus = status;
}

int DisplayController::wrapText(const String& text, int maxWidth, String* lines, int maxLines) {
    // Simple text wrapping helper (can be enhanced)
    int lineCount = 0;
    String remaining = text;
    
    while (remaining.length() > 0 && lineCount < maxLines) {
        if (remaining.length() <= 20) {
            lines[lineCount++] = remaining;
            break;
        }
        
        lines[lineCount++] = remaining.substring(0, 20);
        remaining = remaining.substring(20);
    }
    
    return lineCount;
}
