# Plan 01-03: VIA Custom Value Command Handler Implementation - SUMMARY

**Status:** COMPLETE
**Date:** 2026-01-14
**Phase:** 01-via-foundation (Plan 3 of 3)

## Objective Achieved

Successfully implemented `via_custom_value_command_kb()` handler with read/write/validate operations for VIA custom configuration system. This completes Phase 1's VIA Integration Foundation, enabling full VIA protocol communication for custom settings.

## Implementation Summary

### Task 1: Enable VIA Custom Config in config.h
- **File:** `keyboards/bastardkb/dilemma/config.h`
- **Change:** Added `#define VIA_EEPROM_CUSTOM_CONFIG_SIZE 32`
- **Result:** Enabled 32-byte custom EEPROM region in VIA system
- **Commit:** `8dc3bcde46`

### Task 2: Define Value ID Enumeration in dilemma.h
- **File:** `keyboards/bastardkb/dilemma/dilemma.h`
- **Changes:**
  - Added `enum via_dilemma_value_id` with 8 value IDs
  - Declared external reference to `g_via_dilemma_config`
- **Value IDs Defined:**
  - `id_dilemma_unhandled` (0): Required by VIA protocol
  - `id_dilemma_config_save` (1): Save command
  - `id_dilemma_dpi_preset` (2): DPI preset selection (3-bit)
  - `id_dilemma_custom_dpi` (3): Custom DPI value (12-bit)
  - `id_dilemma_drag_scroll_x` (4): Drag scroll X divisor (4-bit)
  - `id_dilemma_drag_scroll_y` (5): Drag scroll Y divisor (4-bit)
  - `id_dilemma_two_finger_x` (6): Two-finger X divisor (4-bit)
  - `id_dilemma_two_finger_y` (7): Two-finger Y divisor (4-bit)
- **Commit:** `0afab64aec`

### Task 3: Implement VIA Custom Value Command Handler
- **File:** `keyboards/bastardkb/dilemma/dilemma.c`
- **Components Added:**
  1. **Global config instance:** `via_dilemma_config_t g_via_dilemma_config`
  2. **EEPROM read function:** `read_via_dilemma_config()`
     - Loads 32-byte config from VIA EEPROM region
  3. **EEPROM write function:** `write_via_dilemma_config()`
     - Persists config to VIA EEPROM region
  4. **Command handler:** `via_custom_value_command_kb()`
     - Implements three VIA commands:
       - `id_custom_set_value` (0x07): Write config values
       - `id_custom_get_value` (0x08): Read config values
       - `id_custom_save` (0x09): Persist to EEPROM
- **Handler Details:**
  - Parses VIA protocol data structure: `[command_id, channel_id, value_id, value_data...]`
  - Switches on command ID and value ID
  - Handles multi-byte values (custom_dpi as 16-bit across 2 bytes)
  - Returns `id_unhandled` for unknown value IDs
  - Does not break existing `dilemma_config_t` system (coexistence)
- **Commit:** `1ef2ede9ad`

### Task 4: Initialize VIA Config on Boot
- **File:** `keyboards/bastardkb/dilemma/dilemma.c`
- **Changes:**
  1. **matrix_init_kb():**
     - Added `read_via_dilemma_config()` call
     - Loads VIA config on every boot
  2. **eeconfig_init_kb():**
     - Added VIA config initialization with defaults
     - Default values:
       - `dpi_preset = 3` (1000 DPI)
       - `drag_scroll_x_divisor = 8`
       - `drag_scroll_y_divisor = 8`
       - `two_finger_x_divisor = 8`
       - `two_finger_y_divisor = 8`
       - `config_version = 1`
     - Calls `write_via_dilemma_config()` to persist defaults
- **Result:** VIA config integrated with boot sequence without breaking existing functionality
- **Commit:** `c95d92df47`

## Technical Details

### VIA Protocol Integration
The implementation follows VIA's custom value protocol:
- **Data Structure:** `uint8_t data[] = [command_id, channel_id, value_id, value_data...]`
- **Command IDs:**
  - `id_custom_set_value = 0x07`: Set a value
  - `id_custom_get_value = 0x08`: Get a value
  - `id_custom_save = 0x09`: Save to EEPROM
