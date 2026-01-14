# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2025-01-14)

**Core value:** VIA customization interface - If this fails, nothing else matters. Every feature (gestures, scroll divisors, DPI controls) must be configurable through VIA without requiring firmware rebuilds.
**Current focus:** Phase 3 — Basic Gesture Enablement

## Current Position

Phase: 3 of 12 (Basic Gesture Enablement)
Plan: 3 of 4 in current phase
Status: Plan 03-03 complete
Last activity: 2026-01-14 — Completed Plan 03-03 (VIA Gesture Enable Command Handlers)

Progress: ██████████░░ 69%

## Performance Metrics

**Velocity:**
- Total plans completed: 9
- Average duration: 15 min
- Total execution time: 2.25 hours

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| 1. VIA Integration Foundation | 3 | 3 | 18 min |
| 2. VIA Configuration Integration | 3 | 3 | 11 min |
| 3. Basic Gesture Enablement | 3 | 4 | 15 min |

**Recent Trend:**
- Last 5 plans: 02-02 (12 min), 02-03 (10 min), 03-01 (15 min), 03-02 (15 min), 03-03 (15 min)
- Trend: Stable | Consistent execution

## Accumulated Context

### Decisions

Decisions are logged in PROJECT.md Key Decisions table.
Recent decisions affecting current work:

| Phase | Decision | Rationale |
|-------|----------|-----------|
| 1 | 32-byte VIA custom config provides ample space for all planned features | Maximum allowed by VIA_EEPROM_CUSTOM_CONFIG_SIZE |
| 1 | Three-phase migration: coexistence → transition → deprecation | Maintains backward compatibility during transition |
| 1 | Keep current eeconfig_read_kb() during transition | Allows old configs to work with new firmware |
| 1 | Union struct pattern for EEPROM (raw + packed fields) | Enables dual access: byte array for I/O, struct for code clarity |
| 1 | Bit-field packing for maximum density | 30+ settings in 32 bytes with 57.8% utilization |
| 1 | Grouped organization (DPI, scroll, gestures) | Logical structure, minimizes padding, improves maintainability |
| 1 | 13.5 bytes reserved for future expansion | Enables backwards-compatible additions in Phases 10+ |
| 1 | 4-bit version field for migration strategy | Supports 16 format revisions, sufficient for project lifetime |
| 2 | Apply scroll divisors at dilemma.c level, not digitizer driver | dilemma.c has VIA config access, cleaner separation |
| 2 | Update divisors on each pointing_device_task iteration | Low overhead, ensures immediate responsiveness to mode changes |
| 2 | Separate divisors for drag-scroll vs two-finger scroll | Different use cases need different scroll speeds |
| 2 | Validate VIA input at set handlers, not get handlers | Fail-fast on invalid input prevents config corruption |
| 2 | Apply VIA config immediately on save and mode changes | No reboot required, responsive UX |
| 2 | Keep apply_via_dilemma_config() calling update_scroll_divisors() | Cleaner than separate calls, ensures consistency |
| 3 | Firmware-based gesture filtering required (MaxTouch has no hardware gesture registers) | MaxTouch MXT336U lacks GESTURE_EVENTS like Azoteq IQS5xx, must filter at firmware level |
| 3 | Use VIA config flags to enable/disable gestures at state machine level | Consistent with existing VIA pattern, firmware controls behavior not hardware |
| 3 | Default all gestures ENABLED for backward compatibility | Prevent breaking existing user configs, opt-out via VIA |
| 3 | Intercept digitizer_update_mouse_report() state machine for filtering | Clean integration point, minimal code changes, check flags before state transitions |
| 3 | Static gesture state structure tracks gesture events for VIA filtering | Follows Phase 2 pattern (g_current_scroll_x/y_divisor), enables centralized state management |
| 3 | Public API accessors for gesture state (dilemma_get_*) | Follows existing pattern (dilemma_get_pointer_sniping_enabled), enables external querying |

### Deferred Issues

None yet.

### Pending Todos

None yet.

### Blockers/Concerns

None yet.

## Session Continuity

Last session: 2026-01-14
Stopped at: Completed Plan 03-02 (Gesture Event Tracking)
Resume file: None

## Plan 03-03 Summary

**Duration:** 15 minutes (4 tasks, 4 atomic commits)
**Status:** ✅ COMPLETE

