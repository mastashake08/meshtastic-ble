#include "MessageHandler.h"

// Payload buffer for encoding/decoding
static uint8_t payload_buffer[256];

MessageHandler::MessageHandler() {
}

bool MessageHandler::begin() {
    messages.clear();
    Serial.println("Message handler initialized with official Meshtastic protobufs");
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
    meshtastic_FromRadio fromRadio = meshtastic_FromRadio_init_zero;
    
    // Decode the message
    if (!decode_from_radio(data, length, &fromRadio)) {
        Serial.println("Decode failed");
        return false;
    }
    
    // Check which variant is present
    if (fromRadio.which_payload_variant == meshtastic_FromRadio_packet_tag) {
        meshtastic_MeshPacket packet = fromRadio.packet;
        
        // Check if packet has decoded data (not encrypted)
        if (packet.which_payload_variant == meshtastic_MeshPacket_decoded_tag) {
            meshtastic_Data decoded = packet.decoded;
            
            // Check if it's a text message
            if (decoded.portnum == meshtastic_PortNum_TEXT_MESSAGE_APP) {
                // Extract text from payload bytes
                String text = String((char*)decoded.payload.bytes);
                text = text.substring(0, decoded.payload.size); // Limit to actual size
                String sender = String(packet.from, HEX);
                
                Serial.printf("Received message from 0x%08X: %s\n", packet.from, text.c_str());
                addMessage(sender, text, false);
                return true;
            }
        }
    }
    
    return false;
}

bool MessageHandler::createTextMessage(const String& text, uint8_t* buffer, size_t* length, size_t maxLen) {
    // Initialize ToRadio message
    meshtastic_ToRadio toRadio = meshtastic_ToRadio_init_zero;
    meshtastic_MeshPacket packet = meshtastic_MeshPacket_init_zero;
    meshtastic_Data msgData = meshtastic_Data_init_zero;
    
    // Set up the Data message
    msgData.portnum = meshtastic_PortNum_TEXT_MESSAGE_APP;
    
    // Copy text to payload
    size_t textLen = text.length();
    if (textLen > sizeof(msgData.payload.bytes) - 1) {
        textLen = sizeof(msgData.payload.bytes) - 1;
    }
    memcpy(msgData.payload.bytes, text.c_str(), textLen);
    msgData.payload.size = textLen;
    
    // Set up the MeshPacket
    packet.which_payload_variant = meshtastic_MeshPacket_decoded_tag;
    packet.decoded = msgData;
    packet.to = 0xFFFFFFFF; // Broadcast address
    packet.want_ack = false;
    packet.hop_limit = 3; // Default hop limit
    packet.priority = meshtastic_MeshPacket_Priority_DEFAULT;
    
    // Set up ToRadio message
    toRadio.which_payload_variant = meshtastic_ToRadio_packet_tag;
    toRadio.packet = packet;
    
    // Encode the message
    size_t bytes_written = 0;
    if (!encode_to_radio(buffer, maxLen, &toRadio, &bytes_written)) {
        Serial.println("Encode failed");
        return false;
    }
    
    *length = bytes_written;
    Serial.printf("Created message, encoded %zu bytes\n", *length);
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
