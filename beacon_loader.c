#include "beacon_loader.h"
#include <furi.h>
#include <toolbox/stream/file_stream.h>
#include <furi_hal.h>

// Helper to parse hex string to bytes
static void hex_string_to_bytes(const char* hex, uint8_t* bytes, size_t len) {
    for(size_t i = 0; i < len; i++) {
        char c1 = hex[i * 2];
        char c2 = hex[i * 2 + 1];
        uint8_t v1 = 0;
        uint8_t v2 = 0;
        
        if(c1 >= '0' && c1 <= '9') v1 = c1 - '0';
        else if(c1 >= 'A' && c1 <= 'F') v1 = c1 - 'A' + 10;
        else if(c1 >= 'a' && c1 <= 'f') v1 = c1 - 'a' + 10;

        if(c2 >= '0' && c2 <= '9') v2 = c2 - '0';
        else if(c2 >= 'A' && c2 <= 'F') v2 = c2 - 'A' + 10;
        else if(c2 >= 'a' && c2 <= 'f') v2 = c2 - 'a' + 10;

        bytes[i] = (v1 << 4) | v2;
    }
}

// Very simple JSON parser tailored for this specific structure
// Assumes well-formed JSON and specific order or keys present
bool beacon_loader_load(const char* path, BeaconItem* beacons, size_t max_count, size_t* count_out) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    Stream* stream = file_stream_alloc(storage);
    
    if(!file_stream_open(stream, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E("BeaconLoader", "Failed to open file: %s", path);
        file_stream_close(stream);
        stream_free(stream);
        furi_record_close(RECORD_STORAGE);
        return false;
    }

    FuriString* line = furi_string_alloc();
    size_t count = 0;
    bool in_object = false;
    BeaconItem current_item = {0};

    // Simple line-by-line parsing (assuming formatted JSON for simplicity, or we handle tokens)
    // Actually, let's read the whole file into a buffer or read char by char for better robustness
    // But for simplicity on Flipper, let's read line by line and look for keys
    
    while(stream_read_line(stream, line) && count < max_count) {
        // Trim whitespace
        furi_string_trim(line);
        
        if(furi_string_start_with_str(line, "{")) {
            in_object = true;
            memset(&current_item, 0, sizeof(BeaconItem));
        } else if(furi_string_start_with_str(line, "}")) {
            if(in_object) {
                beacons[count] = current_item;
                count++;
                in_object = false;
            }
        } else if(in_object) {
            // Parse fields
            if(furi_string_search_str(line, "\"name\":") != FURI_STRING_FAILURE) {
                size_t start = furi_string_search_str(line, ":") + 1;
                size_t end = furi_string_search_rchar(line, '"');
                size_t first_quote = furi_string_search_char(line, '"', start);
                
                if(first_quote != FURI_STRING_FAILURE && end != FURI_STRING_FAILURE && end > first_quote) {
                    FuriString* val = furi_string_alloc();
                    furi_string_set_n(val, line, first_quote + 1, end - first_quote - 1);
                    strlcpy(current_item.name, furi_string_get_cstr(val), sizeof(current_item.name));
                    furi_string_free(val);
                }
            } else if(furi_string_search_str(line, "\"uuid\":") != FURI_STRING_FAILURE) {
                size_t start = furi_string_search_str(line, ":") + 1;
                size_t end = furi_string_search_rchar(line, '"');
                size_t first_quote = furi_string_search_char(line, '"', start);
                
                if(first_quote != FURI_STRING_FAILURE && end != FURI_STRING_FAILURE && end > first_quote) {
                    FuriString* val = furi_string_alloc();
                    furi_string_set_n(val, line, first_quote + 1, end - first_quote - 1);
                    // Remove dashes if any
                    // For simplicity assume clean hex or handle it
                    // Let's just parse the first 32 hex chars found
                    const char* uuid_str = furi_string_get_cstr(val);
                    char clean_hex[33] = {0};
                    int c_idx = 0;
                    for(int i=0; uuid_str[i] != '\0' && c_idx < 32; i++) {
                        if(isalnum((unsigned char)uuid_str[i])) {
                            clean_hex[c_idx++] = uuid_str[i];
                        }
                    }
                    hex_string_to_bytes(clean_hex, current_item.uuid, 16);
                    furi_string_free(val);
                }
            } else if(furi_string_search_str(line, "\"major\":") != FURI_STRING_FAILURE) {
                size_t start = furi_string_search_str(line, ":") + 1;
                size_t end = furi_string_search_char(line, ',', start);
                if (end == FURI_STRING_FAILURE) end = furi_string_size(line);
                
                FuriString* val = furi_string_alloc();
                furi_string_set_n(val, line, start, end - start);
                furi_string_trim(val);
                current_item.major = (uint16_t)strtol(furi_string_get_cstr(val), NULL, 10);
                furi_string_free(val);
            } else if(furi_string_search_str(line, "\"minor\":") != FURI_STRING_FAILURE) {
                size_t start = furi_string_search_str(line, ":") + 1;
                size_t end = furi_string_search_char(line, ',', start);
                if (end == FURI_STRING_FAILURE) end = furi_string_size(line);
                
                FuriString* val = furi_string_alloc();
                furi_string_set_n(val, line, start, end - start);
                furi_string_trim(val);
                current_item.minor = (uint16_t)strtol(furi_string_get_cstr(val), NULL, 10);
                furi_string_free(val);
            } else if(furi_string_search_str(line, "\"rssi_1m\":") != FURI_STRING_FAILURE) {
                size_t start = furi_string_search_str(line, ":") + 1;
                size_t end = furi_string_search_char(line, ',', start);
                if (end == FURI_STRING_FAILURE) end = furi_string_size(line);
                
                FuriString* val = furi_string_alloc();
                furi_string_set_n(val, line, start, end - start);
                furi_string_trim(val);
                current_item.rssi_1m = (int8_t)strtol(furi_string_get_cstr(val), NULL, 10);
                furi_string_free(val);
            }
        }
    }

    *count_out = count;
    furi_string_free(line);
    file_stream_close(stream);
    stream_free(stream);
    furi_record_close(RECORD_STORAGE);
    return true;
}
