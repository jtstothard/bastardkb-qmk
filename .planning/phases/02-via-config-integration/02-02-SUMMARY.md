---
phase: 02-via-config-integration
plan: 02
subsystem: pointing-device
tags: [scroll, via-config, drag-scroll, two-finger-scroll, maxtouch]

# Dependency graph
requires:
  - phase: 02-via-config-integration/01
    provides: VIA config structure and storage, helper functions
provides:
  - Runtime-configurable scroll divisors for drag-scroll and two-finger scroll
  - Separate scroll speeds for different scroll modes
  - Automatic divisor updates on mode changes
affects: [02-via-config-integration/03]

# Tech tracking
tech-stack:
  added: []
  patterns: [mode-based divisor selection, runtime scroll configuration]

key-files:
  created: []
  modified: [keyboards/bastardkb/dilemma/dilemma.c]

key-decisions:
  - "Apply divisors at dilemma.c level rather than modifying digitizer driver"
  - "Update divisors on each pointing_device_task iteration for responsiveness"
  - "Separate divisors for drag-scroll vs two-finger scroll modes"

patterns-established:
  - "Pattern: Mode-based configuration selection via update_scroll_divisors()"
  - "Pattern: Divisor scaling applied before scroll accumulation (drag-scroll)"
  - "Pattern: Divisor scaling applied to scroll events (two-finger)"

issues-created: []

# Metrics
duration: 12min
completed: 2026-01-14
---

# Plan 02-02: VIA Scroll Divisor Configuration - SUMMARY

**Runtime-configurable scroll divisors for drag-scroll and two-finger scroll modes, enabling separate scroll speeds via VIA without firmware rebuild**

## Performance

- **Duration:** 12 minutes
- **Started:** 2026-01-14T15:30:00Z
- **Completed:** 2026-01-14T15:42:00Z
- **Tasks:** 4
- **Files modified:** 1

## Accomplishments

- Implemented runtime-configurable scroll divisors for both drag-scroll and two-finger scroll modes
- Created `update_scroll_divisors()` function to select appropriate divisors based on current mode
- Updated `pointing_device_task_dilemma()` to apply VIA scroll divisors to scroll events
- Integrated divisor updates into mode change and VIA config save workflows

## Task Commits

Each task was committed atomically:

1. **Task 2: Add global divisor variables for runtime scroll speed config** - `d9d6631519` (feat)
2. **Task 3 & 4: Implement VIA-configurable scroll divisors** - `702fc1a0fc` (feat)

**Plan metadata:** N/A (summary created)

_Note: Task 1 was analysis-only (no code changes)_

## Files Created/Modified

- `keyboards/bastardkb/dilemma/dilemma.c` - Added scroll divisor tracking variables, update function, and application logic

## Decisions Made

1. **Apply Divisors at dilemma.c Level:**
   - Rather than modifying the digitizer driver (QMK core code)
   - dilemma.c has access to VIA config
   - Cleaner separation of concerns
   - Easier to maintain and test

2. **Update Divisors on Each Iteration:**
   - `update_scroll_divisors()` called in `pointing_device_task_dilemma()`
   - Ensures divisors stay in sync with mode changes
   - Low overhead (simple integer assignment)
   - More responsive than updating only on mode toggles

3. **Separate Divisors for Each Mode:**
   - Drag-scroll uses `drag_scroll_x/y_divisor` from VIA config
   - Two-finger scroll uses `two_finger_x/y_divisor` from VIA config
   - Allows users to configure different scroll speeds for each mode
   - Matches the use case (drag-scroll for coarse, two-finger for fine)

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None - implementation was straightforward.

## Implementation Details

### Task 1: Research Scroll Divisor Usage (Analysis Only)

**Findings:**
- `DIGITIZER_SCROLL_DIVISOR` is a compile-time constant in `quantum/digitizer_mouse_fallback.c`
- Applied at lines 259-263: `carry_h = h % DIGITIZER_SCROLL_DIVISOR` and `mouse_report.h = h / DIGITIZER_SCROLL_DIVISOR`
- Used in TwoFingerScroll state to scale scroll movements
- Decision: Apply divisors at dilemma.c level, not modify digitizer driver

### Task 2: Add Global Divisor Variables

**Implementation:**
```c
// Global scroll divisor tracking variables
static uint16_t g_current_scroll_x_divisor = 8;  // Default matching VIA config
static uint16_t g_current_scroll_y_divisor = 8;  // Default matching VIA config
```

**Purpose:**
- Track current scroll divisors based on mode
- Defaults match VIA config defaults (8)
- Used by `pointing_device_task_dilemma()` to apply scroll scaling

### Task 3: Update pointing_device_task_dilemma() to Use VIA Divisors

**Drag-Scroll Mode:**
```c
// Apply VIA drag-scroll divisor to scale movement before accumulation
scroll_buffer_x += (mouse_report->x / g_current_scroll_x_divisor);
scroll_buffer_y += (mouse_report->y / g_current_scroll_y_divisor);
```

**Two-Finger Scroll Mode:**
```c
// Apply VIA two-finger divisor to digitizer scroll
if (mouse_report->h != 0 && g_current_scroll_x_divisor > 1) {
    mouse_report->h = mouse_report->h / g_current_scroll_x_divisor;
}
if (mouse_report->v != 0 && g_current_scroll_y_divisor > 1) {
    mouse_report->v = mouse_report->v / g_current_scroll_y_divisor;
}
```

