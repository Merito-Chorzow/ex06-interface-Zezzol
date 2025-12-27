#ifndef DRIVER_H
#define DRIVER_H

#include "proto.h"

typedef enum { MODE_OPEN, MODE_CLOSED, MODE_SAFE } drive_mode_t;

typedef struct {
    uint8_t speed;
    drive_mode_t mode;
} device_state_t;

// Obsługa odebranej ramki: wykonanie komendy i przygotowanie odpowiedzi
void driver_handle_frame(proto_t *p, device_state_t *dev, uint8_t *tx_buf, size_t *tx_len);

#endif