**Completed Tasks:**
1. ✅ Added VIA value IDs for 4 gesture enables (8-11) to dilemma.h
2. ✅ Added VIA set handlers for gesture enables to dilemma.c
3. ✅ Added VIA get handlers for gesture enables to dilemma.c
4. ✅ Initialized gesture enable defaults in eeconfig_init_kb()

**Key Achievements:**
- Complete VIA read/write access for 4 basic gesture enable flags
- No validation needed - boolean flags (0 or 1) are always valid
- Sensible defaults matching macOS trackpad behavior
- Clean pattern adherence following existing DPI/scroll divisor implementations
- Infrastructure ready for Plan 03-04 to implement gesture filtering logic

**Files Modified:**
- keyboards/bastardkb/dilemma/dilemma.h (+5 lines, VIA value IDs 8-11)
- keyboards/bastardkb/dilemma/dilemma.c (+28 lines, set/get handlers + defaults)

**Summary:** .planning/phases/03-basic-gesture-enablement/03-03-SUMMARY.md

## Plan 03-02 Summary

**Duration:** 15 minutes (4 tasks, 4 atomic commits)
**Status:** ✅ COMPLETE

**Completed Tasks:**
1. ✅ Added gesture state tracking structure (g_gesture_state) to dilemma.c
2. ✅ Created update_gesture_state() function with comprehensive TODO
3. ✅ Integrated gesture state updates into pointing_device_task_dilemma()
4. ✅ Added public API getter functions for all gesture types

**Key Achievements:**
- Gesture state infrastructure established
- Public API provides 7 getter functions (single_tap, two_finger_tap, swipes, press_and_hold)
- Integration points ready for Plan 03-03 VIA config filtering
- Comprehensive TODO documents MaxTouch firmware-based detection
- Follows Phase 2 patterns (static globals, accessor functions)

**Files Modified:**
- keyboards/bastardkb/dilemma/dilemma.c (+81 lines)
- keyboards/bastardkb/dilemma/dilemma.h (+8 lines)

**Summary:** .planning/phases/03-basic-gesture-enablement/03-02-SUMMARY.md

## Phase 2 Progress Summary

**Phase 2: VIA Configuration Integration** ✅ COMPLETE

**Completed Plans:**
1. ✅ 02-01: VIA Config Helper Functions (10 min)
   - Created `get_dpi_from_preset()` to map DPI presets to values
   - Created `apply_via_dilemma_config()` to apply VIA config to hardware
   - Integrated application into `matrix_init_kb()` boot sequence
   - Summary: `.planning/phases/02-via-config-integration/02-01-SUMMARY.md`

2. ✅ 02-02: VIA Scroll Divisor Configuration (12 min)
   - Added global divisor tracking variables (`g_current_scroll_x/y_divisor`)
   - Created `update_scroll_divisors()` to select divisors based on mode
   - Updated `pointing_device_task_dilemma()` to apply VIA divisors
   - Separate divisors for drag-scroll and two-finger scroll modes
   - Automatic divisor updates on mode changes and VIA config saves
   - Summary: `.planning/phases/02-via-config-integration/02-02-SUMMARY.md`

3. ✅ 02-03: VIA Config Application Integration (10 min)
   - Integrated VIA config application into VIA save handler
   - Added VIA config application to mode change handlers (sniping/drag-scroll)
   - Implemented DPI preset validation (0-6 range, reject > 6)
   - Implemented custom DPI validation (clamp to 200-4000 range)
   - VIA config changes now take effect immediately without reboot
   - Full backward compatibility maintained
   - Summary: `.planning/phases/02-via-config-integration/02-03-SUMMARY.md`

**Key Achievements in Phase 2:**
- ✅ Bridge created between VIA config storage and device behavior
- ✅ DPI preset mapping implemented (8 presets: 6 standard + custom + reserved)
- ✅ Boot sequence applies VIA config on startup
- ✅ Scroll divisors now configurable via VIA (separate for drag/two-finger)
- ✅ Automatic divisor updates on mode changes
- ✅ VIA config changes take effect immediately (no reboot required)
- ✅ Input validation prevents invalid config values
- ✅ Full backward compatibility maintained
- ✅ Old config system still works alongside VIA

