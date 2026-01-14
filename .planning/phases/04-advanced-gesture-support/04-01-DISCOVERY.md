# Advanced Gesture Support Discovery

## Executive Summary

This document provides the technical foundation for Phase 4 (Advanced Gesture Support) implementation. It documents current firmware gesture capabilities, specifies requirements for distinguishing 3-finger vs 4-finger swipes, outlines pinch-to-zoom detection algorithms, and provides an implementation strategy for all 5 remaining plans in Phase 4.

**Purpose:** Enable advanced gestures (3/4-finger swipe distinction, pinch-to-zoom) through VIA configuration interface while maintaining backward compatibility and following established Phase 3 patterns.

---

## Current Firmware Capabilities

### 1. Swipe Detection in digitizer_mouse_fallback.c

**Location:** `quantum/digitizer_mouse_fallback.c` (lines 174-344)

**State Machine:**
```c
typedef enum {
    None,           // No gesture in progress
    Down,           // Finger(s) on surface
    MoveScroll,     // Moving or scrolling
    Tapped,         // Tap detected
    DoubleTapped,   // Double-tap detected
    Drag,           // Dragging
    Swipe,          // Multi-finger swipe (currently 3+ fingers)
    Finished        // Gesture complete, waiting for lift
} State;
```

**Swipe State Entry (lines 233-234, 251-252):**
```c
// From Down state
if (contacts >= 3) {
    state = Swipe;
}

// From MoveScroll state
if (contacts == 3 && duration < DIGITIZER_MOUSE_SWIPE_TIMEOUT) {
    state = Swipe;
}
```

**Current Swipe Detection Logic (lines 284-310):**
```c
case Swipe: {
    const int32_t distance_x = x - contact_start_x;
    const int32_t distance_y = y - contact_start_y;
    if (contacts == 0) {
        state = None;
    } else if (duration > DIGITIZER_MOUSE_SWIPE_TIMEOUT) {
        state = MoveScroll;
    } else if (digitizer_send_mouse_reports) {
        if (distance_x > DIGITIZER_MOUSE_SWIPE_DISTANCE && abs(distance_y) < DIGITIZER_MOUSE_SWIPE_THRESHOLD) {
            // Swipe right
            tap_code(DIGITIZER_SWIPE_RIGHT_KC);
            state = Finished;
        } else if (distance_x < -DIGITIZER_MOUSE_SWIPE_DISTANCE && abs(distance_y) < DIGITIZER_MOUSE_SWIPE_THRESHOLD) {
            // Swipe left
            tap_code(DIGITIZER_SWIPE_LEFT_KC);
            state = Finished;
        } else if (distance_y > DIGITIZER_MOUSE_SWIPE_DISTANCE && abs(distance_x) < DIGITIZER_MOUSE_SWIPE_THRESHOLD) {
            // Swipe down
            tap_code(DIGITIZER_SWIPE_DOWN_KC);
            state = Finished;
        } else if (distance_y < -DIGITIZER_MOUSE_SWIPE_DISTANCE && abs(distance_x) < DIGITIZER_MOUSE_SWIPE_THRESHOLD) {
            // Swipe up
            tap_code(DIGITIZER_SWIPE_UP_KC);
            state = Finished;
        }
    }
    break;
}
```

**Swipe Detection Parameters (config.h overrides):**
- `DIGITIZER_MOUSE_SWIPE_DISTANCE`: 500 (minimum distance in pixels)
- `DIGITIZER_MOUSE_SWIPE_THRESHOLD`: 300 (maximum perpendicular movement)
- `DIGITIZER_MOUSE_SWIPE_TIMEOUT`: 1000ms (time window for swipe detection)

**Key Characteristics:**
- Detects 4 directions: left, right, up, down
- Direction determined by primary axis of movement (x vs y)
- Distance must exceed threshold on primary axis
- Perpendicular movement must stay below threshold (prevents diagonal swipes)
- Timeout prevents holding Swipe state indefinitely

---

### 2. Current Keycode Mappings

