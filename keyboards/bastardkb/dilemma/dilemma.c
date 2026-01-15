/**
 * Copyright 2020 Christopher Courtney <drashna@live.com> (@drashna)
 * Copyright 2021 Quentin LEBASTARD <qlebastard@gmail.com>
 * Copyright 2022 Charly Delay <charly@codesink.dev> (@0xcharly)
 * Copyright 2023 casuanoob <casuanoob@hotmail.com> (@casuanoob)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Publicw License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dilemma.h"

#ifdef CONSOLE_ENABLE
#    include "print.h"
#endif // CONSOLE_ENABLE

#ifndef LAYER_INDICATOR_BRIGHTNESS_INC
#    define LAYER_INDICATOR_BRIGHTNESS_INC 22
#endif

#ifndef CAPSLOCK_INDICATOR_BRIGHTNESS_INC
#    define CAPSLOCK_INDICATOR_BRIGHTNESS_INC 76
#endif

#ifdef POINTING_DEVICE_ENABLE
#    ifndef DILEMMA_MINIMUM_DEFAULT_DPI
#        define DILEMMA_MINIMUM_DEFAULT_DPI 400
#    endif // DILEMMA_MINIMUM_DEFAULT_DPI

#    ifndef DILEMMA_DEFAULT_DPI_CONFIG_STEP
#        define DILEMMA_DEFAULT_DPI_CONFIG_STEP 200
#    endif // DILEMMA_DEFAULT_DPI_CONFIG_STEP

#    ifndef DILEMMA_MINIMUM_SNIPING_DPI
#        define DILEMMA_MINIMUM_SNIPING_DPI 200
#    endif // DILEMMA_MINIMUM_SNIPING_DPI

#    ifndef DILEMMA_SNIPING_DPI_CONFIG_STEP
#        define DILEMMA_SNIPING_DPI_CONFIG_STEP 100
#    endif // DILEMMA_SNIPING_DPI_CONFIG_STEP

// Fixed DPI for drag-scroll.
#    ifndef DILEMMA_DRAGSCROLL_DPI
#        define DILEMMA_DRAGSCROLL_DPI 100
#    endif // DILEMMA_DRAGSCROLL_DPI

#    ifndef DILEMMA_DRAGSCROLL_BUFFER_SIZE
#        define DILEMMA_DRAGSCROLL_BUFFER_SIZE 6
#    endif // !DILEMMA_DRAGSCROLL_BUFFER_SIZE

// Forward declarations for static functions
static void apply_via_dilemma_config(void);
static void read_via_dilemma_config(void);
static void write_via_dilemma_config(void);

typedef union {
    uint8_t raw;
    struct {
        uint8_t pointer_default_dpi : 4; // 16 steps available.
        uint8_t pointer_sniping_dpi : 2; // 4 steps available.
        bool    is_dragscroll_enabled : 1;
        bool    is_sniping_enabled : 1;
    } __attribute__((packed));
} dilemma_config_t;

static dilemma_config_t g_dilemma_config = {0};

// VIA custom configuration
via_dilemma_config_t g_via_dilemma_config = {0};

// Global scroll divisor tracking variables
// These track the current scroll divisors based on mode (drag-scroll vs two-finger)
static uint16_t g_current_scroll_x_divisor = 8;  // Default matching VIA config
static uint16_t g_current_scroll_y_divisor = 8;  // Default matching VIA config

// Track current gesture events from Azoteq hardware
static struct {
    bool single_tap;           // GESTURE_EVENTS_0 bit
    bool two_finger_tap;       // GESTURE_EVENTS_1 bit
    bool swipe_x_plus;         // Swipe right
    bool swipe_x_minus;        // Swipe left
    bool swipe_y_plus;         // Swipe up
    bool swipe_y_minus;        // Swipe down
    bool press_and_hold;       // GESTURE_EVENTS_0 bit
    bool scroll;               // GESTURE_EVENTS_1 bit (already tracked via two-finger scroll)
    bool zoom;                 // GESTURE_EVENTS_1 bit
} g_gesture_state = {0};

/**
 * \brief Set the value of `config` from EEPROM.
 *
 * Note that `is_dragscroll_enabled` and `is_sniping_enabled` are purposefully
 * ignored since we do not want to persist this state to memory.  In practice,
 * this state is always written to maximize write-performances.  Therefore, we
 * explicitly set them to `false` in this function.
 */
static void read_dilemma_config_from_eeprom(dilemma_config_t *config) {
    config->raw                   = eeconfig_read_kb() & 0xff;
    config->is_dragscroll_enabled = false;
    config->is_sniping_enabled    = false;
}

/**
 * \brief Save the value of `config` to eeprom.
 *
 * Note that all values are written verbatim, including whether drag-scroll
 * and/or sniper mode are enabled.  `read_dilemma_config_from_eeprom(…)`
 * resets these 2 values to `false` since it does not make sense to persist
 * these across reboots of the board.
 */
static void write_dilemma_config_to_eeprom(dilemma_config_t *config) {
    eeconfig_update_kb(config->raw);
}

/** \brief Return the current value of the pointer's default DPI. */
static uint16_t get_pointer_default_dpi(dilemma_config_t *config) {
    return (uint16_t)config->pointer_default_dpi * DILEMMA_DEFAULT_DPI_CONFIG_STEP + DILEMMA_MINIMUM_DEFAULT_DPI;
}

/** \brief Return the current value of the pointer's sniper-mode DPI. */
static uint16_t get_pointer_sniping_dpi(dilemma_config_t *config) {
    return (uint16_t)config->pointer_sniping_dpi * DILEMMA_SNIPING_DPI_CONFIG_STEP + DILEMMA_MINIMUM_SNIPING_DPI;
}