**Files Modified in Phase 2:**
- `keyboards/bastardkb/dilemma/dilemma.c` (+118 lines, 5 functions)

## Phase 3 Progress Summary

**Phase 3: Basic Gesture Enablement** 🔄 IN PROGRESS (3/4 complete)

**Completed Plans:**
1. ✅ 03-01: Gesture Event Flow Discovery (15 min)
   - Researched MaxTouch MXT336U gesture capabilities
   - DISCOVERY: MaxTouch has NO hardware gesture registers (unlike Azoteq IQS5xx)
   - Documented firmware-based gesture state machine in digitizer_mouse_fallback.c
   - Analyzed gesture detection: Tap, Swipe, Scroll, Drag
   - Documented VIA integration strategy: Filter at state machine level
   - Summary: `.planning/phases/03-basic-gesture-enablement/03-01-SUMMARY.md`
   - Discovery: `.planning/phases/03-basic-gesture-enablement/03-01-DISCOVERY.md`

2. ✅ 03-02: Gesture Event Tracking (15 min)
   - Added gesture state tracking structure (g_gesture_state) to dilemma.c
   - Created update_gesture_state() function with TODO placeholder
   - Integrated gesture state updates into pointing_device_task_dilemma()
   - Added public API getter functions for all gesture types
   - Established infrastructure for VIA config filtering in Plan 03-03
   - Summary: `.planning/phases/03-basic-gesture-enablement/03-02-SUMMARY.md`

3. ✅ 03-03: VIA Gesture Enable Command Handlers (15 min)
   - Added 4 VIA value IDs (8-11) for gesture enables to dilemma.h
   - Added VIA set handlers for all 4 gesture enables in dilemma.c
   - Added VIA get handlers for all 4 gesture enables in dilemma.c
   - Initialized gesture enable defaults matching macOS trackpad behavior
   - Summary: `.planning/phases/03-basic-gesture-enablement/03-03-SUMMARY.md`

**Key Achievements in Plan 03-03:**
- ✅ Complete VIA read/write access for 4 basic gesture enable flags
- ✅ No validation needed - boolean flags (0 or 1) are always valid
- ✅ Sensible defaults matching macOS trackpad behavior
- ✅ Clean pattern adherence following existing DPI/scroll divisor implementations
- ✅ Infrastructure ready for Plan 03-04 to implement gesture filtering logic

**Upcoming Plans:**
4. 03-04: Implement Gesture Filtering
   - Modify digitizer_update_mouse_report() state machine to check gesture enables
   - Complete update_gesture_state() TODO with actual integration
   - Add gesture config checks before gesture detection
   - Test gesture enable/disable functionality through VIA

**Files Modified in Phase 3:**
- `keyboards/bastardkb/dilemma/dilemma.c` (+109 lines, gesture state structure + function + accessors + VIA handlers)
- `keyboards/bastardkb/dilemma/dilemma.h` (+13 lines, accessor declarations + VIA value IDs)

## Phase 1 Completion Summary

**Phase 1: VIA Integration Foundation** ✅ COMPLETE
- Plans: 3/3 completed
- Duration: 55 minutes total
- Summary: `.planning/phases/01-via-foundation/01-03-SUMMARY.md`

**Deliverables:**
1. VIA protocol research and API understanding (01-01)
2. EEPROM struct design with 30+ settings in 32 bytes (01-02)
3. VIA custom value command handler implementation (01-03)

**Key Achievements:**
- ✅ VIA EEPROM region enabled (32 bytes)
- ✅ Config structure designed (via_dilemma_config_t)
- ✅ Value ID system defined (8 IDs implemented)
- ✅ Command handler working (get/set/save operations)
- ✅ Boot integration complete (no breaking changes)
- ✅ Foundation ready for Phase 2

**Files Modified:**
- `keyboards/bastardkb/dilemma/config.h` (VIA enable)
- `keyboards/bastardkb/dilemma/dilemma.h` (value IDs, extern declaration)
- `keyboards/bastardkb/dilemma/dilemma.c` (handler, init, defaults)

**Next Phase:** 02 - VIA Configuration Integration
- Plan 02-01: Migrate pointing device config to VIA
- Plan 02-02: Implement DPI preset switching
- Plan 02-03: Add drag-scroll divisor configuration
