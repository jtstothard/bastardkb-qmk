# Plan 02-01: VIA Config Helper Functions - SUMMARY

**Status:** COMPLETE
**Date:** 2026-01-14
**Phase:** 02-via-config-integration (Plan 1 of 3)

## Objective Achieved

Successfully created helper functions to apply VIA config settings to pointing device hardware. This bridges the gap between VIA config storage (created in Phase 1) and actual device behavior, enabling VIA to control the pointing device's DPI.

## Implementation Summary

### Task 1: Create DPI Preset Lookup Function
- **File:** `keyboards/bastardkb/dilemma/dilemma.c`
- **Function:** `get_dpi_from_preset(uint8_t preset)`
- **Purpose:** Maps VIA DPI preset values (0-7) to actual DPI numbers
- **Mapping:**
  - Preset 0: 200 DPI
  - Preset 1: 400 DPI
  - Preset 2: 600 DPI
  - Preset 3: 800 DPI
  - Preset 4: 1000 DPI
  - Preset 5: 1200 DPI
  - Preset 6: Custom DPI value (from `custom_dpi` field)
  - Preset 7: Reserved (returns 0)
- **Design:** Switch statement with defensive programming (returns 0 for invalid presets)
- **Commit:** `5f07cd97a8`

### Task 2: Create VIA Config Application Function
- **File:** `keyboards/bastardkb/dilemma/dilemma.c`
- **Function:** `apply_via_dilemma_config(void)`
- **Purpose:** Applies VIA config settings to pointing device hardware
- **Implementation:**
  1. Reads current `g_via_dilemma_config.dpi_preset`
  2. Calls `get_dpi_from_preset()` to get actual DPI value
  3. Calls `pointing_device_set_cpi()` with the DPI value (if valid)
  4. Returns void
- **Design Pattern:** Follows same pattern as existing `maybe_update_pointing_device_cpi()`
- **Usage Context:** Will be called from:
  - `matrix_init_kb()` after loading VIA config on boot
  - VIA save handler after config changes
  - After DPI preset changes
- **Commit:** `ba768f4de8`

### Task 3: Integrate apply_via_dilemma_config() into Boot Sequence
- **File:** `keyboards/bastardkb/dilemma/dilemma.c`
- **Function Modified:** `matrix_init_kb()`
- **Changes:**
  - Kept existing `read_dilemma_config_from_eeprom()` call
  - Kept existing `read_via_dilemma_config()` call
  - Added `apply_via_dilemma_config()` call after both config reads
  - Kept existing `matrix_init_user()` call
- **Boot Order:**
  1. Read old `dilemma_config` from EEPROM (backward compatibility)
  2. Read VIA custom config from EEPROM
  3. Apply VIA config to pointing device (VIA takes precedence)
  4. Call user initialization
- **Result:** VIA config is applied on keyboard initialization after loading from EEPROM
- **Commit:** `b64d4b4950`

## Technical Details

### DPI Preset System
The preset system provides a balance between convenience and flexibility:

**Standard Presets (0-5):**
- Common DPI values: 200, 400, 600, 800, 1000, 1200
- Cover typical use cases from precision to speed
- Mapped via simple switch statement (fast, no calculation)

**Custom Preset (6):**
- Allows arbitrary DPI value from `custom_dpi` field (12-bit: 0-4095)
- Supports Azoteq's full DPI range
- Enables fine-grained control beyond standard steps

**Reserved Preset (7):**
- Reserved for future expansion
- Returns 0 (safe fallback)

### Configuration Application Flow
```
matrix_init_kb()
  ↓
read_dilemma_config_from_eeprom()  // Old system (backward compat)
  ↓
read_via_dilemma_config()          // New VIA system
  ↓
apply_via_dilemma_config()         // Apply VIA settings
  ↓
  get_dpi_from_preset(dpi_preset)  // Map preset to DPI
  ↓
  pointing_device_set_cpi(dpi)     // Set hardware DPI
  ↓
matrix_init_user()                 // User code
```

### Coexistence with Old System
The implementation maintains full backward compatibility:

**Old System (`dilemma_config_t`):**
- Still loaded on boot
- Still works with keycodes (POINTER_DEFAULT_DPI_FORWARD, etc.)
- Stored in eeconfig_kb (1 byte)

**New VIA System (`via_dilemma_config_t`):**
- Loaded after old system
- Applied to hardware (takes precedence)
- Stored in VIA custom EEPROM region (32 bytes)

