#ifndef PROTO_H
#define PROTO_H

#include <stdint.h>
#include <stddef.h>

// Konfiguracja protokołu
#define PROTO_STX       0xAA
#define PROTO_MAX_LEN   64

// Komendy
typedef enum {
    CMD_SET_SPEED = 0x01,
    CMD_MODE      = 0x02,
    CMD_STOP      = 0x03,
    CMD_GET_STAT  = 0x04,
    
    // Odpowiedzi
    RESP_ACK      = 0x80,
    RESP_NACK     = 0x81,
    RESP_STAT     = 0x82
} proto_cmd_t;

// Powody NACK
typedef enum {
    NACK_CRC       = 0x01,
    NACK_UNKNOWN   = 0x02,
    NACK_PARAM     = 0x03,
    NACK_LEN       = 0x04
} nack_reason_t;

// Stan maszyny stanów odbiornika (FSM)
typedef enum {
    RX_IDLE,
    RX_WAIT_LEN,
    RX_WAIT_CMD,
    RX_PAYLOAD,
    RX_WAIT_CRC
} rx_state_t;

// Struktura drivera protokołu
typedef struct {
    // Stan FSM
    rx_state_t state;
    uint8_t rx_buf[PROTO_MAX_LEN];
    uint8_t rx_idx;
    uint8_t expected_len;
    uint8_t current_cmd;
    
    // Telemetria
    uint32_t rx_dropped;
    uint32_t broken_frames;
    uint32_t crc_errors;
    uint32_t valid_frames;
    uint32_t last_cmd_latency; // symulowane ticki
} proto_t;

void proto_init(proto_t *p);
// Przetwarza jeden bajt wejściowy. Zwraca 1 jeśli odebrano pełną ramkę.
int proto_parse_byte(proto_t *p, uint8_t b);
// Buduje ramkę do wysłania w buforze out_buf
size_t proto_build_frame(uint8_t cmd, const uint8_t *payload, size_t len, uint8_t *out_buf);

#endif