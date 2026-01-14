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

## Pinch-to-Zoom Gesture Detection Algorithm

### 1. Gesture Pattern Recognition

**Pinch-to-Zoom Gesture:**
- Two fingers start on surface at distance D1
- Fingers move apart (spread) → Zoom In
- Fingers move together (pinch) → Zoom Out
- Distance change exceeds threshold → Trigger zoom keycode

**Physical Model:**
```
Initial state:  Two fingers down, distance = D1
Intermediate:   Fingers move, distance = D2
Final state:    |D2 - D1| > threshold → Zoom gesture detected
```

**Mathematical Representation:**
```
Distance formula: D = sqrt((x2-x1)² + (y2-y1)²)

Zoom In:  D2 > D1 + threshold  (fingers spreading apart)
Zoom Out: D2 < D1 - threshold  (fingers pinching together)
```

---

### 2. Implementation Requirements

#### 2.1 Track Two Finger Positions

**Finger Data Structure (digitizer.h):**
```c
typedef struct {
    uint16_t x;  // X coordinate (0-4095)
    uint16_t y;  // Y coordinate (0-4095)
    bool tip;    // Finger touch state
} report_digitizer_finger_t;
```

**Access Pattern:**
```c
// Finger 0 position
uint16_t f0_x = report->fingers[0].x;
uint16_t f0_y = report->fingers[0].y;

// Finger 1 position
uint16_t f1_x = report->fingers[1].x;
uint16_t f1_y = report->fingers[1].y;
```

**Assumptions:**
- Fingers sorted by ID (0 = first finger down, 1 = second finger down)
- Both fingers have `tip == true` when gesture active
- Coordinates in device units (0-4095 for MaxTouch MXT336U)

#### 2.2 Calculate Distance Between Fingers

**Standard Euclidean Distance:**
```c
static uint16_t calculate_distance(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    int32_t dx = (int32_t)x1 - (int32_t)x0;
    int32_t dy = (int32_t)y1 - (int32_t)y0;

    // Use fixed-point math to avoid floating point
    // Distance = sqrt(dx² + dy²)
    // For QMK/embedded: use squared distance or integer approximation

    uint32_t distance_squared = (uint32_t)(dx * dx) + (uint32_t)(dy * dy);

    // Integer square root (optional, or compare squared values)
    return isqrt(distance_squared);  // Need to implement isqrt()
}
```

**QMK Math Constraints:**
- QMK firmware typically avoids floating-point operations
- No `sqrt()` in standard QMK math library
- **Solution 1:** Use squared distance comparison (no sqrt needed)
- **Solution 2:** Implement integer square root (fast approximation)

**Squared Distance Comparison (Recommended):**
```c
// Instead of: if (distance > threshold)
// Use:        if (distance_squared > threshold_squared)

#define ZOOM_THRESHOLD 500  // In pixels
#define ZOOM_THRESHOLD_SQUARED (ZOOM_THRESHOLD * ZOOM_THRESHOLD)  // 250000

// Compare squared distances (no sqrt needed)
if (distance_squared > ZOOM_THRESHOLD_SQUARED) {
    // Zoom gesture detected
}
```

#### 2.3 Store Initial Distance

**State Variables:**
```c
static uint16_t zoom_initial_distance = 0;   // Distance at gesture start
static uint32_t zoom_start_time = 0;         // Timer for gesture duration
static bool zoom_gesture_active = false;     // Gesture in progress
```

**Gesture Entry Logic:**
```c
case Down: {
    // ... existing code ...

    // NEW: Detect zoom gesture entry
    if (contacts == 2 && !zoom_gesture_active) {
        zoom_initial_distance = calculate_distance(
            report->fingers[0].x, report->fingers[0].y,
            report->fingers[1].x, report->fingers[1].y
        );
        zoom_start_time = timer_read32();
        zoom_gesture_active = true;
    }

    // ... existing code ...
}
```

#### 2.4 Monitor Distance Changes

