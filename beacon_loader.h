#pragma once
#include "beacon_item.h"
#include <storage/storage.h>

// Returns the number of beacons loaded
// beacons_out must be freed by caller if count > 0 (not implemented here for simplicity, we'll use a static max or dynamic array)
// For this simple app, we'll load into a provided array with a max size.
bool beacon_loader_load(const char* path, BeaconItem* beacons, size_t max_count, size_t* count_out);
