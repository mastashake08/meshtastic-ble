#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <Arduino.h>
#include <vector>
#include <pb_encode.h>
#include <pb_decode.h>
#include "proto/meshtastic_protocol.h"

#define MAX_MESSAGES 20

struct Message {
    String sender;
    String text;
    uint32_t timestamp;
    bool isOwn;  // Message sent by us vs received
};

class MessageHandler {
public:
    MessageHandler();
    
    // Initialize message handler
    bool begin();
    
    // Process received Meshtastic data
    bool processReceivedData(uint8_t* data, size_t length);
    
    // Create and encode a text message to send
    bool createTextMessage(const String& text, uint8_t* buffer, size_t* length, size_t maxLen);
    
    // Get messages
    int getMessageCount();
    Message getMessage(int index);
    Message getLatestMessage();
    
    // Clear message history
    void clearMessages();
    
    // Add a sent message to history
    void addSentMessage(const String& text);

private:
    std::vector<Message> messages;
    
    void addMessage(const String& sender, const String& text, bool isOwn = false);
    bool decodeFromRadio(const uint8_t* data, size_t length);
};

#endif // MESSAGE_HANDLER_H