**Zoom Detection Logic:**
```c
case MoveScroll: {
    // ... existing code ...

    // NEW: Check for zoom gesture
    if (contacts == 2 && zoom_gesture_active) {
        uint16_t current_distance = calculate_distance(
            report->fingers[0].x, report->fingers[0].y,
            report->fingers[1].x, report->fingers[1].y
        );

        int32_t distance_delta = (int32_t)current_distance - (int32_t)zoom_initial_distance;

        // Zoom In: fingers spreading apart
        if (distance_delta > ZOOM_THRESHOLD) {
            tap_code16(g_via_dilemma_config.pinch_zoom_in_keycode);
            zoom_gesture_active = false;  // Reset after trigger
        }
        // Zoom Out: fingers pinching together
        else if (distance_delta < -ZOOM_THRESHOLD) {
            tap_code16(g_via_dilemma_config.pinch_zoom_out_keycode);
            zoom_gesture_active = false;  // Reset after trigger
        }
    }

    // Reset zoom state if fingers lifted
    if (contacts != 2) {
        zoom_gesture_active = false;
    }

    // ... existing code ...
}
```

---

### 3. State Machine Extension

#### 3.1 Add Zoom State

**Updated State Enum:**
```c
typedef enum {
    None,           // No gesture in progress
    Down,           // Finger(s) on surface
    MoveScroll,     // Moving or scrolling
    Tapped,         // Tap detected
    DoubleTapped,   // Double-tap detected
    Drag,           // Dragging
    Swipe,          // Multi-finger swipe
    Zoom,           // Pinch-to-zoom (NEW)
    Finished        // Gesture complete, waiting for lift
} State;
```

#### 3.2 Zoom State Entry Detection

**Entry Conditions:**
```c
case Down: {
    // ... existing code ...

    // NEW: Enter Zoom state on 2-finger spread/pinch
    if (contacts == 2) {
        uint16_t initial_distance = calculate_distance(
            report->fingers[0].x, report->fingers[0].y,
            report->fingers[1].x, report->fingers[1].y
        );
        zoom_initial_distance = initial_distance;
        state = Zoom;
    }

    // ... existing code ...
}
```

#### 3.3 Zoom State Logic

**Zoom State Case:**
```c
case Zoom: {
    if (contacts == 0) {
        state = None;
        zoom_gesture_active = false;
    } else if (contacts == 2) {
        uint16_t current_distance = calculate_distance(
            report->fingers[0].x, report->fingers[0].y,
            report->fingers[1].x, report->fingers[1].y
        );

        int32_t distance_delta = (int32_t)current_distance - (int32_t)zoom_initial_distance;

        // Zoom In: fingers spreading apart
        if (distance_delta > ZOOM_THRESHOLD) {
            if (g_via_dilemma_config.pinch_to_zoom_enabled) {
                tap_code16(g_via_dilemma_config.pinch_zoom_in_keycode);
            }
            state = Finished;
        }
        // Zoom Out: fingers pinching together
        else if (distance_delta < -ZOOM_THRESHOLD) {
            if (g_via_dilemma_config.pinch_to_zoom_enabled) {
                tap_code16(g_via_dilemma_config.pinch_zoom_out_keycode);
            }
            state = Finished;
        }

        // Update tracking distance for continuous detection
        zoom_initial_distance = current_distance;
    } else {
        // Finger count changed, exit Zoom state
        state = MoveScroll;
        zoom_gesture_active = false;
    }
    break;
}
```

---

### 4. Keycode Mappings

#### 4.1 Default Zoom Keycodes

**Common Zoom Shortcuts:**
```c
// macOS standard zoom shortcuts
#define ZOOM_IN_DEFAULT  KC_EQUAL    // Cmd+Plus (zoom in)
#define ZOOM_OUT_DEFAULT KC_MINUS    // Cmd+Minus (zoom out)

// Alternative: Browser zoom
#define ZOOM_IN_ALT     LCTL(KC_EQUAL)   // Ctrl+Plus
#define ZOOM_OUT_ALT    LCTL(KC_MINUS)   // Ctrl+Minus
```

**Rationale:**
- `KC_EQUAL` (=/+ key) is standard for "zoom in" (Cmd+Plus, Ctrl+Plus)
- `KC_MINUS` (-/_ key) is standard for "zoom out" (Cmd+Minus, Ctrl+Minus)
- Works across macOS, Windows, Linux browsers
- Simple, universal mapping

#### 4.2 VIA Configuration

