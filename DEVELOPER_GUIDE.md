# Developer Guide: Multi-Beacon Emulator

This guide explains how the Multi-Beacon Emulator app works under the hood. It is designed for beginners who want to understand C programming on the Flipper Zero.

## 1. Architecture Overview

### Why split code into multiple files?

In C programming, as projects grow, keeping everything in one file (`main.c`) becomes messy and hard to manage. We split code to organize it logically:

- **`.h` (Header files)**: These are like a "menu" or "contract". They tell other parts of your program _what_ functions and data types are available, but not _how_ they work.
- **`.c` (Source files)**: These contain the actual logic and implementation. They "do the work".

**In this app:**

1.  **`beacon_item.h`**: Defines what a "Beacon" looks like (data structure).
2.  **`beacon_loader.h`**: Tells the main app "I can load beacons from a file".
3.  **`beacon_loader.c`**: Actually opens the file, reads the text, and turns it into data.
4.  **`main.c`**: The boss. It sets up the UI, talks to the loader, and tells the Flipper to start broadcasting.

---

## 2. Code Walkthrough

### Step 1: Defining the Data (`beacon_item.h`)

Before we can do anything, we need to define what a "Beacon" is.

```c
#pragma once // Prevents this file from being included twice by accident

#include <furi.h> // Standard Flipper definitions

// Constants to avoid "magic numbers" in code
#define MAX_BEACON_NAME_LEN 32
#define BEACON_UUID_LEN 16

// The Blueprint for a Beacon
typedef struct {
    char name[MAX_BEACON_NAME_LEN + 1]; // +1 for the null terminator (\0)
    uint8_t uuid[BEACON_UUID_LEN];      // 16 bytes for the unique ID
    uint16_t major;                     // 2 bytes (0-65535)
    uint16_t minor;                     // 2 bytes (0-65535)
    int8_t rssi_1m;                     // Signal strength at 1 meter
} BeaconItem;
```

### Step 2: The Loader Logic (`beacon_loader.c`)

This file handles the "dirty work" of reading a file from the SD card.

**Key Concept: Parsing**
The file on the SD card is just text (JSON). The Flipper doesn't understand text; it understands numbers and bytes. "Parsing" is the process of converting that text into our `BeaconItem` struct.

```c
// ... includes ...

// Helper: Converts "A1" (text) to 0xA1 (byte)
static void hex_string_to_bytes(const char* hex, uint8_t* bytes, size_t len) {
    // Loops through the string 2 characters at a time
    for(size_t i = 0; i < len; i++) {
        // ... conversion logic ...
    }
}

// The Main Loading Function
bool beacon_loader_load(const char* path, BeaconItem* beacons, size_t max_count, size_t* count_out) {
    // 1. Open access to the Storage system
    Storage* storage = furi_record_open(RECORD_STORAGE);
    Stream* stream = file_stream_alloc(storage);

    // 2. Try to open the file
    if(!file_stream_open(stream, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        // If we fail, clean up and return false
        // ... cleanup code ...
        return false;
    }

    // 3. Read the file line by line
    FuriString* line = furi_string_alloc();

    while(stream_read_line(stream, line) && count < max_count) {
        // We look for specific keys like "name": or "uuid":

        if(furi_string_search_str(line, "\"name\":") != FURI_STRING_FAILURE) {
            // If we find "name":, we extract the text after it
            // ... extraction logic ...
        }
        // ... checks for other fields ...
    }

    // 4. Clean up memory (Very important in C!)
    furi_string_free(line);
    file_stream_close(stream);
    stream_free(stream);
    furi_record_close(RECORD_STORAGE);

    return true; // Success!
}
```

### Step 3: The Application Logic (`main.c`)

This is where the user interface lives. We use a **ViewDispatcher**, which is like a TV channel switcher. It lets us switch between the Menu (Channel 0) and the Emulation Screen (Channel 1).

**The App Struct**
Holds the state of our entire application.

```c
typedef struct {
    ViewDispatcher* view_dispatcher; // The "TV Remote"
    Gui* gui;                        // Access to the screen
    Submenu* submenu;                // The list of beacons
    Widget* widget;                  // The details screen
    BeaconItem beacons[MAX_BEACON_COUNT]; // Array to store loaded beacons
    size_t beacon_count;             // How many we actually loaded
} BeaconApp;
```

**Setting the Beacon Data**
This function tells the Flipper's Bluetooth radio what to broadcast.

```c
static void set_beacon_data(BeaconItem* item) {
    uint8_t adv_data[30];

    // ... constructing the iBeacon packet structure ...
    // 0x4C, 0x00 = Apple ID
    // 0x02, 0x15 = iBeacon Type

    // We use the "Extra Beacon" API which allows background broadcasting
    GapExtraBeaconConfig config = {
        .min_adv_interval_ms = 100, // Broadcast every 100ms
        .adv_power_level = GapAdvPowerLevel_6dBm, // Max power
        // ...
    };

    // Send configuration to the hardware
    furi_hal_bt_extra_beacon_set_config(&config);
    furi_hal_bt_extra_beacon_set_data(adv_data, i);
    furi_hal_bt_extra_beacon_start();
}
```

**The Main Entry Point**

```c
int32_t aula_m4_app(void* p) {
    // 1. Allocate memory for our app
    BeaconApp* app = malloc(sizeof(BeaconApp));

    // 2. Load the beacons from JSON
    if(!beacon_loader_load(BEACONS_PATH, ...)) {
        // If loading fails, create a default one so the app isn't empty
    }

    // 3. Setup the GUI
    app->view_dispatcher = view_dispatcher_alloc();

    // 4. Create the Submenu (The List)
    app->submenu = submenu_alloc();
    // Add each beacon to the menu
    for(size_t i = 0; i < app->beacon_count; i++) {
        submenu_add_item(app->submenu, app->beacons[i].name, ...);
    }

    // 5. Run the ViewDispatcher
    // This function BLOCKS here until the user exits the app
    view_dispatcher_run(app->view_dispatcher);

    // 6. Cleanup
    // When the user exits, we must free all memory to prevent leaks
    stop_beacon_if_active();
    free(app);

    return 0;
}
```

## 3. How it all works together

1.  **Startup**: When you launch the app, `main.c` starts. It immediately calls `beacon_loader.c` to read the JSON file.
2.  **Parsing**: The loader reads the text file, finds the names and UUIDs, and fills the `BeaconItem` array in memory.
3.  **Display**: `main.c` creates a menu item for each beacon found.
4.  **User Action**: When you click a beacon in the menu:
    - The `submenu_callback` function fires.
    - It calls `set_beacon_data()` to update the Bluetooth radio.
    - It switches the view to the `Widget` to show the details.
5.  **Exit**: When you press Back, the app stops the Bluetooth broadcast and cleans up memory.
