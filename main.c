#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <extra_beacon.h>
#include "beacon_loader.h"

#define MAX_BEACONS 20
#define BEACONS_PATH "/ext/apps_data/aula_m4_beacon/beacons.json"

typedef enum {
    BeaconAppViewSubmenu,
    BeaconAppViewWidget,
} BeaconAppView;

typedef struct {
    ViewDispatcher* view_dispatcher;
    Gui* gui;
    Submenu* submenu;
    Widget* widget;
    BeaconItem beacons[MAX_BEACONS];
    size_t beacon_count;
    int selected_beacon_index;
} BeaconApp;

// Helper to construct advertising data
static void set_beacon_data(BeaconItem* item) {
    uint8_t adv_data[30];
    uint8_t i = 0;

    // Flags
    adv_data[i++] = 0x02;
    adv_data[i++] = 0x01;
    adv_data[i++] = 0x06;

    // Manufacturer Data (Apple iBeacon)
    adv_data[i++] = 0x1A; // Length
    adv_data[i++] = 0xFF; // Type
    adv_data[i++] = 0x4C; // Company ID (Apple)
    adv_data[i++] = 0x00;
    adv_data[i++] = 0x02; // Beacon Type
    adv_data[i++] = 0x15; // Length of remaining

    // UUID
    memcpy(&adv_data[i], item->uuid, 16);
    i += 16;

    // Major
    adv_data[i++] = (item->major >> 8) & 0xFF;
    adv_data[i++] = item->major & 0xFF;

    // Minor
    adv_data[i++] = (item->minor >> 8) & 0xFF;
    adv_data[i++] = item->minor & 0xFF;

    // TX Power
    adv_data[i++] = item->rssi_1m;

    // Configure Extra Beacon
    GapExtraBeaconConfig config = {
        .min_adv_interval_ms = 100,
        .max_adv_interval_ms = 150,
        .adv_channel_map = 0x07,
        .adv_power_level = GapAdvPowerLevel_6dBm,
        .address_type = GapAddressTypeRandom,
        .address = {0x69, 0x69, 0x69, 0x69, 0x69, 0x69}
    };

    if(furi_hal_bt_extra_beacon_is_active()) {
        furi_hal_bt_extra_beacon_stop();
    }
    
    furi_hal_bt_extra_beacon_set_config(&config);
    furi_hal_bt_extra_beacon_set_data(adv_data, i);
    furi_hal_bt_extra_beacon_start();
}

static void submenu_callback(void* context, uint32_t index) {
    BeaconApp* app = context;
    if(index < app->beacon_count) {
        app->selected_beacon_index = index;
        BeaconItem* item = &app->beacons[index];
        
        // Start Beacon
        set_beacon_data(item);

        // Update Widget
        widget_reset(app->widget);
        widget_add_string_element(app->widget, 64, 5, AlignCenter, AlignTop, FontPrimary, "Emulating Beacon");
        
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Name: %s", item->name);
        widget_add_string_element(app->widget, 64, 20, AlignCenter, AlignTop, FontSecondary, buffer);

        snprintf(buffer, sizeof(buffer), "Major: %d | Minor: %d", item->major, item->minor);
        widget_add_string_element(app->widget, 64, 32, AlignCenter, AlignTop, FontSecondary, buffer);
        
        // Show partial UUID
        snprintf(buffer, sizeof(buffer), "UUID: ...%02X%02X%02X", item->uuid[13], item->uuid[14], item->uuid[15]);
        widget_add_string_element(app->widget, 64, 44, AlignCenter, AlignTop, FontSecondary, buffer);

        view_dispatcher_switch_to_view(app->view_dispatcher, BeaconAppViewWidget);
    }
}

static uint32_t view_dispatcher_navigation_event_callback(void* context) {
    UNUSED(context);
    return BeaconAppViewSubmenu;
}

static uint32_t view_dispatcher_exit_callback(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

static void stop_beacon_if_active() {
    if(furi_hal_bt_extra_beacon_is_active()) {
        furi_hal_bt_extra_beacon_stop();
    }
}

int32_t aula_m4_app(void* p) {
    UNUSED(p);
    BeaconApp* app = malloc(sizeof(BeaconApp));

    // Load Beacons
    if(!beacon_loader_load(BEACONS_PATH, app->beacons, MAX_BEACONS, &app->beacon_count)) {
        // Fallback if no file: Add default M4 beacon
        app->beacon_count = 1;
        strlcpy(app->beacons[0].name, "Aula M4 (Default)", sizeof(app->beacons[0].name));
        // e2821714-1365-4717-b14d-4845be72ece0
        uint8_t default_uuid[] = {0xE2, 0x82, 0x17, 0x14, 0x13, 0x65, 0x47, 0x17, 0xB1, 0x4D, 0x48, 0x45, 0xBE, 0x72, 0xEC, 0xE0};
        memcpy(app->beacons[0].uuid, default_uuid, 16);
        app->beacons[0].major = 2;
        app->beacons[0].minor = 3;
        app->beacons[0].rssi_1m = -54;
    }

    // GUI Setup
    app->gui = furi_record_open(RECORD_GUI);
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Submenu
    app->submenu = submenu_alloc();
    submenu_set_header(app->submenu, "Select Beacon");
    for(size_t i = 0; i < app->beacon_count; i++) {
        submenu_add_item(app->submenu, app->beacons[i].name, i, submenu_callback, app);
    }
    view_dispatcher_add_view(app->view_dispatcher, BeaconAppViewSubmenu, submenu_get_view(app->submenu));

    // Widget
    app->widget = widget_alloc();
    view_dispatcher_add_view(app->view_dispatcher, BeaconAppViewWidget, widget_get_view(app->widget));

    // Navigation
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, NULL);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, view_dispatcher_navigation_event_callback);
    
    // Start at Submenu
    view_dispatcher_switch_to_view(app->view_dispatcher, BeaconAppViewSubmenu);

    // Run
    view_dispatcher_run(app->view_dispatcher);

    // Cleanup
    stop_beacon_if_active();
    view_dispatcher_remove_view(app->view_dispatcher, BeaconAppViewSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, BeaconAppViewWidget);
    submenu_free(app->submenu);
    widget_free(app->widget);
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);
    free(app);

    return 0;
}
