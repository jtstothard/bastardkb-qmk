# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2025-01-14)

**Core value:** VIA customization interface - If this fails, nothing else matters. Every feature (gestures, scroll divisors, DPI controls) must be configurable through VIA without requiring firmware rebuilds.
**Current focus:** Phase 1 — VIA Integration Foundation

## Current Position

Phase: 2 of 12 (VIA Configuration Integration)
Plan: 1 of 3 in current phase
Status: Plan 02-01 complete, ready for 02-02
Last activity: 2026-01-14 — Completed Plan 02-01 (VIA Config Helper Functions)

Progress: ████████░░░ 33%

## Performance Metrics

**Velocity:**
- Total plans completed: 4
- Average duration: 17 min
- Total execution time: 1.1 hours

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| 1. VIA Integration Foundation | 3 | 3 | 18 min |
| 2. VIA Configuration Integration | 1 | 3 | 10 min |

**Recent Trend:**
- Last 5 plans: 01-01 (25 min), 01-02 (15 min), 01-03 (15 min), 02-01 (10 min)
- Trend: Improving | Faster execution

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

### Deferred Issues

None yet.

### Pending Todos

None yet.

### Blockers/Concerns

None yet.

## Session Continuity

Last session: 2026-01-14
Stopped at: Completed Plan 02-01 (VIA Config Helper Functions), ready for 02-02
Resume file: None

## Phase 2 Progress Summary

**Phase 2: VIA Configuration Integration** - 1 of 3 plans complete

**Completed Plans:**
1. ✅ 02-01: VIA Config Helper Functions (10 min)
   - Created `get_dpi_from_preset()` to map DPI presets to values
   - Created `apply_via_dilemma_config()` to apply VIA config to hardware
   - Integrated application into `matrix_init_kb()` boot sequence
   - Summary: `.planning/phases/02-via-config-integration/02-01-SUMMARY.md`

**Remaining Plans:**
2. 02-02: Implement DPI preset switching via VIA
3. 02-03: Add drag-scroll divisor configuration

**Key Achievements in Phase 2:**
- ✅ Bridge created between VIA config storage and device behavior
- ✅ DPI preset mapping implemented (8 presets: 6 standard + custom + reserved)
- ✅ Boot sequence applies VIA config on startup
- ✅ Full backward compatibility maintained
- ✅ Old config system still works alongside VIA

**Files Modified in Phase 2:**
- `keyboards/bastardkb/dilemma/dilemma.c` (+77 lines, 3 functions)

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
