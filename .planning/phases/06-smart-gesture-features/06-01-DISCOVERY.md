# Smart Gesture Features Discovery

## Executive Summary

**PURPOSE:** Research MaxTouch MXT336U pressure/force sensing capabilities and design firmware-based approach for smart gesture features (force click, smart zoom).

**KEY FINDING:** MaxTouch MXT336U has limited force sensing capabilities through T65 Lens Bending object and T100 amplitude reporting, but these are NOT suitable for reliable pressure-based tap detection. Firmware-based timing approach is recommended for force click (long press).

**STATUS:** Smart zoom already implemented in Phase 4 (Plan 04-05). Only force click detection remains.

---

## Task 1: MaxTouch Pressure/Force Sensing Capabilities

### Search Results

#### 1. T65 Lens Bending Object (Force Sensing)

**Object Type:** T65 - Signal processing for lens bending detection (force/pressure)

**Register Structure** (`drivers/sensors/maxtouch.h:251-274`):
```c
typedef struct PACKED {
    unsigned char ctrl;                    // Control register
    unsigned char gradthr;                 // Gradient threshold
    unsigned char ylonoisemul_lsb;         // Y-low noise multiplier LSB
    unsigned char ylonoisemul_msb;         // Y-low noise multiplier MSB
    unsigned char ylonoisediv_lsb;         // Y-low noise divisor LSB
    unsigned char ylonoisediv_msb;         // Y-low noise divisor MSB
    unsigned char yhinoisemul_lsb;         // Y-high noise multiplier LSB
    unsigned char yhinoisemul_msb;         // Y-high noise multiplier MSB
    unsigned char yhinoisediv_lsb;         // Y-high noise divisor LSB
    unsigned char yhinoisediv_msb;         // Y-high noise divisor MSB
    unsigned char lpfiltcoef;              // Low-pass filter coefficient
    unsigned char forcescale_lsb;          // Force scale LSB (★)
    unsigned char forcescale_msb;          // Force scale MSB (★)
    unsigned char forcethr;                // Force threshold (★)
    unsigned char forcethrhyst;            // Force threshold hysteresis (★)
    unsigned char forcedi;                 // Force detection integration (★)
    unsigned char forcehyst;               // Force hysteresis (★)
    unsigned char atchratio;               // Attach ratio
    unsigned char reserved[2];
    unsigned char exfrcthr;                // Excessive force threshold
    unsigned char exfrcthrhyst;            // Excessive force hysteresis
    unsigned char exfrcto;                 // Excessive force timeout
} mxt_proci_lensbending_t65;
```

**Control Flags** (`drivers/sensors/maxtouch.h:276-282`):
```c
static const unsigned char T65_CTRL_ENABLE         = 0x01;  // Enable object
static const unsigned char T65_CTRL_RPTEN          = 0x02;  // Enable reporting
static const unsigned char T65_CTRL_DISPRESS       = 0x08;  // Disable press detection
static const unsigned char T65_CTRL_DISRELEASE     = 0x10;  // Disable release detection
static const unsigned char T65_CTRL_DISHIST        = 0x80;  // Disable hysteresis
```

**Current Implementation** (`drivers/sensors/maxtouch.c:455-460`):
```c
#ifdef MXT_T65_LENS_BENDING_ENABLE
    if (t65_proci_lensbending_address) {
        mxt_proci_lensbending_t65 t65 = {};
        t65.ctrl                      = T65_CTRL_ENABLE;
        // Force detection configured but NOT enabled by default
        i2c_write_register16(MXT336UD_ADDRESS, t65_proci_lensbending_address,
                             (uint8_t *)&t65, sizeof(mxt_proci_lensbending_t65),
                             MXT_I2C_TIMEOUT_MS);
    }
#endif
```

**Status:** ⚠️ **NOT ENABLED BY DEFAULT** - Requires `MXT_T65_LENS_BENDING_ENABLE` compile flag

#### 2. T100 Amplitude Reporting (Signal Strength)

**Configuration** (`drivers/sensors/maxtouch.c:399`):
```c
cfg.tchaux = 0x2;  // Report amplitude (touch signal strength)
```