/** \brief Set the appropriate DPI for the input config. */
static void maybe_update_pointing_device_cpi(dilemma_config_t *config) {
    if (config->is_dragscroll_enabled) {
        pointing_device_set_cpi(DILEMMA_DRAGSCROLL_DPI);
    } else if (config->is_sniping_enabled) {
        pointing_device_set_cpi(get_pointer_sniping_dpi(config));
    } else {
        pointing_device_set_cpi(get_pointer_default_dpi(config));
    }
}

/**
 * \brief Update the pointer's default DPI to the next or previous step.
 *
 * Increases the DPI value if `forward` is `true`, decreases it otherwise.
 * The increment/decrement steps are equal to DILEMMA_DEFAULT_DPI_CONFIG_STEP.
 */
static void step_pointer_default_dpi(dilemma_config_t *config, bool forward) {
    config->pointer_default_dpi += forward ? 1 : -1;
    maybe_update_pointing_device_cpi(config);
}

/**
 * \brief Update the pointer's sniper-mode DPI to the next or previous step.
 *
 * Increases the DPI value if `forward` is `true`, decreases it otherwise.
 * The increment/decrement steps are equal to DILEMMA_SNIPING_DPI_CONFIG_STEP.
 */
static void step_pointer_sniping_dpi(dilemma_config_t *config, bool forward) {
    config->pointer_sniping_dpi += forward ? 1 : -1;
    maybe_update_pointing_device_cpi(config);
}

uint16_t dilemma_get_pointer_default_dpi(void) {
    return get_pointer_default_dpi(&g_dilemma_config);
}

uint16_t dilemma_get_pointer_sniping_dpi(void) {
    return get_pointer_sniping_dpi(&g_dilemma_config);
}

void dilemma_cycle_pointer_default_dpi_noeeprom(bool forward) {
    step_pointer_default_dpi(&g_dilemma_config, forward);
}

void dilemma_cycle_pointer_default_dpi(bool forward) {
    step_pointer_default_dpi(&g_dilemma_config, forward);
    write_dilemma_config_to_eeprom(&g_dilemma_config);
}

void dilemma_cycle_pointer_sniping_dpi_noeeprom(bool forward) {
    step_pointer_sniping_dpi(&g_dilemma_config, forward);
}

void dilemma_cycle_pointer_sniping_dpi(bool forward) {
    step_pointer_sniping_dpi(&g_dilemma_config, forward);
    write_dilemma_config_to_eeprom(&g_dilemma_config);
}

bool dilemma_get_pointer_sniping_enabled(void) {
    return g_dilemma_config.is_sniping_enabled;
}

void dilemma_set_pointer_sniping_enabled(bool enable) {
    g_dilemma_config.is_sniping_enabled = enable;
    // ONLY apply VIA config - the old system overrides the VIA sniping_dpi value!
    apply_via_dilemma_config();
}

bool dilemma_get_pointer_dragscroll_enabled(void) {
    return g_dilemma_config.is_dragscroll_enabled;
}

void dilemma_set_pointer_dragscroll_enabled(bool enable) {
    g_dilemma_config.is_dragscroll_enabled = enable;
    apply_via_dilemma_config();                      // Apply VIA DPI settings
    maybe_update_pointing_device_cpi(&g_dilemma_config);  // Old system
}

// Gesture state accessor implementations
bool dilemma_get_single_tap(void) {
    return g_gesture_state.single_tap;
}

bool dilemma_get_two_finger_tap(void) {
    return g_gesture_state.two_finger_tap;
}

bool dilemma_get_swipe_right(void) {
    return g_gesture_state.swipe_x_plus;
}

bool dilemma_get_swipe_left(void) {
    return g_gesture_state.swipe_x_minus;
}

bool dilemma_get_swipe_up(void) {
    return g_gesture_state.swipe_y_plus;
}

bool dilemma_get_swipe_down(void) {
    return g_gesture_state.swipe_y_minus;
}

bool dilemma_get_press_and_hold(void) {
    return g_gesture_state.press_and_hold;
}

/**
 * \brief Update scroll divisors based on current mode and VIA config.
 *
 * Selects appropriate divisors from VIA config:
 * - If dragscroll enabled: use drag_scroll_x/y_divisor
 * - If dragscroll disabled: use two_finger_x/y_divisor
 *
 * Called on mode changes and VIA config updates to keep divisors in sync.
 */
static void update_scroll_divisors(void) {
    if (g_dilemma_config.is_dragscroll_enabled) {
        g_current_scroll_x_divisor = g_via_dilemma_config.drag_scroll_x_divisor;
        g_current_scroll_y_divisor = g_via_dilemma_config.drag_scroll_y_divisor;
    } else {
        g_current_scroll_x_divisor = g_via_dilemma_config.two_finger_x_divisor;
        g_current_scroll_y_divisor = g_via_dilemma_config.two_finger_y_divisor;
    }
}

/**
 * \brief Update gesture state from digitizer events.
 *
 * Reads gesture flags from digitizer report and updates g_gesture_state.
 * Called from pointing_device_task_dilemma() on each iteration.
 *
 * Note: This function only READS gesture state from hardware.
 * Gesture filtering based on VIA config happens separately.
 *
 * TODO: Implement actual digitizer gesture read based on 03-01 findings.
 * MaxTouch MXT336U has no hardware gesture registers (unlike Azoteq IQS5xx),
 * so gesture detection happens in firmware via digitizer_mouse_fallback.c state machine.
 * Future implementation will integrate with that state machine to track gesture status.
 */