- **Channel ID:** Always 0 (custom channel) for Dilemma

### EEPROM Layout
- **Location:** VIA custom config region (separate from main eeconfig)
- **Size:** 32 bytes (maximum allowed by VIA)
- **Access Functions:**
  - `nvm_via_read_custom_config(buf, offset, length)`
  - `nvm_via_update_custom_config(buf, offset, length)`
- **Structure:** Bit-field packed union (defined in 01-02)

### Coexistence Strategy
- **Old system:** `dilemma_config_t` (1 byte, eeconfig_kb)
- **New system:** `via_dilemma_config_t` (32 bytes, VIA custom region)
- **Separation:** Complete isolation between systems
- **Migration:** Will happen in Phase 2 (not this plan)

## Verification Checklist

- [x] VIA_EEPROM_CUSTOM_CONFIG_SIZE set to 32 in config.h
- [x] Value ID enumeration defined with 8 IDs
- [x] via_custom_value_command_kb() implemented with get/set/save handlers
- [x] Global config instance declared and initialized
- [x] Config loaded on matrix_init_kb()
- [x] Config defaults set in eeconfig_init_kb()
- [x] Does NOT break existing Dilemma functionality
- [x] Code follows VIA protocol specification

## Success Criteria Met

✅ **VIA custom command system fully functional**
- Can receive read/write/save commands from VIA
- EEPROM operations working correctly
- Boot initialization working without breaking existing features
- Foundation ready for Phase 2 to add more settings

✅ **Code Quality**
- All tasks committed individually
- No modifications to existing config system
- Clear separation of concerns
- Follows QMK VIA patterns

## Phase 1 Status: COMPLETE

This plan completes Phase 1 (VIA Integration Foundation). All three plans achieved:

1. **01-01:** VIA protocol research and API understanding
2. **01-02:** EEPROM struct design with 30+ settings in 32 bytes
3. **01-03:** VIA custom value command handler implementation

**Phase 1 Deliverables:**
- ✅ VIA EEPROM region enabled (32 bytes)
- ✅ Config structure designed and documented
- ✅ Value ID system defined (8 IDs so far)
- ✅ Command handler implemented (get/set/save)
- ✅ Boot integration complete
- ✅ No breaking changes to existing code

## Next Steps

**Phase 2: VIA Configuration Integration** (Ready to begin)
- Plan 02-01: Migrate pointing device config to VIA system
- Plan 02-02: Implement DPI preset switching via VIA
- Plan 02-03: Add drag-scroll divisor configuration

The foundation is now in place to start exposing actual functionality through VIA, beginning with DPI configuration.

## Files Modified

1. `/Users/jay/Documents/bastardkb-qmk/keyboards/bastardkb/dilemma/config.h`
2. `/Users/jay/Documents/bastardkb-qmk/keyboards/bastardkb/dilemma/dilemma.h`
3. `/Users/jay/Documents/bastardkb-qmk/keyboards/bastardkb/dilemma/dilemma.c`

## Commits

- `8dc3bcde46`: feat(via): enable VIA custom config EEPROM region
- `0afab64aec`: feat(via): define VIA custom value ID enumeration
- `1ef2ede9ad`: feat(via): implement VIA custom value command handler
- `c95d92df47`: feat(via): integrate VIA config with boot sequence

## Lessons Learned

1. **VIA Protocol Simplicity:** The VIA custom value system is straightforward - just implement a handler that switches on command ID and value ID
2. **EEPROM Separation:** Using VIA's custom config region keeps changes isolated from main eeconfig
3. **Bit-Field Efficiency:** The 32-byte limit encourages efficient packing (achieved 57.8% utilization)
4. **Coexistence Strategy:** Running old and new config systems in parallel makes migration safer

## Conclusion

Plan 01-03 successfully completes Phase 1 of the Dilemma v3 VIA Customization Enhancement. The VIA integration foundation is now solid, with a working command handler that can read and write configuration settings through VIA. The implementation is clean, well-structured, and ready for Phase 2 to start adding actual functionality.
