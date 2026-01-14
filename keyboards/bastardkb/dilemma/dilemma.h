/**
 * Copyright 2022 Charly Delay <charly@codesink.dev> (@0xcharly)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
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

#pragma once

#include "quantum.h"

#ifdef POINTING_DEVICE_ENABLE
#    ifndef NO_DILEMMA_KEYCODES
enum dilemma_keycodes {
    POINTER_DEFAULT_DPI_FORWARD = QK_KB_0,
    POINTER_DEFAULT_DPI_REVERSE,
    POINTER_SNIPING_DPI_FORWARD,
    POINTER_SNIPING_DPI_REVERSE,
    SNIPING_MODE,
    SNIPING_MODE_TOGGLE,
    DRAGSCROLL_MODE,
    DRAGSCROLL_MODE_TOGGLE,
};

#        define DPI_MOD POINTER_DEFAULT_DPI_FORWARD
#        define DPI_RMOD POINTER_DEFAULT_DPI_REVERSE
#        define S_D_MOD POINTER_SNIPING_DPI_FORWARD
#        define S_D_RMOD POINTER_SNIPING_DPI_REVERSE
#        define SNIPING SNIPING_MODE
#        define SNP_TOG SNIPING_MODE_TOGGLE
#        define DRGSCRL DRAGSCROLL_MODE
#        define DRG_TOG DRAGSCROLL_MODE_TOGGLE
#    endif // !NO_DILEMMA_KEYCODES

// VIA Custom Value IDs
enum via_dilemma_value_id {
    id_dilemma_unhandled = 0, // Must be 0
    id_dilemma_config_save = 1,
    id_dilemma_dpi_preset = 2,
    id_dilemma_custom_dpi = 3,
    id_dilemma_drag_scroll_x = 4,
    id_dilemma_drag_scroll_y = 5,
    id_dilemma_two_finger_x = 6,
    id_dilemma_two_finger_y = 7,
    // Basic gesture enables (Phase 3)
    id_dilemma_tap_to_click_enable = 8,
    id_dilemma_two_finger_tap_enable = 9,
    id_dilemma_two_finger_scroll_enable = 10,
    id_dilemma_press_and_hold_enable = 11,
    // Advanced gesture enables (Byte 7)
    id_dilemma_three_finger_swipe_enable = 12,
    id_dilemma_four_finger_swipe_enable = 13,
    id_dilemma_pinch_to_zoom_enable = 14,
    // 3-finger swipe keycodes (Bytes 18-25, 16-bit each)
    id_dilemma_three_finger_swipe_left_keycode = 15,
    id_dilemma_three_finger_swipe_right_keycode = 16,
    id_dilemma_three_finger_swipe_up_keycode = 17,
    id_dilemma_three_finger_swipe_down_keycode = 18,
    // 4-finger swipe keycodes (Bytes 26-33, 16-bit each)
    id_dilemma_four_finger_swipe_left_keycode = 19,
    id_dilemma_four_finger_swipe_right_keycode = 20,
    id_dilemma_four_finger_swipe_up_keycode = 21,
    id_dilemma_four_finger_swipe_down_keycode = 22,
    // Add more IDs as needed for phases 5-12
};

// Global config instance (defined in dilemma.c)
extern via_dilemma_config_t g_via_dilemma_config;

/**
 * VIA Custom Configuration EEPROM Layout
 * Total size: 32 bytes (256 bits)
 * Version: 1 (increment when structure changes incompatibly)
 *
 * Migration strategy:
 * - Load from EEPROM, check config_version field
 * - If version mismatch, reset to defaults and update version
 * - Reserved fields must be zero-initialized
 *
 * This struct packs 30+ settings into 32 bytes using efficient bit-field layout.
 * Phase 2 will implement the persistence layer for reading/writing these settings.
 */
