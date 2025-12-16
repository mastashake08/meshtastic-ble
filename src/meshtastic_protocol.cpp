#include "proto/meshtastic_protocol.h"
#include <Arduino.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include <pb_common.h>
#include <string.h>

// Initialize a MeshPacket with default values
void init_mesh_packet(meshtastic_MeshPacket *packet) {
    memset(packet, 0, sizeof(meshtastic_MeshPacket));
    packet->hop_limit = 3; // Default hop limit
    packet->priority = meshtastic_MeshPacket_Priority_DEFAULT;
}

// Initialize ToRadio message
void init_to_radio(meshtastic_ToRadio *msg) {
    memset(msg, 0, sizeof(meshtastic_ToRadio));
}

// Initialize FromRadio message
void init_from_radio(meshtastic_FromRadio *msg) {
    memset(msg, 0, sizeof(meshtastic_FromRadio));
}

// Encode MeshPacket to buffer
bool encode_mesh_packet(uint8_t *buffer, size_t buffer_size, const meshtastic_MeshPacket *packet, size_t *bytes_written) {
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);
    
    bool status = pb_encode(&stream, meshtastic_MeshPacket_fields, packet);
    
    if (status && bytes_written) {
        *bytes_written = stream.bytes_written;
    }
    
    return status;
}

// Decode MeshPacket from buffer
bool decode_mesh_packet(const uint8_t *buffer, size_t buffer_size, meshtastic_MeshPacket *packet) {
    pb_istream_t stream = pb_istream_from_buffer(buffer, buffer_size);
    return pb_decode(&stream, meshtastic_MeshPacket_fields, packet);
}

// Encode ToRadio message to buffer
bool encode_to_radio(uint8_t *buffer, size_t buffer_size, const meshtastic_ToRadio *msg, size_t *bytes_written) {
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);
    
    bool status = pb_encode(&stream, meshtastic_ToRadio_fields, msg);
    
    if (status && bytes_written) {
        *bytes_written = stream.bytes_written;
    }
    
    return status;
}

// Decode FromRadio message from buffer
bool decode_from_radio(const uint8_t *buffer, size_t buffer_size, meshtastic_FromRadio *msg) {
    pb_istream_t stream = pb_istream_from_buffer(buffer, buffer_size);
    return pb_decode(&stream, meshtastic_FromRadio_fields, msg);
}
