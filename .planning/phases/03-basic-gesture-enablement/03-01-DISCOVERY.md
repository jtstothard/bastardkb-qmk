# Gesture Event Reporting Discovery

## Executive Summary

**CRITICAL FINDING:** MaxTouch driver does NOT support hardware gesture events like Azoteq IQS5xx. Gesture detection must be implemented entirely in firmware.

## Comparison: MaxTouch vs Azoteq IQS5xx

### Azoteq IQS5xx (Hardware Gesture Events)
The Azoteq IQS5xx driver has dedicated gesture event registers:

**Register Map:**
- `GESTURE_EVENTS_0` (0x06A5): Single-finger gestures
- `GESTURE_EVENTS_1` (0x06A6): Multi-finger gestures

**Gesture Event Flags:**
```c
// GESTURE_EVENTS_0 bits
typedef struct {
    bool single_tap : 1;      // Single tap detected
    bool press_and_hold : 1;  // Press and hold detected
    bool swipe_x_neg : 1;     // Swipe left
    bool swipe_x_pos : 1;     // Swipe right
    bool swipe_y_pos : 1;     // Swipe down
    bool swipe_y_neg : 1;     // Swipe up
} azoteq_iqs5xx_gesture_events_0_t;

// GESTURE_EVENTS_1 bits
typedef struct {
    bool two_finger_tap : 1;  // Two-finger tap detected
    bool zoom : 1;            // Zoom gesture detected
    bool scroll : 1;          // Scroll gesture detected
} azoteq_iqs5xx_gesture_events_1_t;
```

**Gesture Enable/Disable Configuration:**
- Register: `AZOTEQ_IQS5XX_REG_SINGLE_FINGER_GESTURES` (0x06B7)
- Can enable/disable individual gestures via CTRL registers
- Gesture configuration structure:
  ```c
  typedef struct {
      azoteq_iqs5xx_single_finger_gesture_enable_t single_finger_gestures;
      azoteq_iqs5xx_multi_finger_gesture_enable_t multi_finger_gestures;
      uint16_t tap_time;
      uint16_t tap_distance;
      uint16_t swipe_initial_time;
      uint16_t swipe_initial_distance;
      uint16_t swipe_consecutive_time;
      uint16_t swipe_consecutive_distance;
      int8_t swipe_angle;
  } azoteq_iqs5xx_gesture_config_t;
  ```

### MaxTouch MXT336U (Touch Events Only)

**Register Map:**
- T100: Multi-touch touchscreen object
- T5: Message processor object
- T6: Command processor object
- T44: Message count object

**Touch Events (NOT Gesture Events):**
```c
// T100 touch event types (maxtouch.h:381)
enum {
    MXT_NO_EVENT,    // No touch detected
    MXT_MOVE,        // Touch point moved
    MXT_UNSUP,       // Touch suppressed (palm rejection)
    MXT_SUP,         // Touch currently suppressed
    MXT_DOWN,        // Finger touched surface
    MXT_UP,          // Finger lifted
    MXT_UNSUPSUP,    // Suppression state change
    MXT_UNSUPUP,     // Suppression lifted
    MXT_DOWNSUP,     // Touch then suppressed
    MXT_DOWNUP       // Quick tap
};
```

**NO Dedicated Gesture Event Registers:**
- MaxTouch has no GESTURE_EVENTS_0 or GESTURE_EVENTS_1 equivalents
- No swipe, tap, zoom, scroll detection in hardware
- Gesture detection must be implemented in firmware

## Event Flow Analysis

### Azoteq IQS5xx Flow
```
Hardware Gesture Detection
    → GESTURE_EVENTS_0/1 registers
    → azoteq_iqs5xx_read_report() reads gesture flags
    → Check gesture_events_0/1 fields
    → Generate keycodes or mouse events based on gesture
```

### MaxTouch Flow
```
Hardware Touch Detection
    → T100 touch messages (DOWN, MOVE, UP, SUP)
    → maxtouch_get_report() reads touch data
    → digitizer.c: digitizer_task() processes touch contacts
    → digitizer_mouse_fallback.c: gesture state machine
    → Firmware implements gesture detection:
        - Tap detection (quick DOWN → UP within MXT_TAP_TIME)
        - Swipe detection (3-finger movement > threshold)
        - Scroll detection (2-finger movement)
        - Drag detection (sustained DOWN + MOVE)
```

## Current Gesture Implementation in QMK

### digitizer_mouse_fallback.c State Machine

**States:**
```c
typedef enum {
    None,           // No gesture in progress
    Down,           // Finger(s) on surface
    MoveScroll,     // Moving or scrolling
    Tapped,         // Tap detected
    DoubleTapped,   // Double-tap detected
    Drag,           // Dragging
    Swipe,          // Three-finger swipe
    Finished        // Gesture complete, waiting for lift
} State;
```

**Gesture Detection Logic:**

