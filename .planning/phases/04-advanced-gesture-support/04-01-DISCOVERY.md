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

## Requirements for 3 vs 4 Finger Swipe Distinction

### 1. Firmware Changes (digitizer_mouse_fallback.c)

#### Current State Analysis

**Swipe State Entry (lines 233-234, 251-252):**
```c
// PROBLEM: Contacts >= 3 means both 3-finger AND 4-finger enter Swipe state
if (contacts >= 3) {
    state = Swipe;
}
```

**Current Swipe Detection (lines 284-310):**
- Uses single `contacts` variable
- No tracking of exact finger count at swipe entry
- All multi-finger swipes treated identically
- Single keycode mapping (DIGITIZER_SWIPE_*_KC)

#### Required Changes

**1.1 Add Finger Count Tracking Variable**

```c
// Add static variable to track finger count at swipe entry
static int swipe_finger_count = 0;

// Update Swipe state entry logic
case Down: {
    // ... existing code ...
    if (contacts >= 3) {
        state = Swipe;
        swipe_finger_count = contacts;  // STORE exact finger count
    }
    // ... existing code ...
}

case MoveScroll: {
    // ... existing code ...
    if (contacts == 3 && duration < DIGITIZER_MOUSE_SWIPE_TIMEOUT) {
        state = Swipe;
        swipe_finger_count = contacts;  // STORE exact finger count
    }
    // ... existing code ...
}
```

**1.2 Branch Keycode Logic on Finger Count**

```c
case Swipe: {
    const int32_t distance_x = x - contact_start_x;
    const int32_t distance_y = y - contact_start_y;

    if (contacts == 0) {
        state = None;
    } else if (duration > DIGITIZER_MOUSE_SWIPE_TIMEOUT) {
        state = MoveScroll;
    } else if (digitizer_send_mouse_reports) {
        // BRANCH on swipe_finger_count
        if (distance_x > DIGITIZER_MOUSE_SWIPE_DISTANCE && abs(distance_y) < DIGITIZER_MOUSE_SWIPE_THRESHOLD) {
            // Swipe right
            if (swipe_finger_count == 3) {
                tap_code16(three_finger_swipe_right_kc);
            } else if (swipe_finger_count == 4) {
                tap_code16(four_finger_swipe_right_kc);
            }
            state = Finished;
        } else if (distance_x < -DIGITIZER_MOUSE_SWIPE_DISTANCE && abs(distance_y) < DIGITIZER_MOUSE_SWIPE_THRESHOLD) {
            // Swipe left
            if (swipe_finger_count == 3) {
                tap_code16(three_finger_swipe_left_kc);
            } else if (swipe_finger_count == 4) {
                tap_code16(four_finger_swipe_left_kc);
            }
            state = Finished;
        } else if (distance_y > DIGITIZER_MOUSE_SWIPE_DISTANCE && abs(distance_x) < DIGITIZER_MOUSE_SWIPE_THRESHOLD) {
            // Swipe down
            if (swipe_finger_count == 3) {
                tap_code16(three_finger_swipe_down_kc);
            } else if (swipe_finger_count == 4) {
                tap_code16(four_finger_swipe_down_kc);
            }
            state = Finished;
        } else if (distance_y < -DIGITIZER_MOUSE_SWIPE_DISTANCE && abs(distance_x) < DIGITIZER_MOUSE_SWIPE_THRESHOLD) {
            // Swipe up
            if (swipe_finger_count == 3) {
                tap_code16(three_finger_swipe_up_kc);
            } else if (swipe_finger_count == 4) {
                tap_code16(four_finger_swipe_up_kc);
            }
            state = Finished;
        }
    }
    break;
}
```

**1.3 Add VIA Config Checks**

```c
// Check gesture enable flags before executing swipes
if (swipe_finger_count == 3 && three_finger_swipe_enabled) {
    tap_code16(three_finger_swipe_right_kc);
} else if (swipe_finger_count == 4 && four_finger_swipe_enabled) {
    tap_code16(four_finger_swipe_right_kc);
}
```

