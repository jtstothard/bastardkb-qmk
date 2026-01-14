# Plan 04-03 Summary: VIA Value IDs for Advanced Gesture Support

**Status:** ✅ COMPLETE
**Duration:** 15 minutes (4 tasks, 4 atomic commits)
**Date:** 2026-01-14

## Objective

Add VIA value IDs for advanced gesture enables and keycode mappings, extending the VIA custom command system to support 3-finger swipes, 4-finger swipes, and pinch-to-zoom.

## Completed Tasks

### Task 1: Advanced Gesture Enable Value IDs (12-14)
✅ **Commit:** `9e23e0e49f` - feat(04-03): add advanced gesture enable value IDs (12-14)

Added 3 VIA value IDs for advanced gesture enables:
- `id_dilemma_three_finger_swipe_enable = 12`
- `id_dilemma_four_finger_swipe_enable = 13`
- `id_dilemma_pinch_to_zoom_enable = 14`

These map to bit-fields in Byte 7 of `via_dilemma_config_t`:
- `three_finger_swipe_enabled`
- `four_finger_swipe_enabled`
- `pinch_to_zoom_enabled`

### Task 2: 3-Finger Swipe Keycode Value IDs (15-18)
✅ **Commit:** `e22276b7ab` - feat(04-03): add 3-finger swipe keycode value IDs (15-18)

Added 4 VIA value IDs for 3-finger swipe keycode mappings:
- `id_dilemma_three_finger_swipe_left_keycode = 15`
- `id_dilemma_three_finger_swipe_right_keycode = 16`
- `id_dilemma_three_finger_swipe_up_keycode = 17`
- `id_dilemma_three_finger_swipe_down_keycode = 18`

These will map to new 16-bit keycode fields in `via_dilemma_config_t`:
- `three_finger_swipe_left_keycode` (Bytes 18-19)
- `three_finger_swipe_right_keycode` (Bytes 20-21)
- `three_finger_swipe_up_keycode` (Bytes 22-23)
- `three_finger_swipe_down_keycode` (Bytes 24-25)

### Task 3: 4-Finger Swipe Keycode Value IDs (19-22)
✅ **Commit:** `ceb8bd1064` - feat(04-03): add 4-finger swipe keycode value IDs (19-22)

Added 4 VIA value IDs for 4-finger swipe keycode mappings:
- `id_dilemma_four_finger_swipe_left_keycode = 19`
- `id_dilemma_four_finger_swipe_right_keycode = 20`
- `id_dilemma_four_finger_swipe_up_keycode = 21`
- `id_dilemma_four_finger_swipe_down_keycode = 22`

These will map to new 16-bit keycode fields in `via_dilemma_config_t`:
- `four_finger_swipe_left_keycode` (Bytes 26-27)
- `four_finger_swipe_right_keycode` (Bytes 28-29)
- `four_finger_swipe_up_keycode` (Bytes 30-31)
- `four_finger_swipe_down_keycode` (Bytes 32-33)

### Task 4: Pinch-to-Zoom Keycode Value IDs (23-24)
✅ **Commit:** `ec514549d7` - feat(04-03): add pinch-to-zoom keycode value IDs (23-24)

Added 2 VIA value IDs for pinch-to-zoom keycode mappings:
- `id_dilemma_zoom_in_keycode = 23`
- `id_dilemma_zoom_out_keycode = 24`

These will map to new 16-bit keycode fields:
- `zoom_in_keycode` (Bytes 34-35)
- `zoom_out_keycode` (Bytes 36-37)

## Key Achievements

✅ **Complete VIA value ID interface established**
- 13 new VIA value IDs added (12-24)
- Advanced gesture enables: 3 IDs (12-14)
- 3-finger swipe keycodes: 4 IDs (15-18)
- 4-finger swipe keycodes: 4 IDs (19-22)
- Pinch-to-zoom keycodes: 2 IDs (23-24)
- Sequential numbering pattern maintained
- All IDs have descriptive comments documenting EEPROM layout implications

✅ **Interface ready for Plan 04-04 implementation**
- Value IDs define the VIA interface layer
- Command handlers in 04-04 will use these IDs
- EEPROM layout adjustment documented in comments
- Space constraints acknowledged (20 bytes needed for advanced gesture keycodes)

## Technical Details

### EEPROM Space Requirements

**Advanced gesture keycode storage needs:**
- 3-finger swipes: 4 directions × 2 bytes = 8 bytes
- 4-finger swipes: 4 directions × 2 bytes = 8 bytes
- Zoom gestures: 2 directions × 2 bytes = 4 bytes
- **Total: 20 bytes for advanced gesture keycodes**

**Current state:**
- Existing config uses ~20 bytes (DPI, divisors, basic gesture keycodes)
- Advanced gestures would push total to ~40 bytes
- Exceeds 32-byte VIA custom config limit

**Solution (handled in Plan 04-04):**
1. Audit EEPROM usage
2. Remove obsolete fields (swipe_keycode at Bytes 18-19 is now obsolete with directional 3-finger swipe keycodes)
3. Optimize layout or consider increasing VIA_EEPROM_CUSTOM_CONFIG_SIZE to 64 bytes

### Value ID Interface Pattern

Following the established pattern from Phase 3 (IDs 8-11):
- Enable flags are single bytes (0 or 1)
- Keycodes are 16-bit values (uint16_t)
- Sequential numbering ensures no gaps
- Comments document intended EEPROM byte offsets
- Implementation layer (Plan 04-04) will handle actual storage and retrieval

## Files Modified

**keyboards/bastardkb/dilemma/dilemma.h**
- Extended `via_dilemma_value_id` enum with 13 new IDs (12-24)
- Total lines added: 18 (5 + 5 + 5 + 3)
- Enum properly terminated with closing brace and semicolon

## Verification Results

✅ All 13 new value IDs defined and sequentially numbered (12-24)
✅ Enum properly terminated with closing brace
✅ IDs follow sequential numbering pattern
✅ All IDs have descriptive comments
✅ Interface established for Plan 04-04 command handlers

## Next Steps

**Plan 04-04:** Implement VIA command handlers for advanced gestures
- Add set handlers for 3 advanced gesture enables (IDs 12-14)
- Add set/get handlers for 10 advanced gesture keycodes (IDs 15-24)
- Extend EEPROM struct with new fields
- Optimize layout to fit 32-byte limit (remove obsolete swipe_keycode)
- Initialize defaults in eeconfig_init_kb()
- Version bump to 1 for EEPROM breaking change

**Dependencies:**
- Plan 04-02 (EEPROM layout extension) should be completed first
- This plan (04-03) provides the value ID interface
- Plan 04-04 will implement the actual command handlers

## Risk Assessment

**Overall Risk:** LOW

**Risks:**
- EEPROM space constraints may require layout optimization
- Obsolete field removal (swipe_keycode) is a breaking change

**Mitigations:**
- Plan 04-04 will audit and optimize EEPROM usage
- Version bump to 1 enables migration strategy
- Discovery phase (04-01) identified Option C layout as viable solution

## Success Metrics

✅ VIA value ID enum extended with 13 new IDs (12-24)
✅ Advanced gesture enables have IDs (three_finger, four_finger, pinch_zoom)
✅ Swipe keycode IDs cover both 3-finger and 4-finger directions (8 IDs)
✅ Zoom keycode IDs cover zoom in/out (2 IDs)
✅ Interface established for Plan 04-04 command handlers
✅ Comments document EEPROM layout implications
