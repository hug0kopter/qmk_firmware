#include QMK_KEYBOARD_H
#include "eeprom.h"
#include <stdio.h> // For snprintf()

// Define layers
enum layers {
    _NUMPAD,
    _RGBCTRL,
    _MEDIA,
    _SAFARI,
    _LAYER_COUNT
};

// Define custom keycodes
enum custom_keycodes {
    NEXT_LAYER = SAFE_RANGE,
    COLOR_NEXT,
    COLOR_PREV,
    SONAR_VOLUP,
    SONAR_VOLD
};

// EEPROM address for saving HSV
#define RGB_EEPROM_ADDR 0x10

// Color preset list
#define COLOR_PRESET_COUNT 6
HSV color_presets[COLOR_PRESET_COUNT] = {
    { 0,   255, 255 }, // Red
    { 32,  255, 255 }, // Orange
    { 85,  255, 255 }, // Green
    { 170, 255, 255 }, // Blue
    { 200, 255, 255 }, // Indigo
    { 250, 255, 255 }  // Purple
};

uint8_t current_layer = _NUMPAD;
uint8_t previous_layer = _NUMPAD;
uint8_t color_index = 0;

// Keymaps
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_NUMPAD] = LAYOUT(
        NEXT_LAYER,
        KC_1,   KC_2, KC_3,
        KC_4,   KC_5, KC_6,
        KC_7,   KC_8, KC_9,
        KC_BSPC, KC_0, KC_ENT
    ),
    [_RGBCTRL] = LAYOUT(
        NEXT_LAYER,
        RGB_TOG, RGB_MODE_FORWARD, RGB_MODE_REVERSE,
        COLOR_PREV, RGB_VAI, COLOR_NEXT,
        _______, RGB_VAD, _______,
        _______, _______, TO(_NUMPAD)
    ),
    [_MEDIA] = LAYOUT(
        NEXT_LAYER,
        KC_MPRV, KC_MPLY, KC_MNXT,
        SONAR_VOLD, KC_MUTE, SONAR_VOLUP,
        _______, _______, _______,
        _______, _______, TO(_NUMPAD)
    ),
    [_SAFARI] = LAYOUT(
        NEXT_LAYER,
        LCTL(KC_TAB), LCTL(LSFT(KC_TAB)), KC_MPLY,
        LGUI(KC_T),    LGUI(KC_W),        KC_MNXT,
        SGUI(KC_BSLS), SGUI(KC_T),        SGUI(KC_T),
        _______,       _______,           TO(_NUMPAD)
    )
};

// Mode names lookup table
const char *rgb_mode_names[] = {
    "None", "Solid Color", "Alphas Mods", "Gradient Up Down", "Gradient Left Right", "Breathing", "Band Sat",
    "Band Val", "Band Pinwheel Sat", "Band Pinwheel Val", "Band Spiral Sat", "Band Spiral Val", "Cycle All",
    "Cycle Left Right", "Cycle Up Down", "Rainbow Chevron", "Cycle Out In", "Cycle Out In Dual", "Cycle Pinwheel",
    "Cycle Spiral", "Dual Beacon", "Rainbow Beacon", "Rainbow Pinwheels", "Raindrops", "Jellybean Raindrops",
    "Hue Breathing", "Hue Pendulum", "Hue Wave", "Pixel Rain", "Pixel Flow", "Pixel Fractal", "Typing Heatmap",
    "Digital Rain", "Solid Reactive Simple", "Solid Reactive", "Solid Reactive Wide", "Solid Reactive Multiwide",
    "Solid Reactive Cross", "Solid Reactive Multicross", "Solid Reactive Nexus", "Solid Reactive Multinexus",
    "Splash", "Multisplash", "Solid Splash", "Solid Multisplash"
};

// Helper for color name display
const char* get_color_name(uint8_t index) {
    switch (index) {
        case 0: return "Red";
        case 1: return "Orange";
        case 2: return "Green";
        case 3: return "Blue";
        case 4: return "Indigo";
        case 5: return "Purple";
        default: return "Unknown";
    }
}

// Layer cycling and macro handling
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        switch (keycode) {
            case NEXT_LAYER:
                current_layer++;
                if (current_layer >= _LAYER_COUNT) current_layer = 0;
                layer_move(current_layer);
                return false;

            case COLOR_NEXT:
                color_index = (color_index + 1) % COLOR_PRESET_COUNT;
                rgb_matrix_mode(RGB_MATRIX_SOLID_COLOR);
                rgb_matrix_sethsv(color_presets[color_index].h, color_presets[color_index].s, color_presets[color_index].v);
                return false;

            case COLOR_PREV:
                color_index = (color_index == 0) ? COLOR_PRESET_COUNT - 1 : color_index - 1;
                rgb_matrix_mode(RGB_MATRIX_SOLID_COLOR);
                rgb_matrix_sethsv(color_presets[color_index].h, color_presets[color_index].s, color_presets[color_index].v);
                return false;

            case SONAR_VOLUP:
                tap_code16(LCTL(KC_F8));  // Sonar-specific Volume Up
                return false;

            case SONAR_VOLD:
                tap_code16(LCTL(KC_F7));  // Sonar-specific Volume Down
                return false;
        }
    }
    return true;
}