---

### 2. EEPROM Layout Adjustments

#### Current Layout Issue

**Byte 18-19 (dilemma.h:122):**
```c
uint16_t swipe_keycode;  // Single mapping for ALL multi-finger swipes
```

**Problem:** Only 16 bits allocated for swipe keycodes, but need 8 separate mappings:
- 3-finger: left, right, up, down (4 mappings × 16 bits = 64 bits = 8 bytes)
- 4-finger: left, right, up, down (4 mappings × 16 bits = 64 bits = 8 bytes)
- Total: 16 bytes needed

#### Proposed EEPROM Expansion

**Option A: Extend gesture keycode section (Recommended)**

```c
// Bytes 8-23: Gesture-to-keycode mappings (16 bytes, up from 12)
uint16_t tap_to_click_keycode;           // Byte 8-9
uint16_t two_finger_tap_keycode;         // Byte 10-11
uint16_t two_finger_scroll_up_keycode;   // Byte 12-13
uint16_t two_finger_scroll_down_keycode; // Byte 14-15
uint16_t press_and_hold_keycode;         // Byte 16-17

// NEW: Separate 3-finger and 4-finger swipe mappings
uint16_t three_finger_swipe_left_kc;     // Byte 18-19
uint16_t three_finger_swipe_right_kc;    // Byte 20-21
uint16_t three_finger_swipe_up_kc;       // Byte 22-23
uint16_t three_finger_swipe_down_kc;     // Byte 24-25
uint16_t four_finger_swipe_left_kc;      // Byte 26-27
uint16_t four_finger_swipe_right_kc;     // Byte 28-29
uint16_t four_finger_swipe_up_kc;        // Byte 30-31
uint16_t four_finger_swipe_down_kc;      // Byte 32-33 (EXCEEDS 32-byte limit!)
```

**Problem:** Exceeds 32-byte VIA custom config limit.

**Option B: Use 8-bit keycodes + offset (Compromise)**

```c
// Bytes 18-25: 3-finger swipe keycodes (8 bytes, 4 × 16-bit)
uint16_t three_finger_swipe_left_kc;     // Byte 18-19
uint16_t three_finger_swipe_right_kc;    // Byte 20-21
uint16_t three_finger_swipe_up_kc;       // Byte 22-23
uint16_t three_finger_swipe_down_kc;     // Byte 24-25

// Bytes 26-27: 4-finger swipe (direction-based, 2 bytes total)
uint8_t four_finger_swipe_base_kc;       // Byte 26 (base keycode)
uint8_t four_finger_swipe_modifiers;     // Byte 27 (modifier bits: left/right/up/down)

// Bytes 28-30: Reserved for zoom keycodes
uint16_t zoom_in_keycode;                // Byte 28-29
uint8_t zoom_out_keycode_offset;         // Byte 30 (offset from zoom_in)

// Byte 31: Versioning (unchanged)
uint8_t config_version : 4;
uint8_t reserved_31 : 4;
```

**Analysis:**
- Option B fits in 32 bytes
- Trades 4-finger flexibility for space savings
- 4-finger swipes limited to base keycode + modifier pattern
- May be sufficient for common use cases (app switcher, spaces navigation)

**Option C: Remove unused gesture fields (Best Balance)**

