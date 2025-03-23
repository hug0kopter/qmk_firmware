#include QMK_KEYBOARD_H

enum layer_names {
    _BASE,
    _FN,
};

enum layer_keycodes {
    GUI_LCK,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_tkl_ansi(
        KC_ESC,           KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_PSCR, KC_SLCK,  KC_PAUSE,
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, KC_INS,  KC_HOME,  KC_PGUP,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_DEL,  KC_END,   KC_PGDN,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,          KC_ENT,
        KC_LSFT,          KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,          KC_RSFT,          KC_UP,
        KC_LCTL, KC_LGUI, KC_LALT,                            KC_SPC,                             KC_RALT, MO(_FN), KC_APP,  KC_RCTL, KC_LEFT, KC_DOWN,  KC_RIGHT
    ),

    [_FN] = LAYOUT_tkl_ansi(
        RESET,            KC_MYCM, KC_WHOM, KC_CALC, KC_MSEL, KC_MPRV, KC_MNXT, KC_MPLY, KC_MSTP, KC_MUTE, KC_VOLD, KC_VOLU, _______, _______, _______,  _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, RGB_HUD, RGB_HUI, _______, RGB_MOD, RGB_RMOD, RGB_TOG,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,  _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,          RGB_VAI,
        _______, GUI_LCK, _______,                            _______,                            _______, _______, _______, _______, RGB_SPD, RGB_VAD,  RGB_SPI
    )
};

void keyboard_post_init_user(void) {
    // Set default RGB mode (EEPROM removed)
    rgb_matrix_set_flags(LED_FLAG_ALL);
    rgb_matrix_enable_noeeprom();

    // Optional: Always enable NKRO
    keymap_config.nkro = 1;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case RGB_TOG:
            if (record->event.pressed) {
                if (rgb_matrix_get_flags() == LED_FLAG_ALL) {
                    rgb_matrix_set_flags(LED_FLAG_NONE);
                    rgb_matrix_set_color_all(0, 0, 0);
                } else {
                    rgb_matrix_set_flags(LED_FLAG_ALL);
                    rgb_matrix_enable_noeeprom();
                }
            }
            return false;

        case GUI_LCK:
            if (record->event.pressed) {
                keymap_config.no_gui = !keymap_config.no_gui;
            }
            return false;
    }
    return true;
}

void rgb_matrix_indicators_user(void) {
    if ((rgb_matrix_get_flags() & LED_FLAG_ALL)) {
        if (keymap_config.no_gui) {
            rgb_matrix_set_color(77, 255, 0, 0);  // Red indicator if GUI is locked
        }
    } else {
        if (keymap_config.no_gui) {
            rgb_matrix_set_color(77, 255, 0, 0);
        } else {
            rgb_matrix_set_color(77, 0, 0, 0);
        }
    }
}
