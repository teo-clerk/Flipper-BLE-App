#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
// Esta librería es la clave para Momentum/Unleashed
#include <extra_beacon.h>

// --- DATOS DEL REY AMARILLO (AULA M4) ---
// UUID: e2821714-1365-4717-b14d-4845be72ece0
// Major: 2, Minor: 3
// TX Power: -54 dBm (0xCA)

const uint8_t advertising_data[] = {
    0x02, 0x01, 0x06, 
    0x1A, 0xFF, 0x4C, 0x00, 0x02, 0x15,
    0xE2, 0x82, 0x17, 0x14, 0x13, 0x65, 0x47, 0x17,
    0xB1, 0x4D, 0x48, 0x45, 0xBE, 0x72, 0xEC, 0xE0,
    0x00, 0x02, 0x00, 0x03, 0xCA
};

static void draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 10, 10, "Hack Aula M4 Activo");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 10, 25, "UUID: ...72ece0");
    canvas_draw_str(canvas, 10, 38, "Major: 2 | Minor: 3");
    canvas_draw_str(canvas, 10, 50, "Potencia: -54 dBm");
}

static void input_callback(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t aula_m4_app(void* p) {
    UNUSED(p);

    // Configuración del Extra Beacon
    GapExtraBeaconConfig config = {
        .min_adv_interval_ms = 100, // 10Hz (Muy rápido)
        .max_adv_interval_ms = 150,
        .adv_channel_map = 0x07,    // Todos los canales (37, 38, 39)
        .adv_power_level = GapAdvPowerLevel_6dBm, // Potencia Máxima (+6dBm)
        .address_type = GapAddressTypeRandom,
        .address = {0x69, 0x69, 0x69, 0x69, 0x69, 0x69} // MAC aleatoria
    };

    // Iniciar el beacon "Extra"
    if(furi_hal_bt_extra_beacon_is_active()) {
        furi_hal_bt_extra_beacon_stop();
    }
    
    furi_hal_bt_extra_beacon_set_config(&config);
    furi_hal_bt_extra_beacon_set_data(advertising_data, sizeof(advertising_data));
    furi_hal_bt_extra_beacon_start();

    // GUI
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, NULL);
    view_port_input_callback_set(view_port, input_callback, event_queue);
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    InputEvent event;
    while(1) {
        if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
            if(event.key == InputKeyBack && event.type == InputTypeShort) break;
        }
    }

    // Limpieza
    furi_hal_bt_extra_beacon_stop();
    
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_record_close(RECORD_GUI);

    return 0;
}