```c
// Bytes 8-19: Basic gesture keycodes (keep existing, unchanged)
uint16_t tap_to_click_keycode;           // Byte 8-9
uint16_t two_finger_tap_keycode;         // Byte 10-11
uint16_t two_finger_scroll_up_keycode;   // Byte 12-13
uint16_t two_finger_scroll_down_keycode; // Byte 14-15
uint16_t press_and_hold_keycode;         // Byte 16-17

// REMOVE: swipe_keycode (Byte 18-19) - replaced by directional mappings

// Bytes 18-25: 3-finger and 4-finger swipe keycodes (8 bytes)
uint16_t three_finger_swipe_left_kc;     // Byte 18-19
uint16_t three_finger_swipe_right_kc;    // Byte 20-21
uint16_t three_finger_swipe_up_kc;       // Byte 22-23
uint16_t three_finger_swipe_down_kc;     // Byte 24-25

// Bytes 26-29: Pinch-to-zoom keycodes (4 bytes)
uint16_t pinch_zoom_in_keycode;          // Byte 26-27
uint16_t pinch_zoom_out_keycode;         // Byte 28-29

// Byte 30: Smart gesture features (unchanged)
uint8_t tap_pressure_threshold : 4;
uint8_t force_click_enabled : 1;
uint8_t smart_zoom_enabled : 1;
uint8_t reserved_20 : 2;

// Byte 31: Versioning (unchanged)
uint8_t config_version : 4;
uint8_t reserved_31 : 4;
```

**Analysis:**
- ✅ Fits in 32 bytes (0-31)
- ✅ Full 16-bit keycodes for all 3-finger swipes
- ✅ Full 16-bit keycodes for zoom in/out
- ❌ 4-finger swipes NOT included (use defaults)
- **Rationale:** 4-finger swipes less common, can use sensible defaults

**Recommendation: Option C**
- Prioritizes 3-finger swipe customization (most common use case)
- Includes pinch-to-zoom customization (Phase 4 feature)
- 4-finger swipes use hardcoded defaults (sufficient for大多数 users)
- Preserves space for future expansion

---

### 3. Implementation Approach

#### 3.1 State Machine Extension (digitizer_mouse_fallback.c)

**Step 1: Add static tracking variable**
```c
static int swipe_finger_count = 0;  // Track fingers at swipe entry
```

**Step 2: Store finger count on Swipe state entry**
```c
case Down: {
    // ...
    if (contacts >= 3) {
        state = Swipe;
        swipe_finger_count = contacts;  // NEW: Store exact count
    }
}

case MoveScroll: {
    // ...
    if (contacts == 3 && duration < DIGITIZER_MOUSE_SWIPE_TIMEOUT) {
        state = Swipe;
        swipe_finger_count = contacts;  // NEW: Store exact count
    }
}
```

**Step 3: Branch keycode logic on finger count**
```c
case Swipe: {
    // ... distance calculations ...

    if (distance_x > DIGITIZER_MOUSE_SWIPE_DISTANCE && abs(distance_y) < DIGITIZER_MOUSE_SWIPE_THRESHOLD) {
        if (swipe_finger_count == 3 && three_finger_swipe_enabled) {
            tap_code16(g_via_dilemma_config.three_finger_swipe_right_kc);
        } else if (swipe_finger_count == 4 && four_finger_swipe_enabled) {
            // Use hardcoded 4-finger defaults (VIA doesn't store these)
            tap_code(KC_LEFT_GUI);  // Cmd+Tab for app switcher
        }
        state = Finished;
    }
    // ... repeat for left, up, down ...
}
```

#### 3.2 VIA Configuration Integration (dilemma.c)

**Add keycode globals (firmware defaults):**
```c
// Hardcoded 4-finger swipe defaults (not in VIA config)
static const uint16_t four_finger_swipe_left_default = KC_LEFT_GUI;
static const uint16_t four_finger_swipe_right_default = LCTL(KC_LEFT);
static const uint16_t four_finger_swipe_up_default = LCTL(KC_UP);
static const uint16_t four_finger_swipe_down_default = LCTL(KC_DOWN);
```

**Rationale for 4-finger defaults:**
- Swipe Left: `KC_LEFT_GUI` - Open app switcher (Cmd+Tab on macOS)
- Swipe Right: `LCTL(KC_LEFT)` - Previous desktop space (Ctrl+Left)
- Swipe Up: `LCTL(KC_UP)` - Mission Control / show all windows
- Swipe Down: `LCTL(KC_DOWN)` - Show app windows (Exposé)