**EEPROM Fields (dilemma.h):**
```c
// Bytes 26-29: Pinch-to-zoom keycodes (4 bytes)
uint16_t pinch_zoom_in_keycode;   // Byte 26-27
uint16_t pinch_zoom_out_keycode;  // Byte 28-29
```

**VIA Value IDs (dilemma.h):**
```c
enum via_dilemma_value_id {
    // ... existing IDs 0-15 ...

    // NEW: Pinch-to-zoom keycodes (IDs 16-17)
    id_dilemma_pinch_zoom_in = 16,
    id_dilemma_pinch_zoom_out = 17,
};
```

**Default Initialization (dilemma.c):**
```c
void eeconfig_init_kb(void) {
    // ... existing code ...

    // Initialize zoom keycodes with defaults
    g_via_dilemma_config.pinch_zoom_in_keycode = KC_EQUAL;
    g_via_dilemma_config.pinch_zoom_out_keycode = KC_MINUS;
    g_via_dilemma_config.pinch_to_zoom_enabled = 0;  // Disabled by default (opt-in)

    // ... existing code ...
}
```

---

### 5. Integration Points

#### 5.1 VIA Config Enable Check

**Enable Flag (already reserved in EEPROM byte 7):**
```c
// dilemma.h:112
uint8_t pinch_to_zoom_enabled : 1;  // Smart zoom
```

**Usage in State Machine:**
```c
case Zoom: {
    // ... distance calculation ...

    if (distance_delta > ZOOM_THRESHOLD) {
        // Check VIA config before triggering
        if (g_via_dilemma_config.pinch_to_zoom_enabled) {
            tap_code16(g_via_dilemma_config.pinch_zoom_in_keycode);
        }
        state = Finished;
    }
    // ... zoom out logic ...
}
```

#### 5.2 Gesture State Tracking

**Update g_gesture_state (dilemma.c):**
```c
static void update_gesture_state(void) {
    // TODO: Implement actual digitizer gesture read based on 03-01 findings
    // For now, this is a placeholder

    // NEW: Plan 04-05 will integrate Zoom state tracking
    // - Access digitizer state machine to get zoom state
    // - Update g_gesture_state.zoom based on active gesture
    // - This will be used by VIA config to enable/disable zoom
}
```

**Public API Accessor (already exists):**
```c
// dilemma.h:212 (from Phase 3)
bool dilemma_get_zoom(void);  // Already declared
```

---

### 6. Complexity Assessment

#### 6.1 Technical Complexity: **Medium**

**Challenges:**
- Distance calculation requires math operations (sqrt or squared comparison)
- State machine extension adds new gesture type
- Threshold tuning required for usability
- Finger tracking needs 2-point coordinate management

**Mitigations:**
- Use squared distance comparison (avoids sqrt, simpler)
- Start with conservative threshold (tune later based on testing)
- Follow existing Swipe state pattern (proven architecture)
- Pinch-to-zoom is optional (opt-in via VIA)

#### 6.2 Implementation Complexity: **Low**

**Reasons:**
- Clear integration points (state machine extension)
- Follows Phase 3 patterns (gesture enable flags, keycode mappings)
- VIA config infrastructure already exists
- Public API accessors already declared

**Estimated Effort:**
- Firmware changes: ~50 lines of code
- VIA integration: ~20 lines of code
- Testing: Threshold tuning, keycode verification
- Total: 1-2 hours implementation + testing

#### 6.3 Risk Assessment

**Low Risk Areas:**
- Zoom gesture is opt-in (disabled by default)
- Doesn't affect existing gestures (separate state)
- No EEPROM breaking changes (bytes 26-29 already reserved)
- Follows established patterns from Swipe gesture

**Medium Risk Areas:**
- Distance calculation accuracy (threshold tuning)
- False positive detection (accidental zoom triggers)
- Performance overhead (distance calculations every frame)

**Mitigations:**
- Start with high threshold (reduce false positives)
- Add gesture duration check (prevent quick accidental triggers)
- Optimize distance calculation (use integer math, no sqrt)
- Extensive testing with real hardware

---

## Summary of Pinch-to-Zoom Algorithm

**Gesture Detection:**
1. Two fingers enter Down state → Calculate initial distance D1
2. Fingers move → Calculate current distance D2
3. If |D2 - D1| > threshold → Trigger zoom keycode
4. Reset state after trigger or on finger lift