static void update_gesture_state(void) {
    // TODO: Implement actual digitizer gesture read based on 03-01 findings
    // For now, this is a placeholder for the integration point
    //
    // From 03-01 DISCOVERY.md:
    // - MaxTouch has NO GESTURE_EVENTS registers like Azoteq IQS5xx
    // - Gesture detection is firmware-based in digitizer_mouse_fallback.c
    // - State machine tracks: None, Down, MoveScroll, Tapped, DoubleTapped, Drag, Swipe, Zoom, Finished
    // - Plan 03-03 will integrate VIA config filtering into that state machine
    //
    // Future implementation (Plan 03-03):
    // - Access digitizer state machine to get current gesture state
    // - Update g_gesture_state fields based on active gesture
    // - This will be used by VIA config to enable/disable specific gestures
    //
    // Update zoom gesture state (if firmware implements it)
    // Note: This is a placeholder - actual zoom detection happens in digitizer_mouse_fallback.c
    // Future: Read firmware zoom state and update g_gesture_state.zoom
}

/**
 * \brief Filter gesture events based on VIA config enables.
 *
 * Checks gesture state against VIA config enable flags and clears
 * disabled gestures from the state. This prevents disabled gestures
 * from being processed by the firmware.
 *
 * Call this after update_gesture_state() to apply user preferences.
 */
static void filter_gestures_by_via_config(void) {
#ifdef CONSOLE_ENABLE
    bool filtered_tap = false;
    bool filtered_two_finger_tap = false;
    bool filtered_scroll = false;
    bool filtered_hold = false;
#endif

    // Clear gestures that are disabled in VIA config

    if (!g_via_dilemma_config.tap_to_click_enabled) {
        if (g_gesture_state.single_tap) {
#ifdef CONSOLE_ENABLE
            filtered_tap = true;
#endif
            g_gesture_state.single_tap = false;
        }
    }

    if (!g_via_dilemma_config.two_finger_tap_enabled) {
        if (g_gesture_state.two_finger_tap) {
#ifdef CONSOLE_ENABLE
            filtered_two_finger_tap = true;
#endif
            g_gesture_state.two_finger_tap = false;
        }
    }

    if (!g_via_dilemma_config.two_finger_scroll_enabled) {
        if (g_gesture_state.scroll) {
#ifdef CONSOLE_ENABLE
            filtered_scroll = true;
#endif
            g_gesture_state.scroll = false;
        }
        // Note: Existing two-finger scroll logic in dilemma.c also
        // needs to check this flag for consistency
    }

    if (!g_via_dilemma_config.press_and_hold_enabled) {
        if (g_gesture_state.press_and_hold) {
#ifdef CONSOLE_ENABLE
            filtered_hold = true;
#endif
            g_gesture_state.press_and_hold = false;
        }
    }

    // Advanced gesture filtering
    if (!g_via_dilemma_config.three_finger_swipe_enabled) {
        g_gesture_state.swipe_x_plus = false;      // Clear 3-finger swipes
        g_gesture_state.swipe_x_minus = false;
        g_gesture_state.swipe_y_plus = false;
        g_gesture_state.swipe_y_minus = false;
    }

    // Note: 4-finger swipes use same gesture state fields as 3-finger
    // The firmware distinguishes by swipe_finger_count, but VIA config
    // checks are done at state machine level, not in this filter function

    if (!g_via_dilemma_config.pinch_to_zoom_enabled) {
        g_gesture_state.zoom = false;  // Clear zoom gesture
    }

#ifdef CONSOLE_ENABLE
    if (filtered_tap || filtered_two_finger_tap || filtered_scroll || filtered_hold) {
        dprintf("(dilemma) gestures filtered: tap=%u two_finger_tap=%u scroll=%u hold=%u\n",
                filtered_tap, filtered_two_finger_tap, filtered_scroll, filtered_hold);
    }
#endif
}

void pointing_device_init_kb(void) {
    maybe_update_pointing_device_cpi(&g_dilemma_config);
    pointing_device_init_user();
}

/**
 * \brief Augment the pointing device behavior.
 *
 * Implement drag-scroll with configurable divisors and apply
 * VIA scroll divisors to two-finger scroll from digitizer.
 */
static void pointing_device_task_dilemma(report_mouse_t *mouse_report) {
    static int16_t scroll_buffer_x = 0;
    static int16_t scroll_buffer_y = 0;

    // Update scroll divisors based on current mode
    update_scroll_divisors();

    // Update gesture state from digitizer
    update_gesture_state();

    // Filter gestures based on VIA config
    filter_gestures_by_via_config();

    // Note: Force click is handled at firmware level in digitizer_mouse_fallback.c
    // The state machine checks press_and_hold_enabled and triggers press_and_hold_keycode
    // No additional state tracking needed here

    if (g_dilemma_config.is_dragscroll_enabled) {
        // Drag-scroll mode: convert trackball movement to scroll
        // Apply VIA drag-scroll divisor to scale movement before accumulation
#    ifdef DILEMMA_DRAGSCROLL_REVERSE_X
        scroll_buffer_x -= (mouse_report->x / g_current_scroll_x_divisor);
#    else
        scroll_buffer_x += (mouse_report->x / g_current_scroll_x_divisor);
#    endif // DILEMMA_DRAGSCROLL_REVERSE_X
#    ifdef DILEMMA_DRAGSCROLL_REVERSE_Y
        scroll_buffer_y -= (mouse_report->y / g_current_scroll_y_divisor);
#    else
        scroll_buffer_y += (mouse_report->y / g_current_scroll_y_divisor);
#    endif // DILEMMA_DRAGSCROLL_REVERSE_Y
        mouse_report->x = 0;
        mouse_report->y = 0;
        if (abs(scroll_buffer_x) > DILEMMA_DRAGSCROLL_BUFFER_SIZE) {
            mouse_report->h = scroll_buffer_x > 0 ? 1 : -1;
            scroll_buffer_x = 0;
        }
        if (abs(scroll_buffer_y) > DILEMMA_DRAGSCROLL_BUFFER_SIZE) {
            mouse_report->v = scroll_buffer_y > 0 ? 1 : -1;
            scroll_buffer_y = 0;
        }
    } else {
        // Two-finger scroll mode: apply VIA two-finger divisor to digitizer scroll
        // (only if enabled in VIA config)
        // The digitizer driver has already applied DIGITIZER_SCROLL_DIVISOR,
        // so we apply an additional divisor here for fine-grained control
        if (mouse_report->h != 0 && g_via_dilemma_config.two_finger_scroll_enabled && g_current_scroll_x_divisor > 1) {
            mouse_report->h = mouse_report->h / g_current_scroll_x_divisor;
        }
        if (mouse_report->v != 0 && g_via_dilemma_config.two_finger_scroll_enabled && g_current_scroll_y_divisor > 1) {
            mouse_report->v = mouse_report->v / g_current_scroll_y_divisor;
        }
    }
}