1. **Tap Detection:**
   - Trigger: DOWN → UP within `DIGITIZER_MOUSE_TAP_DETECTION_TIMEOUT` (200ms)
   - Distance limit: `DIGITIZER_MOUSE_TAP_DISTANCE` (25 pixels)
   - Result: Mouse button click

2. **Swipe Detection:**
   - Trigger: 3 fingers down + movement
   - Distance threshold: `DIGITIZER_MOUSE_SWIPE_DISTANCE` (500 pixels)
   - Angle limit: `DIGITIZER_MOUSE_SWIPE_THRESHOLD` (300 pixels)
   - Timeout: `DIGITIZER_MOUSE_SWIPE_TIMEOUT` (1000ms)
   - Result: Keycode (configurable via DIGITIZER_SWIPE_*_KC)

3. **Scroll Detection:**
   - Trigger: 2 fingers moving
   - Divisor: `DIGITIZER_SCROLL_DIVISOR` (10)
   - Result: Mouse scroll (h/v)

4. **Drag Detection:**
   - Trigger: 1 finger down + movement beyond tap distance/time
   - Result: Mouse movement + button held

### Integration Points in dilemma.c

**Location:** `keyboards/bastardkb/dilemma/dilemma.c:223-264`

```c
static void pointing_device_task_dilemma(report_mouse_t *mouse_report) {
    // Updates scroll divisors based on VIA config
    update_scroll_divisors();

    if (g_dilemma_config.is_dragscroll_enabled) {
        // Drag-scroll: convert trackball to scroll
        // Applies VIA drag_scroll_x/y_divisor
        scroll_buffer_x += (mouse_report->x / g_current_scroll_x_divisor);
        scroll_buffer_y += (mouse_report->y / g_current_scroll_y_divisor);
        // ... accumulate and emit scroll events
    } else {
        // Two-finger scroll: apply VIA two_finger_x/y_divisor
        // The digitizer driver has already applied DIGITIZER_SCROLL_DIVISOR
        if (mouse_report->h != 0 && g_current_scroll_x_divisor > 1) {
            mouse_report->h = mouse_report->h / g_current_scroll_x_divisor;
        }
        if (mouse_report->v != 0 && g_current_scroll_y_divisor > 1) {
            mouse_report->v = mouse_report->v / g_current_scroll_y_divisor;
        }
    }
}
```