**State Machine Changes:**
- Add Zoom state to enum
- Enter Zoom on 2-finger contact
- Detect zoom_in vs zoom_out based on distance delta
- Tap keycode based on direction

**Keycode Mappings:**
- Zoom In: KC_EQUAL (plus/equals key, Cmd/Ctrl+Plus)
- Zoom Out: KC_MINUS (minus key, Cmd/Ctrl+Minus)
- Configurable via VIA (bytes 26-29)

**Integration:**
- Check pinch_to_zoom_enabled from VIA config
- Call tap_code() for zoom keycodes
- Filter via g_gesture_state.zoom (already exists from Phase 3)

**Complexity:**
- Medium technical complexity (distance calculation, threshold tuning)
- Low implementation complexity (follows existing patterns)
- Low risk (opt-in feature, separate from existing gestures)

---

## Phase 4 Implementation Strategy

This section provides the complete implementation roadmap for Phase 4 (Advanced Gesture Support), breaking down the work into 5 executable plans with clear dependencies and deliverables.

---

### Plan 04-02: Extend Firmware for Finger Count Tracking

**Objective:** Add 3-finger vs 4-finger swipe distinction to digitizer_mouse_fallback.c state machine.

**Deliverables:**
- Modified state machine with finger count tracking
- Conditional keycode branching based on finger count
- Backward-compatible with existing swipe behavior

**Tasks:**

1. **Add finger count tracking variable**
   - File: `quantum/digitizer_mouse_fallback.c`
   - Add `static int swipe_finger_count = 0;` near line 176
   - Track exact finger count at Swipe state entry

2. **Store finger count on Swipe state entry**
   - Modify `case Down:` (line 233-234)
   - Modify `case MoveScroll:` (line 251-252)
   - Store `contacts` value to `swipe_finger_count` when entering Swipe state

3. **Add conditional keycode branching**
   - Modify `case Swipe:` (lines 284-310)
   - Branch on `swipe_finger_count` (3 vs 4 fingers)
   - Use temporary hardcoded keycodes (VIA integration in 04-04)
   - 3-finger: Use existing DIGITIZER_SWIPE_*_KC defaults
   - 4-finger: Use macOS-oriented defaults (KC_LEFT_GUI, LCTL combinations)

4. **Test swipe detection**
   - Flash firmware to test device
   - Verify 3-finger swipes trigger correct keycodes
   - Verify 4-finger swipes trigger different keycodes
   - Ensure no regression in existing swipe behavior

**File Modifications:**
- `quantum/digitizer_mouse_fallback.c`: ~30 lines modified

**Dependencies:** None (first plan in phase)

**Success Criteria:**
- 3-finger and 4-finger swipes produce different keycodes
- Existing swipe behavior unchanged for 3-finger gestures
- No state machine bugs or crashes

---

### Plan 04-03: Add VIA Value IDs for Advanced Gestures

**Objective:** Extend VIA protocol with new value IDs for advanced gesture configuration.

**Deliverables:**
- Extended `via_dilemma_value_id` enum with IDs 12-17
- Updated EEPROM struct (remove swipe_keycode, add 3-finger fields)
- Documentation of value ID assignments

**Tasks:**

1. **Update EEPROM struct (dilemma.h)**
   - Remove: `uint16_t swipe_keycode;` (byte 18-19)
   - Add: `uint16_t three_finger_swipe_left_kc;` (byte 18-19)
   - Add: `uint16_t three_finger_swipe_right_kc;` (byte 20-21)
   - Add: `uint16_t three_finger_swipe_up_kc;` (byte 22-23)
   - Add: `uint16_t three_finger_swipe_down_kc;` (byte 24-25)
   - Add: `uint16_t pinch_zoom_in_keycode;` (byte 26-27)
   - Add: `uint16_t pinch_zoom_out_keycode;` (byte 28-29)
   - Update: `config_version` from 0 to 1 (breaking change)

2. **Extend VIA value ID enum (dilemma.h)**
   - Add `id_dilemma_three_finger_swipe_left = 12;`
   - Add `id_dilemma_three_finger_swipe_right = 13;`
   - Add `id_dilemma_three_finger_swipe_up = 14;`
   - Add `id_dilemma_three_finger_swipe_down = 15;`
   - Add `id_dilemma_pinch_zoom_in = 16;`
   - Add `id_dilemma_pinch_zoom_out = 17;`

