# Plan 03-03 Summary: VIA Gesture Enable Command Handlers

**Status:** ✅ COMPLETE
**Duration:** ~15 minutes (4 tasks, 4 atomic commits)
**Date:** 2026-01-14

## Objective

Add VIA value IDs and command handlers for gesture enable toggles, allowing users to enable/disable basic gestures through VIA.

## Completed Tasks

### Task 1: Add VIA value IDs for gesture enables ✅
**Commit:** `6e86ebb001` - feat(03-03): add VIA value IDs for gesture enables

Added 4 new VIA value IDs (8-11) to the `via_dilemma_value_id` enum:
- `id_dilemma_tap_to_click_enable = 8`
- `id_dilemma_two_finger_tap_enable = 9`
- `id_dilemma_two_finger_scroll_enable = 10`
- `id_dilemma_press_and_hold_enable = 11`

These IDs map to gesture enable flags stored in Byte 6 of `via_dilemma_config_t`.

**Files Modified:**
- `keyboards/bastardkb/dilemma/dilemma.h` (+5 lines)

### Task 2: Add VIA set handlers for gesture enables ✅
**Commit:** `91e5653900` - feat(03-03): add VIA set handlers for gesture enables

Added 4 new case statements to `id_custom_set_value` section in `via_custom_value_command_kb()`:
- `id_dilemma_tap_to_click_enable`: sets `tap_to_click_enabled`
- `id_dilemma_two_finger_tap_enable`: sets `two_finger_tap_enabled`
- `id_dilemma_two_finger_scroll_enable`: sets `two_finger_scroll_enabled`
- `id_dilemma_press_and_hold_enable`: sets `press_and_hold_enabled`

No validation needed - these are boolean enable flags (0 or 1).

**Files Modified:**
- `keyboards/bastardkb/dilemma/dilemma.c` (+12 lines)

### Task 3: Add VIA get handlers for gesture enables ✅
**Commit:** `9c136d3ae3` - feat(03-03): add VIA get handlers for gesture enables

Added 4 new case statements to `id_custom_get_value` section in `via_custom_value_command_kb()`:
- `id_dilemma_tap_to_click_enable`: gets `tap_to_click_enabled`
- `id_dilemma_two_finger_tap_enable`: gets `two_finger_tap_enabled`
- `id_dilemma_two_finger_scroll_enable`: gets `two_finger_scroll_enabled`
- `id_dilemma_press_and_hold_enable`: gets `press_and_hold_enabled`

Follows existing pattern from DPI preset getter.

**Files Modified:**
- `keyboards/bastardkb/dilemma/dilemma.c` (+12 lines)

### Task 4: Initialize gesture enable defaults in eeconfig_init_kb ✅
**Commit:** `fbc7539466` - feat(03-03): initialize gesture enable defaults in eeconfig_init_kb

Added default initialization for gesture enable flags in `eeconfig_init_kb()`:
- `tap_to_click_enabled = 1` (ON - matches macOS trackpad default)
- `two_finger_tap_enabled = 0` (OFF - right-click via tap can be confusing)
- `two_finger_scroll_enabled = 1` (ON - core feature, already working)
- `press_and_hold_enabled = 0` (OFF - can interfere with drag operations)

These defaults match macOS trackpad behavior and can be changed by user via VIA.

**Files Modified:**
- `keyboards/bastardkb/dilemma/dilemma.c` (+4 lines)

## Key Achievements

✅ **Complete VIA read/write access** for 4 basic gesture enable flags
✅ **No validation needed** - boolean flags (0 or 1) are always valid
✅ **Sensible defaults** matching macOS trackpad behavior
✅ **Clean pattern adherence** following existing DPI/scroll divisor implementations
✅ **Infrastructure ready** for Plan 03-04 to implement gesture filtering logic

## Technical Details

### VIA Command Integration

The gesture enable flags are now fully integrated into the VIA custom value command system:

1. **SET operation (id_custom_set_value)**: VIA sends value_data[0] → firmware updates g_via_dilemma_config
2. **GET operation (id_custom_get_value)**: VIA requests current state → firmware returns g_via_dilemma_config value
3. **SAVE operation (id_custom_save)**: Config persisted to EEPROM and applied to hardware

### EEPROM Structure

The gesture enable flags are stored in Byte 6 of the 32-byte VIA custom config:
```c
// Byte 6: Basic gesture enables
uint8_t tap_to_click_enabled : 1;   // Single-finger tap
uint8_t two_finger_tap_enabled : 1; // Right-click equivalent
uint8_t two_finger_scroll_enabled : 1; // Scroll gesture
uint8_t press_and_hold_enabled : 1;  // Long-press selection
uint8_t reserved_6 : 4;            // Future gesture enables
```

### Default Gesture Enable States

| Gesture | Default | Rationale |
|---------|---------|-----------|
| Tap-to-click | ON (1) | Matches macOS trackpad default behavior |
| Two-finger tap | OFF (0) | Right-click via tap can be confusing for some users |
| Two-finger scroll | ON (1) | Core feature, already working in firmware |
| Press-and-hold | OFF (0) | Can interfere with drag operations, power user feature |

## Integration with Phase 2 Patterns

This plan follows the established patterns from Phase 2 (VIA Configuration Integration):

- **Value ID enumeration**: Sequential numbering after existing IDs
- **Set handlers**: Direct assignment to config struct (no validation for booleans)
- **Get handlers**: Direct read from config struct
- **Initialization**: Default values set in `eeconfig_init_kb()`
- **Pattern consistency**: Matches DPI preset and scroll divisor implementations

## Next Steps

Plan 03-04 will use these VIA config flags to implement gesture filtering:
- Add gesture config checks before gesture detection in `digitizer_update_mouse_report()`
- Complete the `update_gesture_state()` TODO placeholder
- Enable firmware to filter gestures based on VIA config flags
- Test gesture enable/disable functionality through VIA

## Files Modified Summary

**Total changes:** +33 lines across 2 files

1. **keyboards/bastardkb/dilemma/dilemma.h**
   - Added 4 VIA value IDs (8-11)
   - Total: +5 lines

2. **keyboards/bastardkb/dilemma/dilemma.c**
   - Added 4 set handlers (id_custom_set_value)
   - Added 4 get handlers (id_custom_get_value)
   - Added 4 default initializations (eeconfig_init_kb)
   - Total: +28 lines

## Success Criteria Met

✅ 4 new VIA value IDs added to enum (sequential after existing IDs)
✅ Set handlers added for all 4 gesture enables
✅ Get handlers added for all 4 gesture enables
✅ Default values initialized in eeconfig_init_kb()
✅ Code follows existing VIA command patterns
✅ No validation needed (boolean flags)
✅ Infrastructure ready for Plan 03-04 gesture filtering

## Related Plans

- **03-01**: Gesture Event Flow Discovery (DISCOVERY - MaxTouch has no hardware gesture registers)
- **03-02**: Gesture Event Tracking (established gesture state infrastructure)
- **03-04**: Implement Gesture Filtering (use these VIA config flags to filter gestures)
