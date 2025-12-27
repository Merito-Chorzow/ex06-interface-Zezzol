#include "proto.h"
#include <string.h>

// Proste CRC-8 (XOR sum) dla celów edukacyjnych
static uint8_t calc_crc(const uint8_t *data, size_t len) {
    uint8_t crc = 0;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
    }
    return crc;
}

void proto_init(proto_t *p) {
    memset(p, 0, sizeof(proto_t));
    p->state = RX_IDLE;
}

size_t proto_build_frame(uint8_t cmd, const uint8_t *payload, size_t len, uint8_t *out_buf) {
    if (len + 4 > PROTO_MAX_LEN) return 0; // STX+LEN+CMD+CRC = 4 overhead
    
    out_buf[0] = PROTO_STX;
    out_buf[1] = (uint8_t)len;
    out_buf[2] = cmd;
    
    if (len > 0 && payload) {
        memcpy(&out_buf[3], payload, len);
    }
    
    // CRC liczone od LEN do końca payloadu (bez STX)
    out_buf[3 + len] = calc_crc(&out_buf[1], len + 2);
    
    return 4 + len;
}

int proto_parse_byte(proto_t *p, uint8_t b) {
    switch (p->state) {
        case RX_IDLE:
            if (b == PROTO_STX) {
                p->state = RX_WAIT_LEN;
                p->rx_idx = 0;
            } else {
                p->rx_dropped++;
            }
            break;
            
        case RX_WAIT_LEN:
            if (b > PROTO_MAX_LEN - 4) { // Zbyt długa ramka
                p->state = RX_IDLE;
                p->broken_frames++;
            } else {
                p->expected_len = b;
                p->rx_buf[p->rx_idx++] = b; // Zapisz LEN do bufora CRC
                p->state = RX_WAIT_CMD;
            }
            break;
            
        case RX_WAIT_CMD:
            p->current_cmd = b;
            p->rx_buf[p->rx_idx++] = b; // Zapisz CMD
            if (p->expected_len == 0) {
                p->state = RX_WAIT_CRC;
            } else {
                p->state = RX_PAYLOAD;
            }
            break;
            
        case RX_PAYLOAD:
            p->rx_buf[p->rx_idx++] = b;
            // rx_idx zawiera LEN(1) + CMD(1) + PAYLOAD(N). Sprawdzamy czy mamy komplet
            if (p->rx_idx >= p->expected_len + 2) {
                p->state = RX_WAIT_CRC;
            }
            break;
            
        case RX_WAIT_CRC:
            {
                uint8_t calc = calc_crc(p->rx_buf, p->rx_idx);
                if (calc == b) {
                    p->valid_frames++;
                    p->state = RX_IDLE;
                    return 1; // Ramka OK
                } else {
                    p->crc_errors++;
                    p->state = RX_IDLE;
                    return -1; // Błąd CRC
                }
            }
            break;
            
        default:
            p->state = RX_IDLE;
            break;
    }
    return 0;
}