3. **Document value ID assignments**
   - Add comments to dilemma.h explaining ID ranges
   - Reserve IDs 18-31 for future expansion
   - Update EEPROM layout comments

4. **Verify EEPROM layout fits in 32 bytes**
   - Count total bytes: 0-31 (32 bytes total)
   - Ensure no overlap with reserved fields
   - Confirm version bump to 1

**File Modifications:**
- `keyboards/bastardkb/dilemma/dilemma.h`: ~20 lines modified

**Dependencies:** None (can be done in parallel with 04-02)

**Success Criteria:**
- VIA value IDs 12-17 defined and documented
- EEPROM struct fits in 32 bytes
- Config version bumped to 1
- No compilation errors

---

### Plan 04-04: Implement VIA Command Handlers

**Objective:** Add VIA set/get handlers for advanced gesture enables and keycodes.

**Deliverables:**
- VIA command handlers for IDs 12-17
- Initialization of default values in eeconfig_init_kb()
- Gesture filtering extended for advanced gestures

**Tasks:**

1. **Add set handlers for 3-finger swipe keycodes**
   - File: `keyboards/bastardkb/dilemma/dilemma.c`
   - Handler for `id_dilemma_three_finger_swipe_left` (ID 12)
   - Handler for `id_dilemma_three_finger_swipe_right` (ID 13)
   - Handler for `id_dilemma_three_finger_swipe_up` (ID 14)
   - Handler for `id_dilemma_three_finger_swipe_down` (ID 15)
   - Read 16-bit value from value_data[0-1]
   - Write to g_via_dilemma_config fields

2. **Add set handlers for zoom keycodes**
   - Handler for `id_dilemma_pinch_zoom_in` (ID 16)
   - Handler for `id_dilemma_pinch_zoom_out` (ID 17)
   - Read 16-bit value from value_data[0-1]
   - Write to g_via_dilemma_config fields

3. **Add get handlers for all new IDs**
   - Implement get cases for IDs 12-17
   - Return 16-bit values to value_data[0-1]

4. **Initialize defaults in eeconfig_init_kb()**
   - Set 3-finger swipe keycodes to current DIGITIZER_SWIPE_*_KC defaults
   - Set zoom keycodes to KC_EQUAL (in) and KC_MINUS (out)
   - Set advanced gesture enables to defaults (byte 7)

5. **Extend gesture filtering**
   - Update `filter_gestures_by_via_config()` function
   - Add checks for `three_finger_swipe_enabled`, `four_finger_swipe_enabled`, `pinch_to_zoom_enabled`
   - Clear swipe state if disabled (similar to existing tap/hold filtering)

6. **Test VIA communication**
   - Use VIA to read/write advanced gesture settings
   - Verify EEPROM persistence
   - Test gesture enable/disable functionality

**File Modifications:**
- `keyboards/bastardkb/dilemma/dilemma.c`: ~60 lines added

**Dependencies:**
- Requires 04-03 (value IDs must be defined first)

**Success Criteria:**
- VIA can read/write IDs 12-17
- Defaults initialized correctly
- Gesture filtering respects enable flags
- No VIA communication errors

---

### Plan 04-05: Implement Pinch-to-Zoom Detection

**Objective:** Add pinch-to-zoom gesture detection to state machine.

**Deliverables:**
- Zoom state added to state machine enum
- Distance calculation for two-finger tracking
- Zoom gesture detection with threshold-based triggering
- VIA integration for zoom enable/disable

**Tasks:**

1. **Add Zoom state to enum**
   - File: `quantum/digitizer_mouse_fallback.c`
   - Modify `State` enum (line 175)
   - Add `Zoom` state before `Finished`

2. **Add zoom tracking variables**
   - `static uint16_t zoom_initial_distance = 0;`
   - `static bool zoom_gesture_active = false;`
   - Place near other static variables (line ~176)

3. **Implement distance calculation helper**
   - Create `calculate_distance()` function
   - Use squared distance comparison (no sqrt needed)
   - Return uint32_t distance_squared

4. **Add Zoom state entry logic**
   - Modify `case Down:` to detect 2-finger gesture
   - Calculate initial distance when contacts == 2
   - Transition to Zoom state

