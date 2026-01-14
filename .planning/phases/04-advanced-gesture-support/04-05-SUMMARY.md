---
phase: 04-advanced-gesture-support
plan: 05
subsystem: firmware
tags: [qmk, gesture-detection, state-machine, pinch-to-zoom, via-config]

# Dependency graph
requires:
  - phase: 04-advanced-gesture-support
    provides: VIA command handlers for advanced gestures (04-04)
provides:
  - Pinch-to-zoom gesture detection in firmware state machine
  - Squared distance calculation helper for zoom detection
  - VIA-configurable zoom keycodes (zoom_in_keycode, zoom_out_keycode)
  - Zoom gesture constants (timeout, threshold, default keycodes)
affects: []

# Tech tracking
tech-stack:
  added: []
  patterns:
    - Squared distance comparison to avoid sqrt (integer-only math)
    - Firmware-level gesture detection with VIA config filtering
    - State machine pattern for gesture lifecycle management

key-files:
  created: []
  modified:
    - quantum/digitizer_mouse_fallback.c (Zoom state, tracking, helper function, constants)
    - keyboards/bastardkb/dilemma/dilemma.c (gesture state placeholder update)

key-decisions:
  - "Use squared distance instead of actual distance to avoid expensive sqrt() operation"
  - "VIA keycode overrides with null checks (0 = no keycode configured)"
  - "Zoom filtering at firmware level via pinch_to_zoom_enabled check"

patterns-established:
  - "Pattern: Integer-only math for performance (squared distance thresholds)"
  - "Pattern: Firmware state machine handles gesture detection and VIA config checks"
  - "Pattern: Static tracking variables for gesture lifecycle (zoom_initial_distance, current_distance, finger positions)"

issues-created: []

# Metrics
duration: 15min
completed: 2026-01-14
---

# Phase 04-05: Pinch-to-Zoom Gesture Detection Summary

**Pinch-to-zoom gesture detection with squared distance calculation, VIA-configurable keycodes, and firmware-level filtering**

## Performance

- **Duration:** 15 min
- **Started:** 2026-01-14T10:00:00Z
- **Completed:** 2026-01-14T10:15:00Z
- **Tasks:** 8
- **Files modified:** 2

## Accomplishments

- Complete pinch-to-zoom gesture detection functional in firmware state machine
- Integer-only distance calculation using squared comparison (no floating-point math)
- VIA-configurable zoom keycodes with null checks (zoom_in_keycode, zoom_out_keycode)
- Two-finger tracking with position capture and threshold-based triggering
- Timeout handling prevents stuck zoom state (300ms default)
- Sensible defaults (KC_EQUAL for zoom in, KC_MINUS for zoom out)

## Task Commits

Each task was committed atomically:

1. **Task 1: Add Zoom state to state machine enum** - `cfee145168` (feat)
2. **Task 2: Add zoom tracking variables** - `7dff8d6b77` (feat)
3. **Task 3: Add zoom distance calculation helper function** - `7a135c26f1` (feat)
4. **Task 4: Add zoom detection constants** - `f708fbba39` (feat)
5. **Task 5: Implement Zoom state in state machine** - `f887d96d3a` (feat)
6. **Task 6: Add Zoom state entry from Down state** - `054b2221af` (feat)
7. **Task 7: Add VIA keycode overrides for zoom** - `06cee1cdcc` (feat)
8. **Task 8: Update gesture state tracking for zoom** - `e263fa0e32` (docs)

**Plan metadata:** N/A (plan execution summary follows)

## Files Created/Modified

- `quantum/digitizer_mouse_fallback.c` - Added Zoom state enum, 6 tracking variables, calculate_squared_distance() helper, 4 zoom constants, Zoom state case (50 lines), Down state zoom entry logic (20 lines), VIA keycode overrides (8 lines)
- `keyboards/bastardkb/dilemma/dilemma.c` - Updated gesture state placeholder to acknowledge Zoom state and firmware-level zoom detection

## Decisions Made

- **Squared distance for performance:** Used squared distance comparison (threshold²) instead of actual distance to avoid expensive sqrt() operation, maintaining integer-only math
- **VIA keycode overrides with null checks:** Added null checks for zoom_in_keycode and zoom_out_keycode (0 = no keycode configured), preventing accidental keypresses
- **Firmware-level zoom filtering:** Kept zoom gesture filtering at firmware level via pinch_to_zoom_enabled check in Zoom state case, following Phase 3 pattern

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None - all tasks completed successfully, code compiles without errors.

## Verification

- [x] Zoom state added to State enum (grep confirms "Zoom" in enum list)
- [x] Six zoom tracking variables added (grep confirms all 6 variables declared)
- [x] calculate_squared_distance() helper function implemented (grep -A 10 shows complete function)
- [x] Four zoom gesture constants defined (grep confirms ZOOM_TIMEOUT, ZOOM_DISTANCE_THRESHOLD, ZOOM_IN_KC, ZOOM_OUT_KC)
- [x] Zoom state case implemented in state machine (grep -A 40 shows complete case)
- [x] Down state detects 2-finger gesture and enters Zoom (grep -B 5 -A 15 shows entry logic)
- [x] Zoom state uses VIA-configured keycodes (grep -B 2 -A 8 shows zoom_in_keycode/zoom_out_keycode usage)
- [x] update_gesture_state() updated with zoom placeholder (grep -A 5 shows placeholder comment)
- [x] Code compiles without syntax errors (gcc test successful)
- [x] No warnings introduced

## Next Phase Readiness

**Plan 04-05 complete** - Pinch-to-zoom gesture detection fully implemented with VIA integration.

**Ready for:** Plan 04-06 (Testing and Documentation) - Final phase 4 plan to validate gesture behavior, update documentation, and prepare for Phase 5.

**Deliverables completed:**
- Zoom gesture state machine with two-finger tracking
- Distance calculation using squared distance (integer-only math)
- VIA-configurable keycodes with defaults (KC_EQUAL/KC_MINUS)
- Timeout handling and state transitions
- Firmware-level filtering via pinch_to_zoom_enabled check

**Files modified:** 2 files, ~120 lines added (including comments)

**Technical highlights:**
- Performance-optimized integer math (no sqrt)
- Complete VIA integration (enable flag + keycodes)
- Backward compatible (zoom disabled by default)
- Follows established Phase 3 patterns

---
*Phase: 04-advanced-gesture-support*
*Completed: 2026-01-14*