report_mouse_t pointing_device_task_kb(report_mouse_t mouse_report) {
    if (is_keyboard_master()) {
        pointing_device_task_dilemma(&mouse_report);
        mouse_report = pointing_device_task_user(mouse_report);
    }
    return mouse_report;
}

/**
 * \brief Layer state change hook for auto-sniping
 *
 * When auto_snipe_enabled is true and auto_snipe_layer becomes active,
 * automatically enable sniping mode. When leaving that layer, disable sniping.
 */
layer_state_t layer_state_set_kb(layer_state_t state) {
    // Debug: Log all layer state changes
    uprintf("LAYER STATE: state=0x%X, target_layer=%d\n",
            state, g_via_dilemma_config.auto_snipe_layer);

    // Auto-snipe is controlled by the layer value:
    // - 127 means auto-snipe is disabled
    // - 0-126 means auto-snipe is enabled for that layer
    uint8_t target_layer = g_via_dilemma_config.auto_snipe_layer;

    // 127 means auto-snipe is disabled
    if (target_layer != 127) {
        // Check if target layer is in current layer state
        if (state & (1 << target_layer)) {
            // Target layer is active, enable sniping
            if (!dilemma_get_pointer_sniping_enabled()) {
                uprintf("AUTO-SNIPE: Layer %d active, enabling sniping\n", target_layer);
                dilemma_set_pointer_sniping_enabled(true);
            }
        } else {
            // Target layer is not active, disable sniping if we enabled it
            if (dilemma_get_pointer_sniping_enabled()) {
                // Only disable if no manual sniping key is held
                // (auto-snipe should not interfere with manual sniping)
                uprintf("AUTO-SNIPE: Layer %d inactive, disabling sniping\n", target_layer);
                dilemma_set_pointer_sniping_enabled(false);
            }
        }
    }

    // Call user-level layer_state_set_user
    return layer_state_set_user(state);
}

#    if defined(POINTING_DEVICE_ENABLE) && !defined(NO_DILEMMA_KEYCODES)
/** \brief Whether SHIFT mod is enabled. */
static bool has_shift_mod(void) {
#        ifdef NO_ACTION_ONESHOT
    return mod_config(get_mods()) & MOD_MASK_SHIFT;
#        else
    return mod_config(get_mods() | get_oneshot_mods()) & MOD_MASK_SHIFT;
#        endif // NO_ACTION_ONESHOT
}
#    endif // POINTING_DEVICE_ENABLE && !NO_DILEMMA_KEYCODES

/**
 * \brief Outputs the Dilemma configuration to console.
 *
 * Prints the in-memory configuration structure to console, for debugging.
 * Includes:
 *   - raw value
 *   - drag-scroll: on/off
 *   - sniping: on/off
 *   - default DPI: internal table index/actual DPI
 *   - sniping DPI: internal table index/actual DPI
 */
static void debug_dilemma_config_to_console(dilemma_config_t *config) {
#    ifdef CONSOLE_ENABLE
    dprintf("(dilemma) process_record_kb: config = {\n"
            "\traw = 0x%X,\n"
            "\t{\n"
            "\t\tis_dragscroll_enabled=%u\n"
            "\t\tis_sniping_enabled=%u\n"
            "\t\tdefault_dpi=0x%X (%u)\n"
            "\t\tsniping_dpi=0x%X (%u)\n"
            "\t}\n"
            "}\n",
            config->raw, config->is_dragscroll_enabled, config->is_sniping_enabled, config->pointer_default_dpi, get_pointer_default_dpi(config), config->pointer_sniping_dpi, get_pointer_sniping_dpi(config));
#    endif // CONSOLE_ENABLE
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_user(keycode, record)) {
        debug_dilemma_config_to_console(&g_dilemma_config);
        return false;
    }
#    ifdef POINTING_DEVICE_ENABLE
#        ifndef NO_DILEMMA_KEYCODES
    switch (keycode) {
        case POINTER_DEFAULT_DPI_FORWARD:
            if (record->event.pressed) {
                // Step backward if shifted, forward otherwise.
                dilemma_cycle_pointer_default_dpi(/* forward= */ !has_shift_mod());
            }
            break;
        case POINTER_DEFAULT_DPI_REVERSE:
            if (record->event.pressed) {
                // Step forward if shifted, backward otherwise.
                dilemma_cycle_pointer_default_dpi(/* forward= */ has_shift_mod());
            }
            break;
        case POINTER_SNIPING_DPI_FORWARD:
            if (record->event.pressed) {
                // Step backward if shifted, forward otherwise.
                dilemma_cycle_pointer_sniping_dpi(/* forward= */ !has_shift_mod());
            }
            break;
        case POINTER_SNIPING_DPI_REVERSE:
            if (record->event.pressed) {
                // Step forward if shifted, backward otherwise.
                dilemma_cycle_pointer_sniping_dpi(/* forward= */ has_shift_mod());
            }
            break;
        case SNIPING_MODE:
            dilemma_set_pointer_sniping_enabled(record->event.pressed);
            break;
        case SNIPING_MODE_TOGGLE:
            if (record->event.pressed) {
                dilemma_set_pointer_sniping_enabled(!dilemma_get_pointer_sniping_enabled());
            }
            break;
        case DRAGSCROLL_MODE:
            dilemma_set_pointer_dragscroll_enabled(record->event.pressed);
            break;
        case DRAGSCROLL_MODE_TOGGLE:
            if (record->event.pressed) {
                dilemma_set_pointer_dragscroll_enabled(!dilemma_get_pointer_dragscroll_enabled());
            }
            break;
    }