// Encoder = full system volume
bool encoder_update_user(uint8_t index, bool clockwise) {
    tap_code(clockwise ? KC_VOLU : KC_VOLD);
    return true;
}

// EEPROM Save
void save_rgb_to_eeprom(void) {
    HSV hsv = rgb_matrix_get_hsv();
    eeprom_update_byte((uint8_t*)RGB_EEPROM_ADDR,     hsv.h);
    eeprom_update_byte((uint8_t*)(RGB_EEPROM_ADDR+1), hsv.s);
    eeprom_update_byte((uint8_t*)(RGB_EEPROM_ADDR+2), hsv.v);
}

// EEPROM Load
void load_rgb_from_eeprom(void) {
    HSV hsv;
    hsv.h = eeprom_read_byte((uint8_t*)RGB_EEPROM_ADDR);
    hsv.s = eeprom_read_byte((uint8_t*)(RGB_EEPROM_ADDR+1));
    hsv.v = eeprom_read_byte((uint8_t*)(RGB_EEPROM_ADDR+2));
    rgb_matrix_sethsv(hsv.h, hsv.s, hsv.v);
    rgb_matrix_mode(RGB_MATRIX_SOLID_COLOR);
}

// OLED clear and RGB color save on layer switch
layer_state_t layer_state_set_user(layer_state_t state) {
    uint8_t new_layer = get_highest_layer(state);
    if (new_layer != previous_layer) {
        oled_clear();
    }
    if (previous_layer == _RGBCTRL && new_layer != _RGBCTRL) {
        save_rgb_to_eeprom();
    }
    previous_layer = new_layer;
    return state;
}

// OLED display
#ifdef OLED_ENABLE
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_0;
}

bool oled_task_user(void) {
    oled_write_ln_P(PSTR("Layer:"), false);
    switch (get_highest_layer(layer_state)) {
        case _NUMPAD:
            oled_write_ln_P(PSTR("NUMPAD         "), false);
            oled_write_ln_P(PSTR("---------------"), false);
            oled_write_ln_P(PSTR(" 1  |  2  |  3 "), false);
            oled_write_ln_P(PSTR(" 4  |  5  |  6 "), false);
            oled_write_ln_P(PSTR(" 7  |  8  |  9 "), false);
            oled_write_ln_P(PSTR("Del |  0  | En "), false);
            oled_write_ln_P(PSTR("---------------"), false);
            break;
        case _RGBCTRL:
            oled_write_ln_P(PSTR("RGB CTRL"), false);
            oled_write_ln_P(PSTR("Mode:"), false);
            uint8_t mode = rgb_matrix_get_mode();
            if (mode < sizeof(rgb_mode_names) / sizeof(rgb_mode_names[0])) {
                oled_write_ln(rgb_mode_names[mode], false);
                if (mode == RGB_MATRIX_SOLID_COLOR) {
                    oled_write_P(PSTR("Color: "), false);
                    oled_write_ln(get_color_name(color_index), false);
                } else {
                    oled_write_ln_P(PSTR(" "), false);
                }
            } else {
                oled_write_ln_P(PSTR("Unknown"), false);
            }
            break;
        case _MEDIA:
            oled_write_ln_P(PSTR("MEDIA"), false);
            oled_write_ln_P(PSTR("------------------"), false);
            oled_write_ln_P(PSTR("[<]   [Pause]   [>]"), false);
            oled_write_ln_P(PSTR("[V-]  [Mute]    [V+]"), false);
            oled_write_ln_P(PSTR("------------------"), false);
            break;
        case _SAFARI:
            oled_write_ln_P(PSTR("SAFARI"), false);
            oled_write_ln_P(PSTR("------------------"), false);
            oled_write_ln_P(PSTR("[Tab<] [Play] [Tab>]"), false);
            oled_write_ln_P(PSTR("[Prev] [+Tab] [Next]"), false);
            oled_write_ln_P(PSTR("       [Close]      "), false);
            oled_write_ln_P(PSTR("------------------"), false);
            break;
        default:
            oled_write_ln_P(PSTR("Unknown"), false);
    }
    return false;
}
#endif

// Load saved color on boot
void keyboard_post_init_user(void) {
    load_rgb_from_eeprom();
}
