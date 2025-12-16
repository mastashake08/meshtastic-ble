#include "proto/meshtastic_protocol.h"
#include <Arduino.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include <pb_common.h>

// Callback for encoding payload data
bool encode_payload_callback(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
    const uint8_t *payload = (const uint8_t*)*arg;
    
    if (!pb_encode_tag_for_field(stream, field)) {
        return false;
    }
    
    // Write the payload - for now, simple string encoding
    return pb_write(stream, payload, strlen((const char*)payload));
}

// Callback for decoding payload data
bool decode_payload_callback(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    uint8_t *buffer = (uint8_t*)*arg;
    
    // Read up to 256 bytes
    size_t len = stream->bytes_left;
    if (len > 256) {
        len = 256;
    }
    
    if (!pb_read(stream, buffer, len)) {
        return false;
    }
    
    buffer[len] = '\0'; // Null terminate
    return true;
}

bool encode_data(pb_ostream_t *stream, const Data *data) {
    // Simple encoding - in a real implementation, this would use proper protobuf fields
    pb_encode_varint(stream, data->portnum);
    
    // Encode payload if present
    if (data->payload.funcs.encode != NULL) {
        void * const *arg_ptr = (void * const *)&data->payload.arg;
        data->payload.funcs.encode(stream, NULL, arg_ptr);
    }
    
    return true;
}

bool decode_data(pb_istream_t *stream, Data *data) {
    // Simple decoding - in a real implementation, this would use proper protobuf fields
    uint64_t portnum;
    if (!pb_decode_varint(stream, &portnum)) {
        return false;
    }
    data->portnum = (PortNum)portnum;
    
    return true;
}

bool encode_mesh_packet(pb_ostream_t *stream, const MeshPacket *packet) {
    // Simplified encoding - encode basic fields
    pb_encode_varint(stream, 1); // Field tag for 'from'
    pb_encode_varint(stream, packet->from);
    
    pb_encode_varint(stream, 2); // Field tag for 'to'
    pb_encode_varint(stream, packet->to);
    
    // Encode decoded data
    pb_encode_varint(stream, 3); // Field tag for decoded
    return encode_data(stream, &packet->decoded);
}

bool decode_mesh_packet(pb_istream_t *stream, MeshPacket *packet) {
    // Simplified decoding
    while (stream->bytes_left) {
        uint64_t tag;
        if (!pb_decode_varint(stream, &tag)) {
            break;
        }
        
        uint32_t field_number = tag >> 3;
        
        switch (field_number) {
            case 1: // from
                pb_decode_varint(stream, (uint64_t*)&packet->from);
                break;
            case 2: // to
                pb_decode_varint(stream, (uint64_t*)&packet->to);
                break;
            case 3: // decoded
                return decode_data(stream, &packet->decoded);
            default:
                pb_skip_field(stream, (pb_wire_type_t)(tag & 0x7));
                break;
        }
    }
    
    return true;
}

bool encode_to_radio(pb_ostream_t *stream, const ToRadio *msg) {
    if (msg->has_packet) {
        // Field 1: packet
        pb_encode_tag(stream, PB_WT_STRING, 1);
        
        // Create submessage
        uint8_t buffer[256];
        pb_ostream_t substream = pb_ostream_from_buffer(buffer, sizeof(buffer));
        
        if (!encode_mesh_packet(&substream, &msg->packet)) {
            return false;
        }
        
        pb_encode_varint(stream, substream.bytes_written);
        return pb_write(stream, buffer, substream.bytes_written);
    }
    
    return true;
}

bool decode_from_radio(pb_istream_t *stream, FromRadio *msg) {
    while (stream->bytes_left) {
        uint64_t tag;
        if (!pb_decode_varint(stream, &tag)) {
            break;
        }
        
        uint32_t field_number = tag >> 3;
        
        switch (field_number) {
            case 1: // id
                pb_decode_varint(stream, (uint64_t*)&msg->id);
                break;
            case 2: { // packet
                pb_istream_t substream;
                if (!pb_make_string_substream(stream, &substream)) {
                    return false;
                }
                
                if (decode_mesh_packet(&substream, &msg->packet)) {
                    msg->has_packet = true;
                }
                
                pb_close_string_substream(stream, &substream);
                break;
            }
            default:
                pb_skip_field(stream, (pb_wire_type_t)(tag & 0x7));
                break;
        }
    }
    
    return true;
}