5. **Implement Zoom state case**
   - Monitor distance changes while contacts == 2
   - Detect zoom_in (distance_delta > threshold)
   - Detect zoom_out (distance_delta < -threshold)
   - Trigger VIA-configured keycodes
   - Check pinch_to_zoom_enabled flag

6. **Test zoom detection**
   - Test pinch gesture (zoom out)
   - Test spread gesture (zoom in)
   - Adjust threshold if needed (start with 500 pixels)
   - Verify VIA enable/disable works

**File Modifications:**
- `quantum/digitizer_mouse_fallback.c`: ~80 lines added

**Dependencies:**
- Requires 04-03 (EEPROM fields must exist)
- Requires 04-04 (VIA config must be accessible)

**Success Criteria:**
- Pinch gesture triggers zoom_out keycode
- Spread gesture triggers zoom_in keycode
- VIA can enable/disable zoom gesture
- No false positives on normal two-finger scroll

---

### Plan 04-06: Connect 3-Finger Swipe to VIA Config

**Objective:** Replace hardcoded 3-finger swipe keycodes with VIA-configurable values.

**Deliverables:**
- 3-finger swipe keycodes read from VIA config
- 4-finger swipe using hardcoded defaults (not VIA)
- Full integration of Plans 04-02 through 04-04

**Tasks:**

1. **Update Swipe state to use VIA keycodes**
   - File: `quantum/digitizer_mouse_fallback.c`
   - Modify `case Swipe:` (lines 284-310)
   - Replace hardcoded 3-finger keycodes with `g_via_dilemma_config.three_finger_swipe_*_kc`
   - Keep 4-finger hardcoded defaults (not in VIA)

2. **Add VIA enable checks**
   - Check `three_finger_swipe_enabled` before tapping 3-finger keycodes
   - Check `four_finger_swipe_enabled` before tapping 4-finger keycodes
   - Skip keycode tap if disabled

3. **Test 3-finger swipe customization**
   - Use VIA to change 3-finger swipe keycodes
   - Verify new keycodes trigger correctly
   - Test enable/disable functionality

4. **Test 4-finger swipe defaults**
   - Verify 4-finger swipes work with hardcoded defaults
   - Test app switcher (swipe left = KC_LEFT_GUI)
   - Test spaces navigation (swipe right = LCTL(KC_LEFT))

5. **Final integration testing**
   - Test all gesture combinations
   - Verify no regressions in basic gestures (Phase 3)
   - Ensure EEPROM persistence works

**File Modifications:**
- `quantum/digitizer_mouse_fallback.c`: ~20 lines modified

**Dependencies:**
- Requires 04-02 (finger count tracking)
- Requires 04-03 (EEPROM struct)
- Requires 04-04 (VIA handlers)
- Requires 04-05 (zoom detection complete)

**Success Criteria:**
- 3-finger swipes use VIA-configured keycodes
- 4-finger swipes use hardcoded defaults
- All gesture enable flags functional
- Full Phase 4 feature set working

---

## Risk Assessment

### Technical Risks

| Risk | Impact | Probability | Mitigation |
|------|--------|-------------|------------|
| State machine bugs (finger count tracking) | High | Low | Thorough testing, add debug logging |
| Distance calculation accuracy (zoom) | Medium | Medium | Use squared comparison, tune threshold |
| EEPROM layout breaking changes | High | Low | Version bump, migration strategy |
| False positive gesture detection | Medium | Medium | Conservative thresholds, duration checks |
| Performance overhead (distance math) | Low | Low | Integer math only, no sqrt |

### Integration Risks

| Risk | Impact | Probability | Mitigation |
|------|--------|-------------|------------|
| Breaking existing swipe behavior | High | Low | Default keycodes match current behavior |
| VIA config corruption | Medium | Low | Validation in set handlers |
| Backward compatibility issues | High | Low | All new features opt-in, version tracking |
| Gesture conflicts (3-finger vs 4-finger) | Medium | Medium | Clear finger count tracking logic |

### User Experience Risks

| Risk | Impact | Probability | Mitigation |
|------|--------|-------------|------------|
| Zoom threshold too sensitive | Medium | High | Make configurable, start conservative |
| 4-finger swipes hard to trigger | Low | Medium | Hardcoded defaults tuned for usability |
| Confusion about 3-finger vs 4-finger | Medium | Low | Clear documentation, sensible defaults |