**Default Swipe Keycodes (digitizer_mouse_fallback.c:43-57):**
```c
#ifndef DIGITIZER_SWIPE_LEFT_KC
#define DIGITIZER_SWIPE_LEFT_KC QK_MOUSE_BUTTON_3
#endif

#ifndef DIGITIZER_SWIPE_RIGHT_KC
#define DIGITIZER_SWIPE_RIGHT_KC QK_MOUSE_BUTTON_4
#endif

#ifndef DIGITIZER_SWIPE_UP_KC
#define DIGITIZER_SWIPE_UP_KC KC_LEFT_GUI
#endif

#ifndef DIGITIZER_SWIPE_DOWN_KC
#define DIGITIZER_SWIPE_DOWN_KC KC_ESC
#endif
```

**Current Default Mappings:**
- Swipe Left: Mouse Button 3 (middle click)
- Swipe Right: Mouse Button 4 (back/forward)
- Swipe Up: Left GUI (Windows/Command key)
- Swipe Down: ESC

**Override Mechanism:**
- Can be overridden in `keyboards/bastardkb/dilemma/config.h`
- No runtime configuration (compile-time only)
- Phase 4 will add VIA configuration for runtime customization

---

### 3. Finger Tracking Logic

**Contact Counting (digitizer_mouse_fallback.c:209-213):**
```c
int contacts = 0;
for (int i = 0; i < DIGITIZER_FINGER_COUNT; i++) {
    if (report->fingers[i].tip) {
        contacts++;
    }
}
```

**Current Finger Count Logic:**
- Variable: `contacts` (counts active fingers with tip=true)
- Swipe state entry: `contacts >= 3` (3 or more fingers)
- **Gap:** Does NOT distinguish between 3 and 4 fingers
- All multi-finger swipes (3, 4, 5+) treated identically

**Contact State Tracking:**
- `tap_contacts`: Maximum finger count seen during gesture
- `last_contacts`: Previous frame's finger count
- Used for tap detection and state transitions

---

### 4. EEPROM Layout (Already Reserved in Phase 1)

**Location:** `keyboards/bastardkb/dilemma/dilemma.h:109-113`

```c
// Byte 7: Advanced gesture enables
uint8_t three_finger_swipe_enabled : 1; // App switcher
uint8_t four_finger_swipe_enabled : 1;  // Desktop spaces
uint8_t pinch_to_zoom_enabled : 1;      // Smart zoom
uint8_t reserved_7 : 5;                // Future advanced gestures
```

**Gesture-to-Keycode Mappings (dilemma.h:115-122):**
```c
// Bytes 8-19: Gesture-to-keycode mappings (12 bytes)
// Each gesture maps to a 16-bit keycode
uint16_t tap_to_click_keycode;         // Byte 8-9
uint16_t two_finger_tap_keycode;       // Byte 10-11
uint16_t two_finger_scroll_up_keycode; // Byte 12-13
uint16_t two_finger_scroll_down_keycode; // Byte 14-15
uint16_t press_and_hold_keycode;       // Byte 16-17
uint16_t swipe_keycode;                // Byte 18-19 (CURRENTLY SINGLE MAPPING)
```

**EEPROM Usage Status:**
- Byte 7: Advanced gesture enables reserved (not yet configurable via VIA)
- Bytes 8-19: Gesture keycodes reserved
- **Gap:** Byte 18-19 has single `swipe_keycode` field
- **Issue:** No separation between 3-finger and 4-finger swipe keycodes

---

## Summary of Current Implementation

**What Works:**
- ✅ Swipe detection for 3+ finger gestures
- ✅ 4-directional swipe recognition (left, right, up, down)
- ✅ Distance, threshold, and timeout-based detection
- ✅ Configurable keycodes (compile-time via config.h)
- ✅ EEPROM space reserved for advanced gestures

**What's Missing:**
- ❌ No distinction between 3-finger and 4-finger swipes
- ❌ Single keycode mapping for all multi-finger swipes
- ❌ No pinch-to-zoom detection
- ❌ No VIA configuration for advanced gestures
- ❌ No runtime keycode customization

**Phase 4 Scope:**
1. Add 3-finger vs 4-finger swipe distinction
2. Add separate keycode mappings for each swipe type
3. Implement pinch-to-zoom gesture detection
4. Add VIA configuration UI for all advanced gestures
5. Maintain backward compatibility with existing defaults

---

**Next Section:** Requirements for 3 vs 4 Finger Swipe Distinction