**T100 Touch Aux Flags** (from MaxTouch MXT336U datasheet):
- Bit 0 (0x1): Report pressure (NOT amplitude)
- Bit 1 (0x2): Report amplitude (touch signal strength) ✅ **CURRENTLY ENABLED**
- Bit 2 (0x4): Report vector data
- Bit 3 (0x8): Report peak detection

**Status:** ✅ **ENABLED** - Amplitude data available in T100 touch messages

#### 3. No Dedicated Pressure Registers

**Search Results:**
- ❌ No `pressure` registers found
- ❌ No `force` magnitude registers (only thresholds/scales for T65)
- ✅ `amplitude` reporting enabled (signal strength, not pressure)
- ⚠️ T65 force sensing requires compile-time enable flag

### Comparison: True Pressure vs. Signal Strength

| Aspect | True Pressure (iPhone 3D Touch) | Signal Strength (MaxTouch) |
|--------|----------------------------------|----------------------------|
| **What it measures** | Capacitive force sensing layer | Mutual capacitance signal amplitude |
| **Hardware required** | Dedicated force sensors | Standard touchscreen |
| **Range** | 0-100% pressure levels | 0-255 arbitrary amplitude units |
| **Accuracy** | High (designed for force) | Low (correlates poorly with force) |
| **Purpose** | Force-aware gestures | Touch detection quality |
| **Reliability** | Consistent across touches | Varies with finger size, moisture, etc. |

### Conclusion: Pressure Sensing NOT Suitable

**T65 Lens Bending Limitations:**
1. **Not enabled by default** - Requires compile-time flag `MXT_T65_LENS_BENDING_ENABLE`
2. **Designed for screen protection** - Detects excessive force that could damage display
3. **Not calibrated for gesture detection** - Thresholds are for damage prevention, not user input
4. **Limited documentation** - No clear mapping from force values to user intent
5. **Hardware variation** - Force sensitivity varies by device, requires per-unit calibration

**T100 Amplitude Limitations:**
1. **Signal strength ≠ pressure** - Measures touch quality, not force applied
2. **Highly variable** - Changes with finger size, moisture, skin condition
3. **Non-linear** - Poor correlation between amplitude and actual pressure
4. **Environment-dependent** - Affected by temperature, humidity, grounding

**Recommendation:** ❌ **DO NOT USE pressure/force sensing** for tap detection. Use timing-based approach instead.

---

## Task 2: Force Click (Long Press) Detection Strategy

### Firmware-Based State Machine Extension

Since MaxTouch lacks reliable pressure sensing, implement force click using **timing-based detection**:

#### Detection Algorithm

**State Machine Extension:**
```c
State current_state;
uint32_t finger_down_time;  // Timer for force click detection

// In Down state:
if (current_state == Down) {
    uint32_t duration = timer_read32() - finger_down_time;

    if (duration > FORCE_CLICK_THRESHOLD) {
        // Force click detected
        register_code(press_and_hold_keycode);
        force_click_detected = true;
    }
}

// On finger lift:
if (current_state == Up) {
    if (duration < TAP_TIMEOUT) {
        // Normal tap
        tap_detected = true;
    } else if (force_click_detected) {
        // Force click complete
        unregister_code(press_and_hold_keycode);
        force_click_detected = false;
    }
}
```

#### Threshold Definition

**Timing Thresholds:**
```c
#define FORCE_CLICK_THRESHOLD_MS 500  // 500ms = macOS long press
#define TAP_TIMEOUT_MS 200            // 200ms = existing tap timeout
#define HYSTERESIS_MS 300             // Gap between tap (200ms) and force click (500ms)
```

**Rationale:**
- **500ms** matches macOS long press duration (consistent with OS expectations)
- **200ms** matches existing `DIGITIZER_MOUSE_TAP_DETECTION_TIMEOUT`
- **300ms hysteresis** prevents ambiguous gestures (clear separation)

#### Implementation Location

**File:** `quantum/digitizer_mouse_fallback.c`

**Changes Required:**
1. Add `force_click_detected` flag to gesture state
2. Track finger down duration using `timer_read32()`
3. Check duration in `Down` state before tap detection
4. Use existing `press_and_hold_keycode` from VIA config (Bytes 16-17, Phase 3)

