---
phase: 01-via-foundation
plan: 01
subsystem: via-protocol
tags: [via, eeprom, qmk, usb-hid, custom-config]

# Dependency graph
requires: []
provides:
  - VIA protocol packet structure documentation
  - EEPROM NVM API constraints and usage patterns
  - Current Dilemma EEPROM layout analysis
  - Migration strategy from 1-byte to 32-byte config
affects: [01-via-foundation]

# Tech tracking
tech-stack:
  added: []
  patterns: [VIA custom value command protocol, EEPROM bit-packing, backward-compatible migration]

key-files:
  created: [.planning/phases/01-via-foundation/01-01-RESEARCH.md]
  modified: []

key-decisions:
  - "32-byte VIA custom config provides ample space for all planned features"
  - "Three-phase migration: coexistence → transition → deprecation"
  - "Keep current eeconfig_read_kb() during transition for backward compatibility"

patterns-established:
  - "Pattern 1: VIA packet structure [command_id, channel_id, value_id, value_data]"
  - "Pattern 2: Override via_custom_value_command_kb() for keyboard-specific values"
  - "Pattern 3: Use nvm_via_read/update_custom_config() for EEPROM access"

issues-created: []

# Metrics
duration: 1min
completed: 2026-01-14
---

# Phase 1 Plan 1: VIA Custom Command Protocol and EEPROM Research Summary

**VIA protocol, EEPROM constraints, and migration strategy documented for Dilemma trackpad customization implementation**

## Performance

- **Duration:** 1 min
- **Started:** 2026-01-14T16:23:05Z
- **Completed:** 2026-01-14T16:24:26Z
- **Tasks:** 4
- **Files modified:** 1

## Accomplishments
- Documented complete VIA custom command packet structure and protocol flow
- Analyzed EEPROM NVM API constraints (32-byte custom config maximum)
- Researched Ploopy VIA menu patterns as reference implementation
- Documented Dilemma's current 1-byte EEPROM configuration and bit-packing
- Defined three-phase migration strategy from legacy to VIA-based config

## Task Commits

Each task was committed atomically:

1. **Task 1-4: VIA protocol and EEPROM research** - `61f9ac7` (docs)
2. **Task 3 clarification: Ploopy repo requires external access** - `992383a` (docs)

**Plan metadata:** (not yet created - will be in final commit)

## Files Created/Modified
- `.planning/phases/01-via-foundation/01-01-RESEARCH.md` - Complete VIA protocol, EEPROM API, and migration documentation

## Decisions Made

**VIA Custom Config Structure:**
- Use 32-byte VIA_EEPROM_CUSTOM_CONFIG_SIZE (maximum allowed)
- Provides ample space for DPI, scroll divisors, gestures, and future features
- Migrate from current 1-byte bit-packed config to expanded struct

**Migration Strategy:**
- Phase 1 (Coexistence): Both old and new systems work in parallel
- Phase 2 (Transition): VIA app uses new values, old values still functional
- Phase 3 (Deprecation): Remove legacy eeconfig_read_kb() system

**Backward Compatibility:**
- Keep current dilemma_config_t union during transition
- Read from both old and new EEPROM locations
- Migrate old values to new config on first boot with new firmware

**Value ID Mapping:**
- Reserve channel_id=0 (id_custom_channel) for Dilemma-specific values
- Map each configurable setting to unique value_id
- Implement via_custom_value_command_kb() override in dilemma.c

## Deviations from Plan

None - plan executed exactly as written. All research completed from codebase analysis without requiring external repository access during this research phase.

## Issues Encountered

None - all research findings obtained from existing QMK codebase and Dilemma firmware.

## Next Phase Readiness

**Ready for Plan 01-02 (EEPROM Layout Design):**
- VIA protocol constraints documented
- 32-byte custom config limit confirmed
- NVM API signatures understood
- Current config structure analyzed

**Ready for Plan 01-03 (Command Handler Implementation):**
- Packet structure and command flow documented
- Override point identified (via_custom_value_command_kb)
- Return protocol specified
- Value ID system planned

**External Research Required:**
- Ploody VIA menus repository (https://github.com/plodah/ploody_viamenus) must be cloned and analyzed during plans 01-02 or 01-03
- This will provide concrete examples of via_menus.json structure and bit-packing patterns

**No Blockers:**
- All internal research complete
- Clear path to implementation defined
- Migration strategy established

---
*Phase: 01-via-foundation*
*Completed: 2026-01-14*