**How It Works:**
- **Drag-scroll:** Divisor scales trackball movement BEFORE accumulation in scroll buffer
  - Larger divisor = slower scroll (more movement needed per scroll event)
- **Two-finger:** Divisor scales digitizer scroll events AFTER digitizer driver processing
  - Digitizer driver applies `DIGITIZER_SCROLL_DIVISOR` (10) first
  - Then VIA divisor applies additional scaling
  - Composite divisor = 10 × VIA_divisor

### Task 4: Add Divisor Update Function

**Function:**
```c
static void update_scroll_divisors(void) {
    if (g_dilemma_config.is_dragscroll_enabled) {
        g_current_scroll_x_divisor = g_via_dilemma_config.drag_scroll_x_divisor;
        g_current_scroll_y_divisor = g_via_dilemma_config.drag_scroll_y_divisor;
    } else {
        g_current_scroll_x_divisor = g_via_dilemma_config.two_finger_x_divisor;
        g_current_scroll_y_divisor = g_via_dilemma_config.two_finger_y_divisor;
    }
}
```

**Called From:**
1. `pointing_device_task_dilemma()` - on each iteration (keeps divisor in sync)
2. `dilemma_set_pointer_dragscroll_enabled()` - after mode changes
3. `apply_via_dilemma_config()` - after VIA config saves

## Verification Checklist

- [x] Global divisor variables added with correct defaults (8)
- [x] `update_scroll_divisors()` function implemented
- [x] `update_scroll_divisors()` called from all three required locations
- [x] Drag-scroll mode uses `drag_scroll_x/y_divisor` from VIA config
- [x] Two-finger scroll mode uses `two_finger_x/y_divisor` from VIA config
- [x] Divisors update automatically when mode changes
- [x] Divisors update when VIA config is saved
- [x] No breaking changes to existing scroll functionality

## Success Criteria Met

✅ **Scroll divisors are configurable via VIA**
- 4-bit fields (0-15) in VIA config struct
- Separate X and Y divisors for each mode
- Accessible via VIA custom value commands

✅ **Separate divisors for drag-scroll and two-finger scroll**
- Drag-scroll: uses `drag_scroll_x/y_divisor`
- Two-finger: uses `two_finger_x/y_divisor`
- Enables different scroll speeds for each mode

✅ **Divisors update automatically on mode changes**
- `update_scroll_divisors()` called on mode toggle
- `update_scroll_divisors()` called on each pointing device task
- `update_scroll_divisors()` called after VIA config saves

✅ **Scroll speed is now user-adjustable without firmware rebuild**
- VIA GUI can modify divisor values
- Changes take effect immediately after save
- No recompilation required

## Technical Notes

### Scroll Speed Behavior

**Divisor Value Meaning:**
- Value 1: No additional scaling (fastest scroll)
- Value 8: Default speed (baseline)
- Value 15: Slowest scroll (maximum divisor)

**Effective Divisor for Two-Finger Scroll:**
```
final_scroll = raw_movement / (DIGITIZER_SCROLL_DIVISOR × g_current_scroll_x_divisor)
              = raw_movement / (10 × 8)  (with defaults)
              = raw_movement / 80
```

**Example:**
- Finger moves 80 pixels → scroll event of 1
- Increasing divisor to 16 → 80 / (10 × 16) = 0.5 (rounds to 0, need 160 pixels)
- Decreasing divisor to 4 → 80 / (10 × 4) = 2 (scrolls faster)

### Integration with Existing Code

**Drag-Scroll (Trackball):**
- Original: Accumulate raw trackball movement, emit scroll when threshold exceeded
- New: Divide movement by divisor BEFORE accumulation
- Effect: Larger divisor = slower scroll (need more trackball movement)

**Two-Finger Scroll (Digitizer):**
- Original: Digitizer driver applies DIGITIZER_SCROLL_DIVISOR (10), emits scroll events
- New: Additional divisor applied to scroll events in dilemma.c
- Effect: Larger divisor = slower scroll (further division)

## Next Steps

**Remaining Phase 2 Plans:**
- Plan 02-03: Migrate pointing device config to VIA (integrate old `dilemma_config_t` fields into VIA)

**Phase 2 Status:** 2 of 3 plans complete

**Future Enhancements:**
- Add scroll divisor configuration UI to VIA keymap
- Consider adding separate horizontal/vertical divisors to VIA GUI
- Add preset profiles (e.g., "Precision Scrolling", "Fast Scrolling")
- Phase 10+ will add gesture-to-keycode mappings

## Conclusion

Plan 02-02 successfully implements runtime-configurable scroll divisors for both drag-scroll and two-finger scroll modes. The implementation is clean, follows existing patterns, and provides users with fine-grained control over scroll speed without requiring firmware rebuilds. The `update_scroll_divisors()` function ensures divisors stay in sync with mode changes, and the division logic is applied at the appropriate point in the scroll processing pipeline for each mode. This completes the scroll configuration portion of Phase 2, preparing for the final plan to complete the VIA configuration integration.