#        endif // !NO_DILEMMA_KEYCODES
#    endif     // POINTING_DEVICE_ENABLE
    debug_dilemma_config_to_console(&g_dilemma_config);
    if (IS_QK_KB(keycode) || IS_MOUSEKEY(keycode)) {
        debug_dilemma_config_to_console(&g_dilemma_config);
    }
    return true;
}

void eeconfig_init_kb(void) {
    g_dilemma_config.raw                 = 0;
    g_dilemma_config.pointer_default_dpi = 3; // DPI=1000
    write_dilemma_config_to_eeprom(&g_dilemma_config);

    // Initialize VIA custom config defaults
    memset(&g_via_dilemma_config, 0, sizeof(g_via_dilemma_config));
    g_via_dilemma_config.dpi_preset = 3; // 1000 DPI default
    g_via_dilemma_config.drag_scroll_x_divisor = 8; // Reasonable defaults
    g_via_dilemma_config.drag_scroll_y_divisor = 8;
    g_via_dilemma_config.two_finger_x_divisor = 8;
    g_via_dilemma_config.two_finger_y_divisor = 8;
    g_via_dilemma_config.tap_to_click_enabled = 1;        // Enabled by default
    g_via_dilemma_config.two_finger_tap_enabled = 0;     // Disabled by default
    g_via_dilemma_config.two_finger_scroll_enabled = 1;  // Enabled by default (already working)
    g_via_dilemma_config.press_and_hold_enabled = 0;     // Disabled by default

    // Byte 7: Advanced gesture enables
    g_via_dilemma_config.three_finger_swipe_enabled = 1;  // ON - core feature
    g_via_dilemma_config.four_finger_swipe_enabled = 0;   // OFF - advanced feature
    g_via_dilemma_config.pinch_to_zoom_enabled = 0;        // OFF - not yet implemented

    // Bytes 18-37: Advanced gesture keycodes
    // 3-finger swipes (match existing defaults)
    g_via_dilemma_config.three_finger_swipe_left_keycode = QK_MOUSE_BUTTON_3;
    g_via_dilemma_config.three_finger_swipe_right_keycode = QK_MOUSE_BUTTON_4;
    g_via_dilemma_config.three_finger_swipe_up_keycode = KC_LEFT_GUI;
    g_via_dilemma_config.three_finger_swipe_down_keycode = KC_ESC;

    // 4-finger swipes (macOS defaults)
    g_via_dilemma_config.four_finger_swipe_left_keycode = LCTL(KC_LEFT);
    g_via_dilemma_config.four_finger_swipe_right_keycode = LCTL(KC_RIGHT);
    g_via_dilemma_config.four_finger_swipe_up_keycode = KC_F11;
    g_via_dilemma_config.four_finger_swipe_down_keycode = KC_F10;

    // Pinch-to-zoom (common defaults)
    g_via_dilemma_config.zoom_in_keycode = KC_EQUAL;
    g_via_dilemma_config.zoom_out_keycode = KC_MINUS;

    // Auto-snipe defaults (Byte 5)
    g_via_dilemma_config.auto_snipe_enabled = 0;  // OFF by default (opt-in feature)
    g_via_dilemma_config.auto_snipe_layer = 4;     // Default to layer 4
    g_via_dilemma_config.sniping_dpi = 200;        // Default sniping DPI (matches pointer_sniping_dpi = 0)

    g_via_dilemma_config.config_version = 1;
    write_via_dilemma_config();

    maybe_update_pointing_device_cpi(&g_dilemma_config);
    eeconfig_init_user();
}

// VIA Custom Configuration
// VIA EEPROM custom config is enabled (VIA_EEPROM_CUSTOM_CONFIG_SIZE = 32)

#include "via.h"

/**
 * \brief Map VIA DPI preset to actual DPI value.
 *
 * Converts a 3-bit preset value (0-7) to a concrete DPI number:
 * - Presets 0-5: Standard DPI steps (200, 400, 600, 800, 1000, 1200)
 * - Preset 6: Custom DPI value (read from custom_dpi field)
 * - Preset 7: Reserved for future expansion
 *
 * \param preset The DPI preset value (0-7).
 * \return The corresponding DPI value, or 0 for invalid presets.
 */
static uint16_t get_dpi_from_preset(uint8_t preset) {
    switch (preset) {
        case 0:
            return 200;
        case 1:
            return 400;
        case 2:
            return 600;
        case 3:
            return 800;
        case 4:
            return 1000;
        case 5:
            return 1200;
        case 6:
            return g_via_dilemma_config.custom_dpi;
        case 7:
        default:
            return 0; // Reserved/invalid
    }
}

/**
 * \brief Apply VIA config settings to pointing device hardware.
 *
 * Reads the current VIA dilemma config and applies the DPI preset to the
 * pointing device. This bridges the gap between VIA config storage and
 * actual device behavior.
 *
 * Called from:
 * - matrix_init_kb() after loading VIA config on boot
 * - VIA save handler after config changes
 * - After DPI preset changes
 *
 * Follows the same pattern as maybe_update_pointing_device_cpi().
 */