**VIA Config Integration (Already Implemented from Phase 3):**
```c
// From dilemma.h EEPROM struct
uint16_t press_and_hold_keycode;  // Bytes 16-17
bool press_and_hold_enabled;      // Bit flag
```

**No EEPROM Changes Required:**
- `press_and_hold_keycode` already exists (Phase 3)
- `press_and_hold_enabled` flag already exists (Phase 3)
- Just need to wire up timing-based detection

#### Keycode Mapping

**Force Click → Press and Hold Keycode:**
- Forward to existing `press_and_hold_keycode` (Bytes 16-17)
- Default: `KC_NO` (disabled, user configures via VIA)
- User can set to any keycode (e.g., `KC_BTN3` for middle click, custom macro, etc.)

#### Advantages

✅ **No special hardware required** - Works with any MaxTouch sensor
✅ **Uses existing infrastructure** - Builds on Phase 3 timing framework
✅ **Backward compatible** - Doesn't break existing tap detection
✅ **VIA-configurable** - Users can disable or change keycode without rebuild
✅ **Consistent with OS expectations** - 500ms matches macOS long press
✅ **Clear separation** - 300ms hysteresis prevents ambiguous gestures

#### Disadvantages

❌ **Not true pressure detection** - Time-based proxy, not force-based
❌ **May interfere with drag operations** - Need careful state management (don't trigger force click during drag-scroll or pointer sniping)
❌ **Slower than tap** - 500ms wait required (vs. 200ms for tap)

#### Edge Cases and State Management

**Conflict Avoidance:**
1. **Drag mode:** If `is_dragscroll_enabled` or `pointer_sniping_enabled`, disable force click detection
2. **Multi-finger gestures:** Only detect force click with 1 finger (ignore 2+ fingers)
3. **Movement:** If finger moves > `DIGITIZER_MOUSE_TAP_DISTANCE`, cancel force click detection

**State Machine Flow:**
```
None → Down (1 finger, timer starts)
Down → Tapped (lift before 200ms)
Down → ForceClick (500ms elapsed, no movement)
Down → MoveScroll (movement > tap distance)
ForceClick → Finished (lift detected)
```

---

## Task 3: Smart Zoom Gesture Status

### Already Implemented in Phase 4 (Plan 04-05)

**Status:** ✅ **COMPLETE** - No additional work needed

#### Implementation Summary

**Pinch-to-Zoom Gesture Detection** (`quantum/digitizer_mouse_fallback.c`):
- Two-finger spread/pinch detection functional
- Integer-only distance calculation (squared comparison, no sqrt)
- Zoom in/out keycodes configurable via VIA
- `zoom_in_keycode` (Bytes 34-35)
- `zoom_out_keycode` (Bytes 36-37)
- `pinch_to_zoom_enabled` flag (Byte 7, bit 0)
- Firmware state machine includes `Zoom` state
- Timeout handling prevents stuck zoom state (300ms)

**State Machine** (`digitizer_mouse_fallback.c:State` enum):
```c
typedef enum {
    None,
    Down,
    MoveScroll,
    Tapped,
    DoubleTapped,
    Drag,
    Swipe,
    Zoom,        // ✅ Added in Phase 4
    Finished
} State;
```

**Zoom Detection Logic:**
```c
// Track 2-finger distance
uint32_t zoom_distance_squared = calculate_squared_distance(
    finger1_x, finger1_y, finger2_x, finger2_y
);

// Detect zoom in (spread)
if (zoom_distance_squared > initial_distance + threshold) {
    register_code(zoom_in_keycode);
}

// Detect zoom out (pinch)
if (zoom_distance_squared < initial_distance - threshold) {
    register_code(zoom_out_keycode);
}
```

**VIA Integration** (`keyboards/bastardkb/dilemma/dilemma.h`):
- EEPROM bytes 34-37: Zoom keycodes (2 × uint16_t)
- VIA value IDs 23-24: `zoom_in_keycode`, `zoom_out_keycode`
- Default keycodes: `KC_EQUAL` (zoom in), `KC_MINUS` (zoom out)

#### Phase 6 Plan Summary

**Active Plans:**
1. **06-01:** Research (this plan) ✅
2. **06-02:** Implement force click (long press) detection ⏳
3. **06-03:** Mark smart zoom as complete (documentation only) ⏳

**Inactive/Canceled:**
4. **06-04:** ~~Smart zoom gesture~~ → Already completed in Phase 4

**Remaining Work:**
- Implement timing-based force click detection (Plan 06-02)
- Update documentation to reflect smart zoom completion (Plan 06-03)

---

## Implementation Roadmap

### Plan 06-02: Force Click Detection (Next)

**File:** `quantum/digitizer_mouse_fallback.c`

**Tasks:**
1. Add force click timing variables to state tracking
2. Extend `Down` state to check duration for force click
3. Add `ForceClick` state or handle in-line during `Down`
4. Integrate with existing `press_and_hold_keycode` from VIA
5. Add conditional logic to disable during drag/pointer modes
6. Test and refine thresholds (500ms default)

**Estimated Duration:** 15 minutes (4-5 atomic commits)

### Plan 06-03: Documentation Update

**File:** `.planning/ROADMAP.md`

**Tasks:**
1. Mark Plan 06-04 as complete (reference Phase 4, Plan 04-05)
2. Update Phase 6 timeline to reflect actual completion status
3. Document smart zoom as already implemented

**Estimated Duration:** 5 minutes (1 atomic commit)

---

## Risk Assessment

### Technical Risks

1. **Force Click State Machine Complexity**
   - Risk: Timing-based detection may introduce state machine bugs
   - Mitigation: Extend existing `Down` state, don't add new state
   - Impact: Medium

2. **Conflict with Drag Operations**
   - Risk: Force click triggers during drag-scroll or pointer sniping
   - Mitigation: Disable force click when `is_dragscroll_enabled` or `pointer_sniping_enabled`
   - Impact: Low

3. **User Expectations**
   - Risk: Users expect true pressure-based force click like iPhone 3D Touch
   - Mitigation: Clear documentation that force click is time-based (long press)
   - Impact: Low (documentation)

### Integration Risks

1. **Backward Compatibility**
   - Risk: Changing tap detection breaks existing user configs
   - Mitigation: Add force click as separate detection path, don't modify tap logic
   - Impact: Low

2. **VIA Config Space**
   - Risk: No EEPROM space for force click settings
   - Mitigation: Use existing `press_and_hold_keycode` from Phase 3
   - Impact: None (no changes needed)

3. **Performance**
   - Risk: Checking duration on every iteration slows processing
   - Mitigation: Timer reads are fast (~10 CPU cycles), negligible overhead
   - Impact: None

---

## Summary

### Key Discoveries

1. ❌ **MaxTouch pressure sensing NOT suitable** for tap detection
   - T65 Lens Bending: Not enabled by default, designed for damage prevention
   - T100 Amplitude: Signal strength, not pressure, highly variable

2. ✅ **Firmware-based timing approach recommended** for force click
   - 500ms threshold matches macOS long press
   - Uses existing `press_and_hold_keycode` from Phase 3
   - No EEPROM changes required

3. ✅ **Smart zoom already implemented** in Phase 4 (Plan 04-05)
   - Pinch-to-zoom gesture detection complete
   - VIA-configurable zoom keycodes
   - Integer-only distance calculation
   - No additional work needed

### Next Steps

**Plan 06-02:** Implement force click (long press) detection
- Extend `digitizer_mouse_fallback.c` state machine
- Add timing-based detection (500ms threshold)
- Integrate with existing VIA `press_and_hold_keycode`
- Disable during drag/pointer modes

**Plan 06-03:** Update documentation
- Mark Plan 06-04 as complete (smart zoom done in Phase 4)
- Update ROADMAP.md timeline

### Files Modified

- `.planning/phases/06-smart-gesture-features/06-01-DISCOVERY.md` (created)

### References

- `drivers/sensors/maxtouch.h` - T65 Lens Bending object, T100 amplitude
- `drivers/sensors/maxtouch.c` - Current force sensing configuration
- `quantum/digitizer_mouse_fallback.c` - Gesture state machine
- `keyboards/bastardkb/dilemma/dilemma.h` - VIA config (Phase 3 press_and_hold fields)
- `.planning/phases/03-basic-gesture-enablement/03-01-DISCOVERY.md` - Gesture event flow discovery
