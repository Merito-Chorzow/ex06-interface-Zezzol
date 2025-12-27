#include <stdio.h>
#include "proto.h"
#include "driver.h"

proto_t proto;
device_state_t dev = { .speed = 0, .mode = MODE_OPEN };
uint8_t tx_buffer[PROTO_MAX_LEN];
size_t tx_len;

// Helper do symulacji wysyłania bajtów do drivera
void simulate_rx(uint8_t *data, size_t len) {
    for(size_t i=0; i<len; i++) {
        int res = proto_parse_byte(&proto, data[i]);
        if (res == 1) {
            printf("[RX] Frame OK. Cmd: 0x%02X\n", proto.current_cmd);
            driver_handle_frame(&proto, &dev, tx_buffer, &tx_len);
            
            // Log response
            printf("   -> [TX] Resp: 0x%02X Len: %lu Payload: ", tx_buffer[2], tx_len);
            for(size_t j=0; j<tx_len; j++) printf("%02X ", tx_buffer[j]);
            printf("\n");
            
        } else if (res == -1) {
            printf("[RX] CRC Error!\n");
        }
    }
}

int main() {
    proto_init(&proto);
    printf("=== Test 1: Funkcjonalny (SET SPEED) ===\n");
    uint8_t frame1[] = { 0xAA, 0x01, 0x01, 50, 0x00 }; // CRC będzie policzone ręcznie lub ignorowane jeśli błędne, tu przykładowe
    // Obliczamy poprawne CRC dla testu: LEN(1)^CMD(1)^PAYLOAD(50) = 1^1^50 = 50 = 0x32
    frame1[4] = 0x32; 
    simulate_rx(frame1, sizeof(frame1));
    printf("   Device State: Speed=%d Mode=%d\n\n", dev.speed, dev.mode);

    printf("=== Test 2: Błędny parametr (Speed > 100) ===\n");
    // STX, LEN=1, CMD=SET, VAL=150, CRC
    uint8_t frame2[] = { 0xAA, 0x01, 0x01, 150, 0x00 }; 
    // CRC: 1^1^150 = 150 = 0x96
    frame2[4] = 0x96;
    simulate_rx(frame2, sizeof(frame2));
    printf("\n");

    printf("=== Test 3: Błąd CRC ===\n");
    uint8_t frame3[] = { 0xAA, 0x01, 0x01, 50, 0xFF }; // Złe CRC
    simulate_rx(frame3, sizeof(frame3));
    printf("\n");

    printf("=== Test 4: Get Stat ===\n");
    // STX, LEN=0, CMD=GET_STAT, CRC
    uint8_t frame4[] = { 0xAA, 0x00, 0x04, 0x00 };
    // CRC: LEN(0)^CMD(4) = 4
    frame4[3] = 0x04;
    simulate_rx(frame4, sizeof(frame4));

    printf("\n=== Telemetria ===\n");
    printf("Valid: %d, CRC Err: %d, Dropped: %d\n", 
           proto.valid_frames, proto.crc_errors, proto.rx_dropped);

    return 0;
}