static void apply_via_dilemma_config(void) {
    // Check if sniping is enabled, use VIA sniping DPI if so
    if (dilemma_get_pointer_sniping_enabled()) {
        // Use VIA-configured sniping DPI (50-800 range)
        uint16_t sniping_dpi = g_via_dilemma_config.sniping_dpi;
        if (sniping_dpi >= 50 && sniping_dpi <= 800) {
            pointing_device_set_cpi(sniping_dpi);
        }
    } else {
        // Use normal DPI preset
        uint16_t dpi = get_dpi_from_preset(g_via_dilemma_config.dpi_preset);
        if (dpi > 0) {
            pointing_device_set_cpi(dpi);
        }
    }
    // Update scroll divisors after VIA config changes
    update_scroll_divisors();
    // Gesture filtering automatically uses updated VIA config
    // No explicit call needed - filter_gestures_by_via_config()
    // runs on each pointing_device_task iteration
}

// Read custom config from EEPROM
static void read_via_dilemma_config(void) {
    via_read_custom_config(g_via_dilemma_config.raw, 0, sizeof(g_via_dilemma_config.raw));
}

// Write custom config to EEPROM
static void write_via_dilemma_config(void) {
    via_update_custom_config(g_via_dilemma_config.raw, 0, sizeof(g_via_dilemma_config.raw));
}

