---
phase: 04-advanced-gesture-support
plan: 01
subsystem: firmware
tags: [qmk, gestures, state-machine, via-protocol, eeprom, digitizer]

# Dependency graph
requires:
  - phase: 03-basic-gesture-enablement
    provides: gesture filtering infrastructure, VIA config pattern
provides:
  - Technical foundation for 3/4-finger swipe distinction
  - Pinch-to-zoom detection algorithm specification
  - Complete Phase 4 implementation roadmap (5 plans)
  - EEPROM layout design for advanced gestures
  - Risk assessment with mitigations
affects: [04-02, 04-03, 04-04, 04-05, 04-06]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - State machine extension pattern (Swipe → Zoom states)
    - Squared distance comparison for embedded math (no sqrt)
    - Option C EEPROM layout (partial VIA config + hardcoded defaults)
    - Finger count tracking at gesture entry
    - Opt-in gesture features (disabled by default)

key-files:
  created: [.planning/phases/04-advanced-gesture-support/04-01-DISCOVERY.md]
  modified: []

key-decisions:
  - "Option C EEPROM layout: 3-finger VIA-configurable, 4-finger hardcoded defaults (balances space vs flexibility)"
  - "Squared distance comparison for zoom: avoids sqrt(), uses integer math only"
  - "Opt-in pinch-to-zoom: disabled by default, requires threshold tuning"
  - "Version bump to 1: breaking change to EEPROM structure (remove swipe_keycode, add directional fields)"

patterns-established:
  - "Discovery pattern: document current implementation → identify gaps → design solutions → create roadmap"
  - "Risk assessment format: technical/integration/UX risks with probability/impact/mitigation tables"
  - "Dependency graph visualization: ASCII art showing parallelization opportunities"
  - "EEPROM constraint analysis: explore options A/B/C, weigh tradeoffs, recommend solution"

issues-created: []

# Metrics
duration: 15min
completed: 2026-01-14
---

# Phase 04 Plan 01: Advanced Gesture Discovery Summary

**Comprehensive technical foundation for 3/4-finger swipe distinction and pinch-to-zoom detection with complete Phase 4 implementation roadmap**

## Performance

- **Duration:** 15 minutes (4 tasks, 4 atomic commits)
- **Started:** 2026-01-14T14:00:00Z (estimated)
- **Completed:** 2026-01-14T14:15:00Z (estimated)
- **Tasks:** 4 completed
- **Files modified:** 1 file created (1,435+ lines)

## Accomplishments

- **Documented current swipe gesture implementation** - Mapped digitizer_mouse_fallback.c state machine with line number references, documented keycode mappings, identified gap (no 3 vs 4 finger distinction)
- **Designed 3 vs 4 finger swipe distinction** - Specified firmware changes (swipe_finger_count tracking), analyzed EEPROM constraints, proposed Option C layout (3-finger VIA-configurable, 4-finger hardcoded defaults)
- **Specified pinch-to-zoom detection algorithm** - Designed gesture pattern (pinch vs spread), implementation requirements (distance calculation, threshold-based triggering), state machine extension (Zoom state), keycode mappings (KC_EQUAL/KC_MINUS)
- **Created complete Phase 4 implementation roadmap** - Detailed 5 plans (04-02 through 04-06), mapped dependencies, identified parallelization opportunities, assessed risks with mitigations

## Task Commits

Each task was committed atomically:

1. **Task 1: Document existing swipe gesture implementation** - `e8e1235f40` (docs)
2. **Task 2: Identify technical requirements for 3 vs 4 finger distinction** - `eceedd1bb9` (docs)
3. **Task 3: Document pinch-to-zoom detection algorithm** - `a2a3f5bc52` (docs)
4. **Task 4: Create implementation strategy section** - `9d182cc31c` (docs)

**Plan metadata:** `9d182cc31c` (docs: complete 04-01 DISCOVERY with implementation strategy)

## Files Created/Modified

- `.planning/phases/04-advanced-gesture-support/04-01-DISCOVERY.md` - Complete technical foundation document (1,435+ lines covering current implementation, 3/4 finger requirements, pinch-to-zoom algorithm, implementation strategy)

## Decisions Made

**Key Design Decisions:**

1. **Option C EEPROM Layout** - Chose balanced approach: 3-finger swipes fully VIA-configurable (8 bytes), 4-finger swipes use hardcoded macOS-oriented defaults (saves 8 bytes), pinch-to-zoom fully VIA-configurable (4 bytes). Fits in 32-byte limit, prioritizes most common use case (3-finger customization).

2. **Squared Distance Comparison for Zoom** - Avoids sqrt() function (not available in QMK math library), uses integer math only, compares squared distances to squared thresholds. Reduces complexity, improves performance, maintains accuracy.

3. **Opt-in Pinch-to-Zoom** - Disabled by default (pinch_to_zoom_enabled = 0), requires threshold tuning for usability (start with 500 pixels), optional feature prevents breaking existing user configs.

4. **Version Bump to 1** - Breaking change to EEPROM structure (remove swipe_keycode field, add directional 3-finger fields), enables migration strategy for existing configs.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None - all documentation tasks completed successfully.

## Next Phase Readiness

**Ready for Plan 04-02:** Extend Firmware for Finger Count Tracking

- Clear specification of firmware changes (add swipe_finger_count variable, store on Swipe state entry, branch keycode logic)
- File locations identified (digitizer_mouse_fallback.c lines 175-310)
- Test criteria defined (3-finger vs 4-finger produce different keycodes, no regressions)

**Ready for Plan 04-03:** Add VIA Value IDs (can run in parallel with 04-02)

- VIA value IDs 12-17 specified with clear purposes
- EEPROM struct changes documented (remove swipe_keycode, add 6 new fields)
- Byte-level layout analysis complete (fits in 32 bytes)

**Foundation Established:**

- Complete technical foundation for all Phase 4 work
- Risk assessment with mitigations for all 5 remaining plans
- Dependency graph with parallelization opportunities
- Success metrics defined for each plan
- Estimated timeline: 75-100 min (5 plans × 15-20 min)

**No Blockers or Concerns**

All research complete, no unknowns remaining, ready to proceed with implementation.

---
*Phase: 04-advanced-gesture-support*
*Completed: 2026-01-14*
