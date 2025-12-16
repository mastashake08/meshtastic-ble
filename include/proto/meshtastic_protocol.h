#ifndef MESHTASTIC_PROTOCOL_H
#define MESHTASTIC_PROTOCOL_H

#include <pb.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include <pb_common.h>

// Meshtastic PortNum enum (simplified version)
typedef enum _PortNum {
    PortNum_UNKNOWN_APP = 0,
    PortNum_TEXT_MESSAGE_APP = 1,
    PortNum_REMOTE_HARDWARE_APP = 2,
    PortNum_POSITION_APP = 3,
    PortNum_NODEINFO_APP = 4,
    PortNum_ROUTING_APP = 5,
    PortNum_ADMIN_APP = 6,
    PortNum_TEXT_MESSAGE_COMPRESSED_APP = 7,
    PortNum_WAYPOINT_APP = 8,
    PortNum_AUDIO_APP = 9,
    PortNum_DETECTION_SENSOR_APP = 10,
    PortNum_REPLY_APP = 32,
    PortNum_IP_TUNNEL_APP = 33,
    PortNum_PAXCOUNTER_APP = 34,
    PortNum_SERIAL_APP = 64,
    PortNum_STORE_FORWARD_APP = 65,
    PortNum_RANGE_TEST_APP = 66,
    PortNum_TELEMETRY_APP = 67,
    PortNum_ZPS_APP = 68,
    PortNum_SIMULATOR_APP = 69,
    PortNum_TRACEROUTE_APP = 70,
    PortNum_NEIGHBORINFO_APP = 71,
    PortNum_ATAK_PLUGIN = 72,
    PortNum_MAP_REPORT_APP = 73,
    PortNum_PRIVATE_APP = 256,
    PortNum_ATAK_FORWARDER = 257,
    PortNum_MAX = 511
} PortNum;

// Data message structure
typedef struct _Data {
    PortNum portnum;
    pb_callback_t payload;
    bool want_response;
    uint32_t dest;
    uint32_t source;
    uint32_t request_id;
    uint32_t reply_id;
    uint32_t emoji;
} Data;

// MeshPacket structure (simplified)
typedef struct _MeshPacket {
    uint32_t from;
    uint32_t to;
    uint8_t channel;
    Data decoded;
    uint32_t id;
    uint32_t rx_time;
    float rx_snr;
    int32_t hop_limit;
    bool want_ack;
    uint8_t priority;
    int32_t rx_rssi;
    bool delayed;
    bool via_mqtt;
    uint32_t hop_start;
} MeshPacket;

// FromRadio structure (simplified)
typedef struct _FromRadio {
    uint32_t id;
    MeshPacket packet;
    uint32_t config_complete_id;
    bool has_packet;
} FromRadio;

// ToRadio structure (simplified)
typedef struct _ToRadio {
    MeshPacket packet;
    bool has_packet;
} ToRadio;

// Callback function prototypes
bool encode_payload_callback(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);
bool decode_payload_callback(pb_istream_t *stream, const pb_field_t *field, void **arg);

// Helper function prototypes
bool encode_data(pb_ostream_t *stream, const Data *data);
bool decode_data(pb_istream_t *stream, Data *data);
bool encode_mesh_packet(pb_ostream_t *stream, const MeshPacket *packet);
bool decode_mesh_packet(pb_istream_t *stream, MeshPacket *packet);
bool encode_to_radio(pb_ostream_t *stream, const ToRadio *msg);
bool decode_from_radio(pb_istream_t *stream, FromRadio *msg);

#endif // MESHTASTIC_PROTOCOL_H
