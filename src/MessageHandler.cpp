#include "MessageHandler.h"

// Payload buffer for encoding/decoding
static uint8_t payload_buffer[256];

MessageHandler::MessageHandler() {
}

bool MessageHandler::begin() {
    messages.clear();
    Serial.println("Message handler initialized");
    return true;
}

bool MessageHandler::processReceivedData(uint8_t* data, size_t length) {
    if (data == nullptr || length == 0) {
        return false;
    }
    
    // Decode the FromRadio protobuf message
    return decodeFromRadio(data, length);
}

bool MessageHandler::decodeFromRadio(const uint8_t* data, size_t length) {
    // Create a stream for decoding
    pb_istream_t stream = pb_istream_from_buffer(data, length);
    FromRadio fromRadio = {0};
    
    // Decode the message
    if (!decode_from_radio(&stream, &fromRadio)) {
        Serial.println("Decode failed");
        return false;
    }
    
    // Check if it's a packet (contains mesh data)
    if (fromRadio.has_packet) {
        MeshPacket packet = fromRadio.packet;
        Data decoded = packet.decoded;
        
        // Check if it's a text message
        if (decoded.portnum == PortNum_TEXT_MESSAGE_APP) {
            // Set up payload callback for decoding
            decoded.payload.funcs.decode = decode_payload_callback;
            decoded.payload.arg = payload_buffer;
            
            // The payload should now be in payload_buffer
            String text = String((char*)payload_buffer);
            String sender = String(packet.from, HEX);
            
            Serial.printf("Received message from %s: %s\n", sender.c_str(), text.c_str());
            addMessage(sender, text, false);
            return true;
        }
    }
    
    return false;
}

bool MessageHandler::createTextMessage(const String& text, uint8_t* buffer, size_t* length, size_t maxLen) {
    // Copy text to payload buffer
    strncpy((char*)payload_buffer, text.c_str(), sizeof(payload_buffer) - 1);
    payload_buffer[sizeof(payload_buffer) - 1] = '\0';
    
    // Create a text message packet
    MeshPacket packet = {0};
    Data msgData;
    memset(&msgData, 0, sizeof(msgData));
    
    // Set message text
    msgData.portnum = PortNum_TEXT_MESSAGE_APP;
    msgData.payload.funcs.encode = encode_payload_callback;
    msgData.payload.arg = payload_buffer;
    
    // Set packet data
    packet.decoded = msgData;
    packet.to = 0xFFFFFFFF; // Broadcast
    packet.want_ack = false;
    
    // Encode to ToRadio
    ToRadio toRadio = {0};
    toRadio.has_packet = true;
    toRadio.packet = packet;
    
    // Encode the message
    pb_ostream_t outStream = pb_ostream_from_buffer(buffer, maxLen);
    if (!encode_to_radio(&outStream, &toRadio)) {
        Serial.println("Encode failed");
        return false;
    }
    
    *length = outStream.bytes_written;
    Serial.printf("Created message, encoded %d bytes\n", *length);
    return true;
}

void MessageHandler::addMessage(const String& sender, const String& text, bool isOwn) {
    Message msg;
    msg.sender = sender;
    msg.text = text;
    msg.timestamp = millis();
    msg.isOwn = isOwn;
    
    messages.push_back(msg);
    
    // Keep only last MAX_MESSAGES
    while (messages.size() > MAX_MESSAGES) {
        messages.erase(messages.begin());
    }
}

void MessageHandler::addSentMessage(const String& text) {
    addMessage("You", text, true);
}

int MessageHandler::getMessageCount() {
    return messages.size();
}

Message MessageHandler::getMessage(int index) {
    if (index >= 0 && index < messages.size()) {
        return messages[index];
    }
    return Message();
}

Message MessageHandler::getLatestMessage() {
    if (messages.size() > 0) {
        return messages[messages.size() - 1];
    }
    return Message();
}

void MessageHandler::clearMessages() {
    messages.clear();
    Serial.println("Message history cleared");
}