// VIA custom value command handler
void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    // data = [command_id, channel_id, value_id, value_data...]
    uint8_t *command_id = &(data[0]);
    uint8_t *value_id   = &(data[2]);
    uint8_t *value_data = &(data[3]);

    switch (*command_id) {
        case id_custom_set_value: // 0x07
            // Write value to config
            switch (*value_id) {
                case id_dilemma_dpi_preset:
                    if (value_data[0] > 6) {  // Preset 7 reserved
                        *command_id = id_unhandled;
                        break;
                    }
                    g_via_dilemma_config.dpi_preset = value_data[0];
                    break;
                case id_dilemma_custom_dpi:
                    // 16-bit value across 2 bytes
                    {
                        uint16_t custom_dpi = ((uint16_t)value_data[1] << 8) | value_data[0];
                        if (custom_dpi < 200) custom_dpi = 200;
                        if (custom_dpi > 4000) custom_dpi = 4000;
                        g_via_dilemma_config.custom_dpi = custom_dpi;
                    }
                    break;
                case id_dilemma_drag_scroll_x:
                    g_via_dilemma_config.drag_scroll_x_divisor = value_data[0];
                    break;
                case id_dilemma_drag_scroll_y:
                    g_via_dilemma_config.drag_scroll_y_divisor = value_data[0];
                    break;
                case id_dilemma_two_finger_x:
                    g_via_dilemma_config.two_finger_x_divisor = value_data[0];
                    break;
                case id_dilemma_two_finger_y:
                    g_via_dilemma_config.two_finger_y_divisor = value_data[0];
                    break;
                case id_dilemma_tap_to_click_enable:
                    g_via_dilemma_config.tap_to_click_enabled = value_data[0];
                    break;
                case id_dilemma_two_finger_tap_enable:
                    g_via_dilemma_config.two_finger_tap_enabled = value_data[0];
                    break;
                case id_dilemma_two_finger_scroll_enable:
                    g_via_dilemma_config.two_finger_scroll_enabled = value_data[0];
                    break;
                case id_dilemma_press_and_hold_enable:
                    g_via_dilemma_config.press_and_hold_enabled = value_data[0];
                    break;
                case id_dilemma_three_finger_swipe_enable:
                    g_via_dilemma_config.three_finger_swipe_enabled = value_data[0];
                    break;
                case id_dilemma_four_finger_swipe_enable:
                    g_via_dilemma_config.four_finger_swipe_enabled = value_data[0];
                    break;
                case id_dilemma_pinch_to_zoom_enable:
                    g_via_dilemma_config.pinch_to_zoom_enabled = value_data[0];
                    break;
                case id_dilemma_three_finger_swipe_left_keycode:
                    g_via_dilemma_config.three_finger_swipe_left_keycode = (value_data[0] << 8) | value_data[1];
                    break;
                case id_dilemma_three_finger_swipe_right_keycode:
                    g_via_dilemma_config.three_finger_swipe_right_keycode = (value_data[0] << 8) | value_data[1];
                    break;
                case id_dilemma_three_finger_swipe_up_keycode:
                    g_via_dilemma_config.three_finger_swipe_up_keycode = (value_data[0] << 8) | value_data[1];
                    break;
                case id_dilemma_three_finger_swipe_down_keycode:
                    g_via_dilemma_config.three_finger_swipe_down_keycode = (value_data[0] << 8) | value_data[1];
                    break;
                case id_dilemma_four_finger_swipe_left_keycode:
                    g_via_dilemma_config.four_finger_swipe_left_keycode = (value_data[0] << 8) | value_data[1];
                    break;
                case id_dilemma_four_finger_swipe_right_keycode:
                    g_via_dilemma_config.four_finger_swipe_right_keycode = (value_data[0] << 8) | value_data[1];
                    break;
                case id_dilemma_four_finger_swipe_up_keycode:
                    g_via_dilemma_config.four_finger_swipe_up_keycode = (value_data[0] << 8) | value_data[1];
                    break;
                case id_dilemma_four_finger_swipe_down_keycode:
                    g_via_dilemma_config.four_finger_swipe_down_keycode = (value_data[0] << 8) | value_data[1];
                    break;
                case id_dilemma_zoom_in_keycode:
                    g_via_dilemma_config.zoom_in_keycode = (value_data[0] << 8) | value_data[1];
                    break;
                case id_dilemma_zoom_out_keycode:
                    g_via_dilemma_config.zoom_out_keycode = (value_data[0] << 8) | value_data[1];
                    break;
                case id_dilemma_auto_snipe_enable:
                    // Boolean validation: 0 or 1
                    if (value_data[0] < 2) {
                        g_via_dilemma_config.auto_snipe_enabled = value_data[0];
                    }
                    break;
                case id_dilemma_auto_snipe_layer:
                    // Layer validation: 0-127 (7-bit field)
                    if (value_data[0] < 128) {
                        g_via_dilemma_config.auto_snipe_layer = value_data[0];
                    }
                    break;
                case id_dilemma_sniping_dpi:
                    // 8-bit value (50-200 range) - dropdown only sends one byte
                    {
                        uint16_t sniping_dpi = value_data[0];
                        if (sniping_dpi < 50) sniping_dpi = 50;
                        if (sniping_dpi > 200) sniping_dpi = 200;
                        g_via_dilemma_config.sniping_dpi = sniping_dpi;
                        uprintf("VIA SET: Sniping DPI=%d (was %d)\n", sniping_dpi, value_data[0]);
                        // Apply immediately if sniping is currently enabled
                        if (dilemma_get_pointer_sniping_enabled()) {
                            pointing_device_set_cpi(g_via_dilemma_config.sniping_dpi);
                            uprintf("VIA SET: Applied sniping DPI=%d immediately\n", g_via_dilemma_config.sniping_dpi);
                        }
                    }
                    break;
                // Add more setters for each value_id
                default:
                    *command_id = id_unhandled; // Unknown value ID
                    break;
            }
            break;

        case id_custom_get_value: // 0x08
            // Read value from config
            switch (*value_id) {
                case id_dilemma_dpi_preset:
                    value_data[0] = g_via_dilemma_config.dpi_preset;
                    break;
                case id_dilemma_custom_dpi:
                    value_data[0] = g_via_dilemma_config.custom_dpi & 0xFF;
                    value_data[1] = (g_via_dilemma_config.custom_dpi >> 8) & 0xFF;
                    break;
                case id_dilemma_drag_scroll_x:
                    value_data[0] = g_via_dilemma_config.drag_scroll_x_divisor;
                    break;
                case id_dilemma_drag_scroll_y:
                    value_data[0] = g_via_dilemma_config.drag_scroll_y_divisor;
                    break;
                case id_dilemma_two_finger_x:
                    value_data[0] = g_via_dilemma_config.two_finger_x_divisor;
                    break;
                case id_dilemma_two_finger_y:
                    value_data[0] = g_via_dilemma_config.two_finger_y_divisor;
                    break;
                case id_dilemma_tap_to_click_enable:
                    value_data[0] = g_via_dilemma_config.tap_to_click_enabled;
                    break;
                case id_dilemma_two_finger_tap_enable:
                    value_data[0] = g_via_dilemma_config.two_finger_tap_enabled;
                    break;
                case id_dilemma_two_finger_scroll_enable:
                    value_data[0] = g_via_dilemma_config.two_finger_scroll_enabled;
                    break;
                case id_dilemma_press_and_hold_enable:
                    value_data[0] = g_via_dilemma_config.press_and_hold_enabled;
                    break;
                case id_dilemma_three_finger_swipe_enable:
                    value_data[0] = g_via_dilemma_config.three_finger_swipe_enabled;
                    break;
                case id_dilemma_four_finger_swipe_enable:
                    value_data[0] = g_via_dilemma_config.four_finger_swipe_enabled;
                    break;
                case id_dilemma_pinch_to_zoom_enable:
                    value_data[0] = g_via_dilemma_config.pinch_to_zoom_enabled;
                    break;
                case id_dilemma_three_finger_swipe_left_keycode:
                    value_data[0] = (g_via_dilemma_config.three_finger_swipe_left_keycode >> 8) & 0xFF;
                    value_data[1] = g_via_dilemma_config.three_finger_swipe_left_keycode & 0xFF;
                    break;
                case id_dilemma_three_finger_swipe_right_keycode:
                    value_data[0] = (g_via_dilemma_config.three_finger_swipe_right_keycode >> 8) & 0xFF;
                    value_data[1] = g_via_dilemma_config.three_finger_swipe_right_keycode & 0xFF;
                    break;
                case id_dilemma_three_finger_swipe_up_keycode:
                    value_data[0] = (g_via_dilemma_config.three_finger_swipe_up_keycode >> 8) & 0xFF;
                    value_data[1] = g_via_dilemma_config.three_finger_swipe_up_keycode & 0xFF;
                    break;
                case id_dilemma_three_finger_swipe_down_keycode:
                    value_data[0] = (g_via_dilemma_config.three_finger_swipe_down_keycode >> 8) & 0xFF;
                    value_data[1] = g_via_dilemma_config.three_finger_swipe_down_keycode & 0xFF;
                    break;
                case id_dilemma_four_finger_swipe_left_keycode:
                    value_data[0] = (g_via_dilemma_config.four_finger_swipe_left_keycode >> 8) & 0xFF;
                    value_data[1] = g_via_dilemma_config.four_finger_swipe_left_keycode & 0xFF;
                    break;
                case id_dilemma_four_finger_swipe_right_keycode:
                    value_data[0] = (g_via_dilemma_config.four_finger_swipe_right_keycode >> 8) & 0xFF;
                    value_data[1] = g_via_dilemma_config.four_finger_swipe_right_keycode & 0xFF;
                    break;
                case id_dilemma_four_finger_swipe_up_keycode:
                    value_data[0] = (g_via_dilemma_config.four_finger_swipe_up_keycode >> 8) & 0xFF;
                    value_data[1] = g_via_dilemma_config.four_finger_swipe_up_keycode & 0xFF;
                    break;
                case id_dilemma_four_finger_swipe_down_keycode:
                    value_data[0] = (g_via_dilemma_config.four_finger_swipe_down_keycode >> 8) & 0xFF;
                    value_data[1] = g_via_dilemma_config.four_finger_swipe_down_keycode & 0xFF;
                    break;
                case id_dilemma_zoom_in_keycode:
                    value_data[0] = (g_via_dilemma_config.zoom_in_keycode >> 8) & 0xFF;
                    value_data[1] = g_via_dilemma_config.zoom_in_keycode & 0xFF;
                    break;
                case id_dilemma_zoom_out_keycode:
                    value_data[0] = (g_via_dilemma_config.zoom_out_keycode >> 8) & 0xFF;
                    value_data[1] = g_via_dilemma_config.zoom_out_keycode & 0xFF;
                    break;
                case id_dilemma_auto_snipe_enable:
                    value_data[0] = g_via_dilemma_config.auto_snipe_enabled;
                    break;
                case id_dilemma_auto_snipe_layer:
                    value_data[0] = g_via_dilemma_config.auto_snipe_layer;
                    break;
                case id_dilemma_sniping_dpi:
                    value_data[0] = g_via_dilemma_config.sniping_dpi & 0xFF;
                    value_data[1] = (g_via_dilemma_config.sniping_dpi >> 8) & 0xFF;
                    break;
                // Add more getters for each value_id
                default:
                    *command_id = id_unhandled;
                    break;
            }
            break;

        case id_custom_save: // 0x09
            // Persist config to EEPROM
            write_via_dilemma_config();     // Persist to EEPROM
            apply_via_dilemma_config();     // Apply to hardware
            break;

        default:
            *command_id = id_unhandled;
            break;
    }
}