typedef union {
    uint8_t raw[32];
    struct {
        // Byte 0: DPI settings
        uint8_t dpi_preset : 3;           // 0-7 (5 presets + custom + 2 expansion)
        uint8_t reserved_0 : 5;           // Future DPI features

        // Bytes 1-2: Custom DPI value (12 bits) + reserved
        uint16_t custom_dpi : 12;         // 0-4095 (Azoteq range)
        uint16_t reserved_1 : 4;          // Future DPI precision

        // Byte 3: Drag scroll divisors (4 bits each)
        uint8_t drag_scroll_x_divisor : 4; // 0-15
        uint8_t drag_scroll_y_divisor : 4; // 0-15

        // Byte 4: Two-finger scroll divisors
        uint8_t two_finger_x_divisor : 4;  // 0-15
        uint8_t two_finger_y_divisor : 4;  // 0-15

        // Byte 5: Auto-sniping configuration
        uint8_t auto_snipe_enabled : 1;    // Enable/disable
        uint8_t auto_snipe_layer : 7;      // Target layer (0-127)

        // Byte 6: Basic gesture enables
        uint8_t tap_to_click_enabled : 1;   // Single-finger tap
        uint8_t two_finger_tap_enabled : 1; // Right-click equivalent
        uint8_t two_finger_scroll_enabled : 1; // Scroll gesture
        uint8_t press_and_hold_enabled : 1;  // Long-press selection
        uint8_t reserved_6 : 4;            // Future gesture enables

        // Byte 7: Advanced gesture enables
        uint8_t three_finger_swipe_enabled : 1; // App switcher
        uint8_t four_finger_swipe_enabled : 1;  // Desktop spaces
        uint8_t pinch_to_zoom_enabled : 1;      // Smart zoom
        uint8_t reserved_7 : 5;                // Future advanced gestures

        // Bytes 8-19: Gesture-to-keycode mappings (12 bytes)
        // Each gesture maps to a 16-bit keycode
        uint16_t tap_to_click_keycode;         // Byte 8-9
        uint16_t two_finger_tap_keycode;       // Byte 10-11
        uint16_t two_finger_scroll_up_keycode; // Byte 12-13
        uint16_t two_finger_scroll_down_keycode; // Byte 14-15
        uint16_t press_and_hold_keycode;       // Byte 16-17
        uint16_t swipe_keycode;                // Byte 18-19

        // Byte 20: Smart gesture features
        uint8_t tap_pressure_threshold : 4;    // 0-15 (sensitivity)
        uint8_t force_click_enabled : 1;       // Long-press force click
        uint8_t smart_zoom_enabled : 1;        // Pinch-to-zoom enhancement
        uint8_t reserved_20 : 2;              // Future smart features

        // Bytes 21-30: Reserved for future expansion (10 bytes)
        uint8_t reserved_21[10];              // Phase 10+ features

        // Byte 31: Versioning and final reserved
        uint8_t config_version : 4;           // EEPROM format version
        uint8_t reserved_31 : 4;             // Future versioning needs
    } __attribute__((packed));
} via_dilemma_config_t;

/** \brief Return the current DPI value for the pointer's default mode. */
uint16_t dilemma_get_pointer_default_dpi(void);

/**
 * \brief Update the pointer's default DPI to the next or previous step.
 *
 * Increases the DPI value if `forward` is `true`, decreases it otherwise.
 * The increment/decrement steps are equal to DILEMMA_DEFAULT_DPI_CONFIG_STEP.
 *
 * The new value is persisted in EEPROM.
 */
void dilemma_cycle_pointer_default_dpi(bool forward);

/**
 * \brief Same as `dilemma_cycle_pointer_default_dpi`, but do not write to
 * EEPROM.
 *
 * This means that reseting the board will revert the value to the last
 * persisted one.
 */
void dilemma_cycle_pointer_default_dpi_noeeprom(bool forward);

/** \brief Return the current DPI value for the pointer's sniper-mode. */
uint16_t dilemma_get_pointer_sniping_dpi(void);

/**
 * \brief Update the pointer's sniper-mode DPI to the next or previous step.
 *
 * Increases the DPI value if `forward` is `true`, decreases it otherwise.
 * The increment/decrement steps are equal to DILEMMA_SNIPING_DPI_CONFIG_STEP.
 *
 * The new value is persisted in EEPROM.
 */
void dilemma_cycle_pointer_sniping_dpi(bool forward);

/**
 * \brief Same as `dilemma_cycle_pointer_sniping_dpi`, but do not write to
 * EEPROM.
 *
 * This means that reseting the board will revert the value to the last
 * persisted one.
 */
void dilemma_cycle_pointer_sniping_dpi_noeeprom(bool forward);

/** \brief Whether sniper-mode is enabled. */
bool dilemma_get_pointer_sniping_enabled(void);

/**
 * \brief Enable/disable sniper mode.
 *
 * When sniper mode is enabled the dpi is reduced to slow down the pointer for
 * more accurate movements.
 */
void dilemma_set_pointer_sniping_enabled(bool enable);

/** \brief Whether drag-scroll is enabled. */
bool dilemma_get_pointer_dragscroll_enabled(void);

/**
 * \brief Enable/disable drag-scroll mode.
 *
 * When drag-scroll mode is enabled, horizontal and vertical pointer movements
 * are translated into horizontal and vertical scroll movements.
 */
void dilemma_set_pointer_dragscroll_enabled(bool enable);

// Gesture state accessors
bool dilemma_get_single_tap(void);
bool dilemma_get_two_finger_tap(void);
bool dilemma_get_swipe_right(void);
bool dilemma_get_swipe_left(void);
bool dilemma_get_swipe_up(void);
bool dilemma_get_swipe_down(void);
bool dilemma_get_press_and_hold(void);
#endif // POINTING_DEVICE_ENABLE
