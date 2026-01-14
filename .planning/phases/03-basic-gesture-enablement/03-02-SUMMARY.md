# Plan 03-02: Gesture Event Tracking - Summary

**Date:** 2026-01-14
**Status:** ✅ COMPLETE
**Duration:** ~15 minutes
**Commits:** 4 atomic commits

## Objective

Add gesture event tracking to dilemma.c to read gesture flags from hardware and make them available for filtering via VIA config.

## Background from Plan 03-01

**Critical Discovery:** MaxTouch MXT336U does NOT have hardware gesture event registers like Azoteq IQS5xx. Gesture detection is entirely firmware-based in `digitizer_mouse_fallback.c` state machine.

This plan establishes the infrastructure for gesture state tracking, preparing for Plan 03-03 which will integrate VIA config filtering into the firmware gesture state machine.

## Implementation Summary

### Task 1: Gesture State Structure ✅

**Commit:** `85237e5b0e` - feat(03-02): add gesture state tracking structure to dilemma.c

Added static structure to track gesture events:

```c
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
```

**Design Decisions:**
- Structure tracks all gesture types from Azoteq IQS5xx spec for future compatibility
- Initialized to zero/false (no active gestures on startup)
- Follows existing pattern: static globals `g_dilemma_config`, `g_via_dilemma_config`
- Placed after scroll divisor variables (line ~76)

### Task 2: Gesture State Update Function ✅

**Commit:** `017dd47c58` - feat(03-02): create gesture state update function with TODO placeholder

Created `update_gesture_state()` function with comprehensive TODO:

```c
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
    // - State machine tracks: None, Down, MoveScroll, Tapped, DoubleTapped, Drag, Swipe, Finished
    // - Plan 03-03 will integrate VIA config filtering into that state machine
    //
    // Future implementation (Plan 03-03):
    // - Access digitizer state machine to get current gesture state
    // - Update g_gesture_state fields based on active gesture
    // - This will be used by VIA config to enable/disable specific gestures
}
```

**Design Decisions:**
- Function is static (internal to dilemma.c)
- Includes comprehensive TODO referencing 03-01 DISCOVERY findings
- Documents integration point for Plan 03-03
- Follows existing pattern: `update_scroll_divisors()`
- Placed after `update_scroll_divisors()` function (line ~225)

### Task 3: Gesture State Integration ✅

**Commit:** `624ef556d0` - feat(03-02): integrate gesture state updates into pointing_device_task

Added call to `update_gesture_state()` in main pointing device loop:

```c
static void pointing_device_task_dilemma(report_mouse_t *mouse_report) {
    static int16_t scroll_buffer_x = 0;
    static int16_t scroll_buffer_y = 0;

    // Update scroll divisors based on current mode
    update_scroll_divisors();

    // Update gesture state from digitizer
    update_gesture_state();

    if (g_dilemma_config.is_dragscroll_enabled) {
        // ... drag-scroll logic
    } else {
        // ... two-finger scroll logic
    }
}
```

**Design Decisions:**
- Called immediately after `update_scroll_divisors()` for consistency
- Ensures gesture state refreshed on each pointing device iteration
- Follows Phase 2 pattern established for scroll divisor updates
- Minimal overhead (function currently empty placeholder)

### Task 4: Public API for Gesture State ✅

**Commit:** `26945b4d00` - feat(03-02): add gesture state accessor functions to public API

Added getter functions to dilemma.h:

```c
// Gesture state accessors
bool dilemma_get_single_tap(void);
bool dilemma_get_two_finger_tap(void);
bool dilemma_get_swipe_right(void);
bool dilemma_get_swipe_left(void);
bool dilemma_get_swipe_up(void);
bool dilemma_get_swipe_down(void);
bool dilemma_get_press_and_hold(void);
```

Implemented in dilemma.c:

```c
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
```

**Design Decisions:**
- Follows existing pattern: `dilemma_get_pointer_sniping_enabled()`
- All functions return bool for gesture active/inactive state
- Swipe directions mapped to x/y axis naming (x_plus=right, x_minus=left, etc.)
- Public API enables other firmware modules to query gesture state
- Essential for Plan 03-03 VIA config filtering integration

## Files Modified

