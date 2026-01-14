---
phase: 01-via-foundation
plan: 02
subsystem: firmware
tags: [qmk, via, eeprom, bit-packing, struct-design]

# Dependency graph
requires:
  - phase: 01-01
    provides: VIA custom command protocol research and EEPROM patterns
provides:
  - via_dilemma_config_t union struct with 32-byte packed layout
  - Bit-efficient mapping for 30+ settings across DPI, scroll, gestures, and keycodes
  - Versioning strategy for EEPROM format evolution
  - 13.5 bytes reserved space for future feature expansion
affects: [02-EEPROM-Configuration-System, 03-DPI-Configuration, 04-Scroll-Speed-Controls, 05-Auto-Sniping-Toggle, 06-Basic-Gesture-Enablement, 07-Advanced-Gesture-Support, 08-Gesture-to-Keycode-Mapping, 09-Smart-Gesture-Features]

# Tech tracking
tech-stack:
  added: []
  patterns: [bit-field packing, union struct for dual byte/struct access, __attribute__((packed)) for EEPROM layout control, version field for migration strategy]

key-files:
  created: []
  modified: [keyboards/bastardkb/dilemma/dilemma.h]

key-decisions:
  - "32-byte VIA custom config limit accepted as constraint"
  - "Bit-field packing chosen over byte-aligned fields for maximum density"
  - "Related settings grouped (DPI, scroll, gestures) for logical organization"
  - "13.5 bytes reserved for future expansion (Phase 10+ features)"
  - "4-bit version field supports 16 EEPROM format revisions"

patterns-established:
  - "Union struct pattern: raw[32] byte array + packed bit-field struct for EEPROM read/write"
  - "Versioning strategy: config_version field with reset-to-default on mismatch"
  - "Reserved fields naming: reserved_N where N is starting byte offset"
  - "Comment annotation: byte ranges for multi-byte fields (e.g., 'Byte 8-9')"

issues-created: []

# Metrics
duration: 15min
completed: 2026-01-14
---

# Phase 1: VIA Integration Foundation Summary

**32-byte EEPROM union struct with bit-packed layout for 30+ VIA-configurable settings including DPI presets, scroll divisors, gesture enables, keycode mappings, and smart gesture features**

## Performance

- **Duration:** 15 min
- **Started:** 2026-01-14T16:09:00Z
- **Completed:** 2026-01-14T16:24:15Z
- **Tasks:** 3/3
- **Files modified:** 1

## Accomplishments

- Designed complete EEPROM data structure for all 12 phases of VIA customization
- Verified 32-byte constraint with 57.8% utilization (148 bits used, 108 bits reserved)
- Created versioning strategy supporting 16 format revisions with migration path
- Organized settings by functional groups for logical code structure
- Tested struct compilation and verified exact 32-byte size with gcc

## Task Commits

All tasks committed as single atomic change:

1. **Tasks 1-3: EEPROM struct design** - `6a615d1a08` (feat)

**Plan metadata:** (will be added after SUMMARY creation)

## Files Created/Modified

- `keyboards/bastardkb/dilemma/dilemma.h` - Added via_dilemma_config_t union with complete 32-byte layout covering all planned settings from phases 3-12

## EEPROM Layout Details

### Byte-by-Byte Breakdown

**Byte 0: DPI Configuration**
- `dpi_preset` (3 bits): Preset index 0-7
- `reserved_0` (5 bits): Future DPI features

**Bytes 1-2: Custom DPI**
- `custom_dpi` (12 bits): Value 0-4095
- `reserved_1` (4 bits): Future precision

**Byte 3: Drag Scroll Divisors**
- `drag_scroll_x_divisor` (4 bits): 0-15
- `drag_scroll_y_divisor` (4 bits): 0-15

**Byte 4: Two-Finger Scroll Divisors**
- `two_finger_x_divisor` (4 bits): 0-15
- `two_finger_y_divisor` (4 bits): 0-15

**Byte 5: Auto-Sniping**
- `auto_snipe_enabled` (1 bit): Toggle
- `auto_snipe_layer` (7 bits): Layer 0-127

**Byte 6: Basic Gesture Enables**
- `tap_to_click_enabled` (1 bit)
- `two_finger_tap_enabled` (1 bit)
- `two_finger_scroll_enabled` (1 bit)
- `press_and_hold_enabled` (1 bit)
- `reserved_6` (4 bits): Future gestures

**Byte 7: Advanced Gesture Enables**
- `three_finger_swipe_enabled` (1 bit)
- `four_finger_swipe_enabled` (1 bit)
- `pinch_to_zoom_enabled` (1 bit)
- `reserved_7` (5 bits): Future advanced

**Bytes 8-19: Gesture Keycodes (12 bytes)**
- `tap_to_click_keycode` (16 bits): Byte 8-9
- `two_finger_tap_keycode` (16 bits): Byte 10-11
- `two_finger_scroll_up_keycode` (16 bits): Byte 12-13
- `two_finger_scroll_down_keycode` (16 bits): Byte 14-15
- `press_and_hold_keycode` (16 bits): Byte 16-17
- `swipe_keycode` (16 bits): Byte 18-19

**Byte 20: Smart Gesture Features**
- `tap_pressure_threshold` (4 bits): 0-15
- `force_click_enabled` (1 bit)
- `smart_zoom_enabled` (1 bit)
- `reserved_20` (2 bits): Future smart features

**Bytes 21-30: Reserved Expansion (10 bytes)**
- `reserved_21[10]`: Phase 10+ features

**Byte 31: Versioning**
- `config_version` (4 bits): Format version
- `reserved_31` (4 bits): Future versioning

### Bit Count Summary

- **DPI settings:** 15 bits (3 + 12)
- **Scroll divisors:** 16 bits (4 × 4)
- **Auto-sniping:** 8 bits (1 + 7)
- **Basic gestures:** 4 bits (4 × 1)
- **Advanced gestures:** 3 bits (3 × 1)
- **Gesture keycodes:** 96 bits (6 × 16)
- **Smart features:** 6 bits (4 + 1 + 1)
- **Versioning:** 4 bits
- **Reserved:** 108 bits

**Total:** 148 bits used + 108 bits reserved = 256 bits (32 bytes)

## Decisions Made

- **Union struct pattern:** Chose union with raw[32] + packed struct for dual access modes (byte array for EEPROM I/O, struct fields for code)
- **Bit-field packing:** Used compiler bit-fields instead of manual shift/mask for readability and maintainability
- **Grouped organization:** Arranged related settings together (DPI, scroll, gestures) to minimize padding and improve code clarity
- **Reserved space allocation:** Reserved 13.5 bytes upfront for future phases rather than tight packing to enable backwards-compatible additions
- **4-bit version field:** Supports 16 format revisions, sufficient for project lifetime
- **Comment annotation:** Added byte offset comments for multi-byte fields to aid debugging and VIA menu implementation

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None - struct compiled successfully and verified as exactly 32 bytes on first attempt.

## Next Phase Readiness

**Ready for Phase 2: EEPROM Configuration System**

The struct design provides:
- Complete data structure definition for persistence layer
- Clear field organization for default value initialization
- Version field for format validation on EEPROM read
- Reserved space identified for future feature additions

Phase 2 will implement:
- Default values for each setting
- EEPROM read/write functions with validation
- Settings reset to defaults command
- Version checking and migration logic

**No blockers or concerns.** The struct is well-designed, tested, and ready for implementation.

---
*Phase: 01-via-foundation*
*Completed: 2026-01-14*