**Current State:**
- dilemma.c processes mouse reports AFTER digitizer_mouse_fallback gesture detection
- Applies scroll divisors to TWO-FINGER scroll from digitizer
- Does NOT receive or process gesture events (they don't exist in MaxTouch)
- Gesture filtering would need to intercept state machine in digitizer_mouse_fallback.c

## VIA Configuration Integration Points

### Current VIA Config (Phase 2)
```c
typedef union {
    uint8_t raw[32];
    struct {
        // DPI settings (8 bytes)
        uint8_t dpi_preset : 3;          // 0-6 (preset 7 reserved)
        uint16_t custom_dpi;             // 200-4000 range

        // Scroll divisors (4 bytes)
        uint8_t drag_scroll_x_divisor;   // 1-255
        uint8_t drag_scroll_y_divisor;   // 1-255
        uint8_t two_finger_x_divisor;    // 1-255
        uint8_t two_finger_y_divisor;    // 1-255

        // RESERVED: Gesture enable flags (future)
        // bool enable_tap : 1;
        // bool enable_swipe : 1;
        // bool enable_zoom : 1;
        // ... etc
    } __attribute__((packed));
} via_dilemma_config_t;
```

**Available Space:** 18 bytes reserved for future expansion (current usage: 14/32 bytes)

## Implementation Strategy for Gesture Enable/Disable

### Challenge
MaxTouch does NOT report hardware gestures, so we cannot "disable" them at the hardware level like Azoteq IQS5xx.

### Solution: Firmware-Based Gesture Filtering

**Approach 1: State Machine Interception (Recommended)**
1. Add gesture enable flags to VIA config (use reserved bytes)
2. In `digitizer_update_mouse_report()`, check flags before entering gesture states
3. If gesture disabled, bypass detection logic

```c
void digitizer_update_mouse_report(report_digitizer_t *report) {
    // ... existing code ...

    switch (state) {
        case Down:
            // Check if tap/swipe enabled
            if (!gesture_config.tap_enabled) {
                // Skip tap detection, go straight to MoveScroll
                state = MoveScroll;
            }
            break;

        case Swipe:
            // Check if swipe enabled
            if (!gesture_config.swipe_enabled) {
                state = MoveScroll;  // Fall back to scroll/move
            }
            break;
        // ... etc
    }
}
```

**Approach 2: Pre-Filter Touch Data**
1. In `maxtouch_get_report()`, filter touch events based on VIA config
2. Suppress specific touch patterns (e.g., ignore 3-finger touches if swipe disabled)
3. Downside: Complex logic, may break other features

**Recommended: Approach 1** (state machine interception)

### Required VIA Config Fields

```c
// Gesture enable flags (1 byte total)
struct {
    uint8_t enable_tap : 1;           // Single/double tap gestures
    uint8_t enable_swipe : 1;         // Three-finger swipe gestures
    uint8_t enable_zoom : 1;          // Zoom/pinch gestures (if implemented)
    uint8_t reserved : 5;
} gesture_flags;

// Gesture sensitivity (optional, 4 bytes)
uint8_t tap_sensitivity;             // Adjust DIGITIZER_MOUSE_TAP_DISTANCE
uint8_t swipe_sensitivity;           // Adjust DIGITIZER_MOUSE_SWIPE_DISTANCE
uint8_t scroll_sensitivity;          // Adjust DIGITIZER_SCROLL_DIVISOR
uint8_t drag_sensitivity;            // Adjust drag detection
```

**Memory Impact:** 1-5 bytes (within 18-byte reserve)

## Register Read/Write Patterns

### Reading Gesture State (Azoteq IQS5xx)
```c
// Read GESTURE_EVENTS_0 register
i2c_read_register16(AZOTEQ_IQS5XX_ADDRESS, 0x06A5, &gesture_events_0, 1, timeout);

// Check flags
if (gesture_events_0.single_tap) {
    // Handle tap
}
```

### Enabling/Disabling Gestures (Azoteq IQS5xx)
```c
// Read current config
i2c_read_register16(AZOTEQ_IQS5XX_ADDRESS, 0x06B7, &config, sizeof(config), timeout);

// Modify flags
config.single_finger_gestures.single_tap = ENABLE;
config.single_finger_gestures.swipe_x_plus = DISABLE;

// Write back
i2c_write_register16(AZOTEQ_IQS5XX_ADDRESS, 0x06B7, &config, sizeof(config), timeout);
```

### MaxTouch Equivalent (Firmware-Based)
```c
// No hardware registers to read/write
// Gesture config is in VIA EEPROM
via_dilemma_config_t config = g_via_dilemma_config;

// Apply to gesture state machine
if (config.gesture_flags.enable_tap) {
    // Allow tap detection in state machine
} else {
    // Skip tap detection logic
}
```

## Risk Assessment

### Technical Risks

1. **No Hardware Gesture Support**
   - Risk: MaxTouch cannot report gestures like IQS5xx
   - Mitigation: Implement firmware-based gesture detection
   - Status: ✅ Already implemented in `digitizer_mouse_fallback.c`

2. **State Machine Complexity**
   - Risk: Adding enable/disable flags may introduce state machine bugs
   - Mitigation: Thorough testing, add debug logging
   - Impact: Medium

3. **Performance Overhead**
   - Risk: Checking VIA config on every gesture may slow processing
   - Mitigation: Cache config in static variable, only reload on changes
   - Impact: Low (minimal overhead)

### Integration Risks

1. **Backward Compatibility**
   - Risk: Changing gesture behavior breaks existing user configs
   - Mitigation: Default all gestures ENABLED, opt-out via VIA
   - Impact: Low

2. **VIA Config Space**
   - Risk: Running out of EEPROM space for gesture flags
   - Mitigation: 18 bytes reserved, only need 1-5 bytes
   - Impact: None

3. **User Expectations**
   - Risk: Users expect hardware gesture support like IQS5xx
   - Mitigation: Clear documentation that gestures are firmware-based
   - Impact: Low (documentation)

## Recommended Implementation Plan

### Phase 03-02: Add VIA Gesture Flags
1. Extend `via_dilemma_config_t` with gesture enable flags
2. Add VIA value IDs for gesture settings
3. Implement get/set handlers in `via_custom_value_command_kb()`

### Phase 03-03: Implement Gesture Filtering
1. Modify `digitizer_update_mouse_report()` state machine
2. Add gesture config checks before gesture detection
3. Test all gesture types with enable/disable

### Phase 03-04: Add Gesture Sensitivity (Optional)
1. Add sensitivity fields to VIA config
2. Map VIA values to gesture thresholds
3. Implement dynamic threshold adjustment

## Summary

**Key Discovery:** MaxTouch MXT336U does NOT have hardware gesture event registers like Azoteq IQS5xx. Gesture detection is entirely firmware-based in `digitizer_mouse_fallback.c`.

**Implication:** We cannot simply "read gesture events" from MaxTouch hardware. Instead, we must:
1. Add gesture enable flags to VIA config
2. Intercept the firmware gesture state machine
3. Filter gesture detection based on VIA settings

**Advantages:**
- ✅ More flexible than hardware gestures
- ✅ Can add new gesture types without hardware changes
- ✅ Sensitivity adjustable at runtime
- ✅ Already have state machine infrastructure

**Disadvantages:**
- ❌ More CPU overhead than hardware gestures
- ❌ Requires careful state machine testing
- ❌ May have higher latency than hardware detection

**Next Steps:** Proceed with firmware-based gesture filtering in Plans 02-04.