1. **keyboards/bastardkb/dilemma/dilemma.c** (+81 lines)
   - Added `g_gesture_state` structure
   - Added `update_gesture_state()` function (with TODO)
   - Integrated gesture state updates into `pointing_device_task_dilemma()`
   - Added 7 getter function implementations

2. **keyboards/bastardkb/dilemma/dilemma.h** (+8 lines)
   - Added 7 getter function declarations

## Verification Checklist

- [x] Gesture state structure added to dilemma.c
- [x] `update_gesture_state()` function created (with TODO placeholder)
- [x] `update_gesture_state()` called from `pointing_device_task_dilemma()`
- [x] Getter functions declared in dilemma.h
- [x] Getter functions implemented in dilemma.c
- [x] Code follows established patterns (static globals, accessor functions)
- [x] All tasks committed individually

## Code Quality

### Patterns Followed
- Static global variables for state tracking
- Static helper functions for internal operations
- Public getter functions for external access
- Doxygen comments for documentation
- Consistent naming convention (dilemma_get_*)

### Integration Points
- `pointing_device_task_dilemma()` - Main gesture update loop
- TODO references to Plan 03-03 implementation
- Public API ready for VIA config filtering

### Documentation
- Comprehensive TODO in `update_gesture_state()` with 03-01 DISCOVERY references
- Clear function documentation
- Comments explaining integration strategy

## Key Achievements

1. **Gesture Infrastructure Established**
   - State structure tracks all gesture types
   - Update function integrates into main loop
   - Public API enables external querying

2. **Foundation for Plan 03-03**
   - Clear integration points identified
   - TODO documents MaxTouch firmware-based detection
   - Accessors ready for VIA config filtering

3. **Code Quality Maintained**
   - Follows Phase 2 patterns (scroll divisors)
   - Atomic commits for each task
   - Comprehensive documentation

## Technical Notes

### MaxTouch vs Azoteq IQS5xx

**Azoteq IQS5xx (Hardware Gesture Events):**
- Has GESTURE_EVENTS_0 and GESTURE_EVENTS_1 registers
- Firmware reads hardware-detected gestures
- Enable/disable via hardware CTRL registers

**MaxTouch MXT336U (Firmware Gesture Detection):**
- NO hardware gesture event registers
- Gesture detection in `digitizer_mouse_fallback.c` state machine
- States: None, Down, MoveScroll, Tapped, DoubleTapped, Drag, Swipe, Finished
- Enable/disable via VIA config at firmware level (Plan 03-03)

### Gesture State Machine Integration

Plan 03-03 will:
1. Intercept `digitizer_update_mouse_report()` state machine
2. Add VIA config checks before gesture state transitions
3. Use `g_gesture_state` accessor functions for filtering
4. Implement actual `update_gesture_state()` logic

## Next Steps

**Plan 03-03: Implement Gesture Filtering**
- Extend `via_dilemma_config_t` with gesture enable flags
- Add VIA value IDs for gesture settings
- Modify `digitizer_update_mouse_report()` state machine
- Implement gesture config checks before gesture detection
- Complete `update_gesture_state()` TODO with actual integration

**Deliverables for 03-03:**
- VIA config extended with gesture enable/disable flags (bytes 6-7 already reserved)
- State machine filtering implemented
- All gesture types testable with enable/disable
- Full gesture control through VIA interface

## Risk Assessment

### Current Risks: None

All code is additive with no behavior changes:
- Structure added but not yet used
- Function called but does nothing (placeholder)
- Accessors added but not yet called
- No existing functionality affected

### Future Risks (Plan 03-03)

**State Machine Complexity**
- Risk: Adding enable/disable flags may introduce state machine bugs
- Mitigation: Thorough testing, add debug logging
- Impact: Medium

**Backward Compatibility**
- Risk: Changing gesture behavior breaks existing user configs
- Mitigation: Default all gestures ENABLED, opt-out via VIA
- Impact: Low

## Conclusion

Plan 03-02 successfully established gesture state tracking infrastructure in dilemma.c. The implementation follows established patterns from Phase 2 and provides clear integration points for Plan 03-03's VIA config filtering.

The gesture state structure, update function, and public API are now in place and ready for the next phase of implementation. All code is well-documented with comprehensive TODOs referencing the 03-01 DISCOVERY findings.

**Status:** ✅ READY FOR PLAN 03-03
