#ifndef MESHTASTIC_PROTOCOL_H
#define MESHTASTIC_PROTOCOL_H

// Official Meshtastic Protobuf Definitions
// Generated from https://github.com/meshtastic/protobufs
#include "meshtastic/mesh.pb.h"
#include "meshtastic/portnums.pb.h"
#include "meshtastic/channel.pb.h"
#include "meshtastic/config.pb.h"
#include "meshtastic/telemetry.pb.h"
#include "meshtastic/admin.pb.h"

// Helper function prototypes for encoding/decoding
bool encode_mesh_packet(uint8_t *buffer, size_t buffer_size, const meshtastic_MeshPacket *packet, size_t *bytes_written);
bool decode_mesh_packet(const uint8_t *buffer, size_t buffer_size, meshtastic_MeshPacket *packet);
bool encode_to_radio(uint8_t *buffer, size_t buffer_size, const meshtastic_ToRadio *msg, size_t *bytes_written);
bool decode_from_radio(const uint8_t *buffer, size_t buffer_size, meshtastic_FromRadio *msg);

// Utility functions
void init_mesh_packet(meshtastic_MeshPacket *packet);
void init_to_radio(meshtastic_ToRadio *msg);
void init_from_radio(meshtastic_FromRadio *msg);

#endif // MESHTASTIC_PROTOCOL_H