void matrix_init_kb(void) {
    read_dilemma_config_from_eeprom(&g_dilemma_config);

    // Initialize VIA custom config
    read_via_dilemma_config();

    // Apply VIA config settings to pointing device
    apply_via_dilemma_config();

    matrix_init_user();
}
#endif // POINTING_DEVICE_ENABLE

#ifdef RGB_MATRIX_ENABLE
static HSV _get_hsv_for_layer_index(uint8_t layer) {
    switch (layer) {
        case 1:
            return (HSV){HSV_BLUE};
        case 2:
            return (HSV){HSV_ORANGE};
        case 3:
            return (HSV){HSV_AZURE};
        case 4:
            return (HSV){HSV_GREEN};
        case 5:
            return (HSV){HSV_TEAL};
        case 6:
            return (HSV){HSV_PURPLE};
        case 7:
        default:
            return (hsv_t){HSV_RED};
            break;
    };
}

// Layer state indicator
bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {
    if (!rgb_matrix_indicators_advanced_user(led_min, led_max)) {
        return false;
    }

    // Set underglow to a solid color for highest active layer apart from the base layer.
    const uint8_t layer = get_highest_layer(layer_state);
    if (layer > 0) {
        HSV hsv = _get_hsv_for_layer_index(layer);

        // Set brightness to the configured interval brighter than current brightness, clamped to 255 (ie. uint8_t max value). This compensates for the dimmer appearance of the underglow LEDs.
        hsv.v         = MIN(rgb_matrix_get_val() + LAYER_INDICATOR_BRIGHTNESS_INC, 255);
        const RGB rgb = hsv_to_rgb(hsv);

        for (int i = led_min; i < led_max; i++) {
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        }
    }

    // Set underglow LEDs to red if caps lock is enabled
    if (host_keyboard_led_state().caps_lock) {
        for (int i = led_min; i <= led_max; i++) {
            if (HAS_FLAGS(g_led_config.flags[i], LED_FLAG_UNDERGLOW)) {
                // set modifier-flagged LEDs to a pure a configured interval brighter than the current brightness, clamped to 255 (ie. uint8_t max value).
                rgb_matrix_set_color(i, MIN(rgb_matrix_get_val() + CAPSLOCK_INDICATOR_BRIGHTNESS_INC, 255), 0, 0);
            }
        }
    }

    return true;
};
#endif // RGB_MATRIX_ENABLE

#ifdef ENCODER_ENABLE
bool encoder_update_kb(uint8_t index, bool clockwise) {
    if (!encoder_update_user(index, clockwise)) {
        return false;
    }
    switch (index) {
        case 0: // Left-half encoder, mouse scroll.
            tap_code(clockwise ? MS_WHLU : MS_WHLD);
            break;
        case 1: // Right-half encoder, volume control.
            tap_code(clockwise ? KC_AUDIO_VOL_UP : KC_AUDIO_VOL_DOWN);
            break;
    }
    return true;
}
#endif // ENCODER_ENABLE

// Forward declare RP2040 SDK declaration.
void gpio_init(uint gpio);

void keyboard_pre_init_kb(void) {
    // Ensures that GP26 through GP29 are initialized as digital inputs (as
    // opposed to analog inputs).  These GPIOs are shared with A0 through A3,
    // respectively.  On RP2040-B2 and later, the digital inputs are disabled by
    // default (see RP2040-E6).
    gpio_init(GP26);
    gpio_init(GP27);
    gpio_init(GP28);
    gpio_init(GP29);

    keyboard_pre_init_user();
}

bool shutdown_kb(bool jump_to_bootloader) {
    if (!shutdown_user(jump_to_bootloader)) {
        return false;
    }
#ifdef RGBLIGHT_ENABLE
    rgblight_enable_noeeprom();
    rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
    rgblight_setrgb(RGB_RED);
#endif // RGBLIGHT_ENABLE
#ifdef RGB_MATRIX_ENABLE
    rgb_matrix_set_color_all(RGB_RED);
    rgb_matrix_update_pwm_buffers();
#endif // RGB_MATRIX_ENABLE
    return true;
}
