#pragma once
#include <furi.h>

#define MAX_BEACON_NAME_LEN 32
#define BEACON_UUID_LEN 16

typedef struct {
    char name[MAX_BEACON_NAME_LEN + 1];
    uint8_t uuid[BEACON_UUID_LEN];
    uint16_t major;
    uint16_t minor;
    int8_t rssi_1m; // RSSI at 1 meter (Tx Power)
} BeaconItem;
