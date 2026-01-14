---
phase: 03-basic-gesture-enablement
plan: 01
subsystem: pointing-device
tags: [maxtouch, digitizer, gestures, firmware-state-machine, via-config]

# Dependency graph
requires:
  - phase: 02-via-config-integration
    provides: VIA config infrastructure, EEPROM management, scroll divisors
provides:
  - Complete gesture event flow documentation from hardware to firmware
  - Discovery that MaxTouch uses firmware-based gestures (not hardware like IQS5xx)
  - State machine architecture for gesture detection
  - Integration strategy for VIA-configurable gesture filtering
affects: [phase 03-02, phase 03-03, phase 03-04]

# Tech tracking
tech-stack:
  added: []
  patterns: [firmware-gesture-state-machine, VIA-config-filtering]

key-files:
  created: [.planning/phases/03-basic-gesture-enablement/03-01-DISCOVERY.md]
  modified: []

key-decisions:
  - "Firmware-based gesture filtering required (MaxTouch has no hardware gesture registers)"
  - "Use VIA config flags to enable/disable gestures at state machine level"
  - "Default all gestures ENABLED for backward compatibility"

patterns-established:
  - "Pattern 1: Firmware gesture state machine in digitizer_mouse_fallback.c"
  - "Pattern 2: VIA config used to filter firmware behavior, not hardware registers"
  - "Pattern 3: Gesture detection entirely in QMK firmware, no hardware dependencies"

issues-created: []

# Metrics
duration: 15min
completed: 2026-01-14
---

# Phase 03-01: Gesture Event Flow Discovery Summary

**Firmware-based gesture detection via state machine in digitizer_mouse_fallback.c, no hardware gesture registers on MaxTouch MXT336U**

## Performance

- **Duration:** 15 min
- **Started:** 2026-01-14T12:00:00Z
- **Completed:** 2026-01-14T12:15:00Z
- **Tasks:** 4
- **Files modified:** 0 (research only)

## Accomplishments

- **Comprehensive gesture flow analysis** from MaxTouch hardware through digitizer.c to firmware state machine
- **Critical discovery:** MaxTouch MXT336U does NOT have gesture event registers (unlike Azoteq IQS5xx with GESTURE_EVENTS_0/1)
- **Documentation of current gesture implementation** in digitizer_mouse_fallback.c state machine (Tap, Swipe, Scroll, Drag)
- **VIA integration strategy** for firmware-based gesture filtering using enable flags
- **Risk assessment** with mitigation strategies for state machine complexity

## Task Commits

Each task was committed atomically:

1. **Task 1: Analyze MaxTouch driver gesture reporting** - `df1cac5933` (docs)
   - Searched for gesture registers in MaxTouch driver
   - Found NO GESTURE_EVENTS registers (only T100 touch events)
   - Compared with Azoteq IQS5xx driver (has GESTURE_EVENTS_0/1)

2. **Task 2: Trace digitizer gesture event flow** - (covered in same commit)
   - Documented flow: MaxTouch touch events → digitizer.c → digitizer_mouse_fallback.c
   - State machine processes touch events into gestures (Tap, Swipe, Scroll, Drag)

3. **Task 3: Examine existing gesture handling in dilemma.c** - (covered in same commit)
   - dilemma.c applies scroll divisors AFTER gesture detection
   - Does not process gesture events (they don't exist in MaxTouch)
   - Integration point: Apply gesture filtering at state machine level

4. **Task 4: Create DISCOVERY.md with findings** - `df1cac5933` (docs)
   - Complete 388-line DISCOVERY.md with:
     - MaxTouch vs Azoteq IQS5xx comparison
     - Event flow diagrams
     - State machine documentation
     - VIA integration strategy
     - Risk assessment

**Plan metadata:** `df1cac5933` (docs: complete plan)

## Files Created/Modified

- `.planning/phases/03-basic-gesture-enablement/03-01-DISCOVERY.md` - Comprehensive gesture research documentation with hardware comparison, event flow, state machine analysis, VIA integration strategy, and risk assessment

## Decisions Made

**Decision 1: Firmware-based gesture filtering required**
- Rationale: MaxTouch has no hardware gesture registers like Azoteq IQS5xx (GESTURE_EVENTS_0/1)
- Implication: Must filter gestures at firmware state machine level, not hardware
- Implementation: Add VIA config flags to enable/disable gestures in `digitizer_update_mouse_report()`

**Decision 2: Use VIA config flags for gesture enable/disable**
- Rationale: Consistent with existing VIA config pattern (DPI presets, scroll divisors)
- Pattern: Firmware behavior controlled by VIA EEPROM config, not hardware registers
- Default: All gestures ENABLED for backward compatibility

**Decision 3: Intercept state machine for gesture filtering**
- Rationale: Clean integration point, minimal code changes
- Location: `digitizer_mouse_fallback.c::digitizer_update_mouse_report()`
- Approach: Check VIA config flags before entering gesture states (Tap, Swipe, etc.)

## Deviations from Plan

None - plan executed exactly as specified. All research tasks completed, DISCOVERY.md created with comprehensive documentation.

## Issues Encountered

None - research phase proceeded smoothly without blockers.

## Next Phase Readiness

**Ready for Plan 03-02: Add VIA Gesture Flags**
- DISCOVERY.md provides complete hardware context
- VIA config structure documented with 18 bytes reserved
- Clear integration strategy: Add gesture enable flags to VIA config
- State machine entry points identified for filtering

**Ready for Plan 03-03: Implement Gesture Filtering**
- State machine architecture fully documented
- Touch event flow understood
- Filtering strategy established (check flags before state transitions)

**Ready for Plan 03-04: Add Gesture Sensitivity (Optional)**
- Current threshold constants documented
- VIA config extension strategy defined
- Dynamic adjustment approach identified

**No blockers or concerns** - research phase complete, implementation plan clear.

---
*Phase: 03-basic-gesture-enablement*
*Plan: 01*
*Completed: 2026-01-14*