**Integration Strategy:**
- Both systems coexist during transition period
- VIA config wins when both are present
- Old keycodes still update old config
- Future phases will add VIA handlers for keycodes

## Verification Checklist

- [x] `get_dpi_from_preset()` function created with correct mapping for presets 0-7
- [x] `apply_via_dilemma_config()` function created to apply VIA config to hardware
- [x] Both functions properly documented with Doxygen comments
- [x] `apply_via_dilemma_config()` integrated into `matrix_init_kb()`
- [x] Boot sequence maintains correct order (old config → VIA config → apply)
- [x] No breaking changes to existing functionality
- [x] Functions follow existing code patterns (e.g., `maybe_update_pointing_device_cpi()`)

## Success Criteria Met

✅ **Helper functions created to bridge VIA config to pointing device**
- `get_dpi_from_preset()` maps presets to DPI values
- `apply_via_dilemma_config()` applies config to hardware
- Both functions are static (internal to dilemma.c)

✅ **DPI preset switching logic implemented**
- All 8 presets (0-7) handled
- Standard presets provide common DPI values
- Custom preset allows full Azoteq range
- Defensive programming (returns 0 for invalid/reserved)

✅ **Boot sequence applies VIA config on startup**
- VIA config loaded from EEPROM on boot
- Config applied before user code runs
- VIA takes precedence over old config
- Old config still loaded for backward compatibility

✅ **Old config system still works**
- No changes to existing `dilemma_config_t` code
- All existing keycodes still function
- Coexistence maintained during transition

## Design Decisions

1. **Switch Statement for Preset Mapping:**
   - Chosen over lookup table for clarity
   - Compiler optimizes to jump table (same performance)
   - Easy to add comments explaining each preset
   - Simplifies custom preset handling (reads from struct)

2. **Defensive Return Value (0):**
   - Invalid presets return 0 instead of crashing
   - `apply_via_dilemma_config()` checks `if (dpi > 0)` before applying
   - Safe fallback if EEPROM corruption occurs
   - Reserved preset 7 won't break system if accidentally selected

3. **Static Function Scope:**
   - Both helper functions are `static`
   - Internal implementation detail of dilemma.c
   - Not exposed in header files
   - Follows principle of least visibility

4. **VIA Takes Precedence:**
   - VIA config applied AFTER old config
   - Ensures VIA settings win on boot
   - Simplifies migration path
   - Users see VIA settings take effect immediately

## Files Modified

1. `/Users/jay/Documents/bastardkb-qmk/keyboards/bastardkb/dilemma/dilemma.c`
   - Added `get_dpi_from_preset()` function (33 lines)
   - Added `apply_via_dilemma_config()` function (41 lines)
   - Modified `matrix_init_kb()` to call `apply_via_dilemma_config()` (+3 lines)

## Commits

- `5f07cd97a8`: feat(via): add DPI preset lookup function
- `ba768f4de8`: feat(via): add VIA config application function
- `b64d4b4950`: feat(via): integrate VIA config application into boot sequence

## Lessons Learned

1. **Helper Functions Bridge Gap:** Phase 1 created storage (VIA config struct), this plan adds the "glue" to actually use it
2. **Coexistence Works Well:** Loading both old and new configs on boot maintains backward compatibility while VIA takes precedence
3. **Defensive Programming Matters:** Returning 0 for invalid presets prevents crashes from bad EEPROM data
4. **Simple Switch Statement:** For small lookup tables (8 items), switch is clearer than array lookup

## Next Steps

**Remaining Phase 2 Plans:**
- Plan 02-02: Implement DPI preset switching via VIA (add handlers to VIA command)
- Plan 02-03: Add drag-scroll divisor configuration

**Phase 2 Status:** 1 of 3 plans complete

**Future Enhancements:**
- Add VIA command handler updates to call `apply_via_dilemma_config()` after DPI changes
- Integrate custom DPI value editing in VIA
- Add scroll divisor application to `apply_via_dilemma_config()`
- Eventually migrate old keycodes to use VIA config

## Conclusion

Plan 02-01 successfully creates the bridge between VIA config storage and actual device behavior. The helper functions are clean, well-documented, and follow existing code patterns. VIA config is now applied on boot, taking precedence over the old system while maintaining full backward compatibility. The foundation is now ready for Plan 02-02 to implement runtime DPI switching through VIA.