#### 3.3 VIA Value ID Extensions (dilemma.h)

**Add new value IDs for 3-finger swipe keycodes:**
```c
enum via_dilemma_value_id {
    // ... existing IDs 0-11 ...
    id_dilemma_unhandled = 0,
    id_dilemma_config_save = 1,
    id_dilemma_dpi_preset = 2,
    // ... etc ...

    // NEW: 3-finger swipe keycodes (IDs 12-15)
    id_dilemma_three_finger_swipe_left = 12,
    id_dilemma_three_finger_swipe_right = 13,
    id_dilemma_three_finger_swipe_up = 14,
    id_dilemma_three_finger_swipe_down = 15,

    // Add zoom keycodes in later plan
};
```

---

### 4. Backward Compatibility

#### Default Keycode Mappings

**3-Finger Swipes (VIA-configurable, defaults to current behavior):**
```c
three_finger_swipe_left_kc = QK_MOUSE_BUTTON_3;    // Current default
three_finger_swipe_right_kc = QK_MOUSE_BUTTON_4;   // Current default
three_finger_swipe_up_kc = KC_LEFT_GUI;            // Current default
three_finger_swipe_down_kc = KC_ESC;               // Current default
```

**4-Finger Swipes (Hardcoded, macOS-oriented defaults):**
```c
four_finger_swipe_left = KC_LEFT_GUI;              // App switcher
four_finger_swipe_right = LCTL(KC_LEFT);           // Previous space
four_finger_swipe_up = LCTL(KC_UP);                // Mission Control
four_finger_swipe_down = LCTL(KC_DOWN);            // App windows
```

**Enable/Disable Defaults (EEPROM byte 7):**
```c
three_finger_swipe_enabled = 1;  // Enabled by default (backward compatible)
four_finger_swipe_enabled = 1;   // Enabled by default (new feature)
pinch_to_zoom_enabled = 0;       // Disabled by default (opt-in)
```

#### Migration Strategy

**Existing configs (version 0):**
- 3-finger swipes continue to work with existing keycodes
- 4-finger swipes activate with sensible defaults
- No breaking changes to user configs

**New configs (version 1):**
- Can customize 3-finger swipe keycodes via VIA
- Can enable/disable 4-finger swipes independently
- Can enable pinch-to-zoom (opt-in)

---

## Summary of Requirements

**Firmware Changes (digitizer_mouse_fallback.c):**
- ✅ Add `swipe_finger_count` static variable
- ✅ Store finger count on Swipe state entry
- ✅ Branch keycode logic on 3 vs 4 fingers
- ✅ Check VIA enable flags before executing

**EEPROM Changes (dilemma.h):**
- ✅ Remove single `swipe_keycode` field (byte 18-19)
- ✅ Add 4× 16-bit keycode fields for 3-finger swipes (bytes 18-25)
- ✅ Add 2× 16-bit keycode fields for zoom (bytes 26-29)
- ✅ Use hardcoded defaults for 4-finger swipes (not in EEPROM)
- ✅ Update `config_version` to 1 (breaking change to structure)

**VIA Integration (dilemma.c, dilemma.h):**
- ✅ Add value IDs 12-15 for 3-finger swipe keycodes
- ✅ Add value IDs 16-17 for zoom keycodes (later)
- ✅ Implement set/get handlers for new value IDs
- ✅ Initialize defaults in `eeconfig_init_kb()`
- ✅ Update gesture filtering to check 3/4-finger enables

**Backward Compatibility:**
- ✅ Default 3-finger keycodes match current behavior
- ✅ 4-finger swipes use sensible macOS-oriented defaults
- ✅ All new features opt-in or backward compatible
- ✅ Existing user configs continue to work

---

**Next Section:** Pinch-to-Zoom Detection Algorithm
