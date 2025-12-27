#include "driver.h"
#include <stdio.h>

void driver_handle_frame(proto_t *p, device_state_t *dev, uint8_t *tx_buf, size_t *tx_len) {
    // rx_buf: [LEN, CMD, PAYLOAD...]
    // Payload zaczyna się od indeksu 2
    uint8_t *payload = &p->rx_buf[2];
    uint8_t resp_payload[32];
    size_t resp_len = 0;
    uint8_t resp_cmd = RESP_ACK;

    switch (p->current_cmd) {
        case CMD_SET_SPEED:
            if (p->expected_len != 1) {
                resp_cmd = RESP_NACK;
                resp_payload[0] = NACK_LEN;
                resp_len = 1;
            } else {
                uint8_t val = payload[0];
                if (val > 100) {
                    resp_cmd = RESP_NACK;
                    resp_payload[0] = NACK_PARAM; // Poza zakresem
                    resp_len = 1;
                } else {
                    dev->speed = val;
                }
            }
            break;

        case CMD_MODE:
            if (p->expected_len != 1) {
                resp_cmd = RESP_NACK;
                resp_payload[0] = NACK_LEN;
                resp_len = 1;
            } else {
                uint8_t m = payload[0];
                if (m > 2) { // 0=OPEN, 1=CLOSED, 2=SAFE
                    resp_cmd = RESP_NACK;
                    resp_payload[0] = NACK_PARAM;
                    resp_len = 1;
                } else {
                    dev->mode = (drive_mode_t)m;
                }
            }
            break;
            
        case CMD_STOP:
            dev->mode = MODE_SAFE;
            dev->speed = 0;
            break;

        case CMD_GET_STAT:
            resp_cmd = RESP_STAT;
            // Prosta serializacja statystyk (speed, mode, valid_frames, crc_errors)
            resp_payload[0] = dev->speed;
            resp_payload[1] = (uint8_t)dev->mode;
            // Uproszczone: wysyłamy tylko młodsze bajty liczników dla przykładu
            resp_payload[2] = (uint8_t)(p->valid_frames & 0xFF);
            resp_payload[3] = (uint8_t)(p->crc_errors & 0xFF);
            resp_len = 4;
            break;

        default:
            resp_cmd = RESP_NACK;
            resp_payload[0] = NACK_UNKNOWN;
            resp_len = 1;
            break;
    }
    
    *tx_len = proto_build_frame(resp_cmd, resp_payload, resp_len, tx_buf);
}