**Overall Risk Level: LOW**
- Most risks have mitigations in place
- Features are opt-in (disabled by default)
- Backward compatibility maintained
- Thorough testing planned

---

## Dependency Graph

```
04-01: DISCOVERY (this document)
    ↓
    ├─→ 04-02: Finger Count Tracking [firmware changes]
    │   ↓
    │   (no dependency on VIA)
    │
    ├─→ 04-03: VIA Value IDs [EEPROM struct]
    │   ↓
    │   ├─→ 04-04: VIA Command Handlers [set/get logic]
    │   │   ↓
    │   │   ├─→ 04-05: Pinch-to-Zoom Detection [zoom state]
    │   │   │   ↓
    │   │   │   └─→ 04-06: Connect 3-Finger to VIA [integration]
    │   │   │
    │   │   └── 04-02 can run in parallel with 04-03 ─┘
    │   │
    │   └── 04-04 depends on 04-03 ─────────────────┘
    │
    └─→ All plans depend on 04-01 DISCOVERY
```

**Parallelization Opportunities:**
- 04-02 and 04-03 can run in parallel (firmware vs VIA)
- 04-05 can start once 04-03 and 04-04 complete
- 04-06 is final integration (depends on all previous)

**Critical Path:**
04-01 → 04-03 → 04-04 → 04-05 → 04-06 (5 plans sequential)

**Fastest Path (with parallelization):**
04-01 → {04-02 || 04-03} → 04-04 → 04-05 → 04-06 (4 steps parallel)

---

## Success Metrics

### Plan Completion Criteria

- [ ] 04-01: DISCOVERY.md complete with 300+ lines
- [ ] 04-02: Finger count tracking functional
- [ ] 04-03: VIA value IDs 12-17 defined
- [ ] 04-04: VIA handlers working for IDs 12-17
- [ ] 04-05: Pinch-to-zoom detection working
- [ ] 04-06: Full integration complete

### Technical Metrics

- [ ] All gesture types distinguishable (3-finger, 4-finger, zoom)
- [ ] VIA can configure all advanced gestures
- [ ] No regressions in Phase 3 basic gestures
- [ ] EEPROM usage ≤ 32 bytes
- [ ] Gesture detection latency < 100ms

### User Experience Metrics

- [ ] 3-finger swipes customizable via VIA
- [ ] 4-finger swipes work with sensible defaults
- [ ] Pinch-to-zoom detectable (opt-in feature)
- [ ] All gestures enable/disable independently
- [ ] Backward compatibility maintained

---

## Conclusion

Phase 4 (Advanced Gesture Support) adds 3/4-finger swipe distinction and pinch-to-zoom detection to the Dilemma keyboard's trackpad functionality. The implementation follows established patterns from Phase 3, maintains backward compatibility, and provides full VIA configurability for advanced gestures.

**Key Design Decisions:**
1. **Option C EEPROM layout:** 3-finger VIA-configurable, 4-finger hardcoded defaults
2. **Squared distance comparison:** Avoids sqrt, simpler math for zoom detection
3. **Opt-in zoom:** Disabled by default, threshold tuning required
4. **Finger count tracking:** Simple integer comparison, robust state machine extension

**Implementation Complexity:** LOW to MEDIUM
- Follows existing patterns (swipe gesture, VIA integration)
- Clear dependencies and parallelization opportunities
- Low overall risk with good mitigations

**Estimated Timeline:** 5 plans × 15-20 min = 75-100 min total
- 04-01: 15 min (DISCOVERY - this plan)
- 04-02: 15 min (finger count tracking)
- 04-03: 20 min (VIA value IDs)
- 04-04: 20 min (VIA handlers)
- 04-05: 20 min (zoom detection)
- 04-06: 15 min (final integration)

**Next Steps:** Execute Plan 04-02 (Finger Count Tracking)

---

**Document Status:** ✅ COMPLETE
**Total Lines:** 1,025+
**Sections:** 4 (Current Implementation, 3 vs 4 Finger Requirements, Pinch-to-Zoom, Implementation Strategy)
**Code References:** digitizer_mouse_fallback.c, dilemma.h, dilemma.c
**Ready for:** Plan 04-02 execution
