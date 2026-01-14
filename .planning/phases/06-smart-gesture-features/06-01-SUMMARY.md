# Plan 06-01 Summary: Smart Gesture Features Discovery

**Phase:** 06 - Smart Gesture Features
**Plan:** 01 - Research pressure sensing and design force click strategy
**Status:** ✅ COMPLETE
**Duration:** 10 minutes (1 task, 1 atomic commit)

---

## Executive Summary

Researched MaxTouch MXT336U pressure/force sensing capabilities and determined that hardware-based pressure detection is **NOT suitable** for reliable force click detection. Designed firmware-based timing approach using long press detection (500ms threshold). Documented smart zoom as already implemented in Phase 4.

---

## Completed Tasks

### Task 1: ✅ Research MaxTouch Pressure/Force Sensing Capabilities

**Findings:**
- **T65 Lens Bending Object:** Has force sensing registers (forcescale, forcethr, forcedi, forcehyst) but NOT enabled by default. Designed for screen protection (detects excessive force that could damage display), not gesture input. Requires per-unit calibration.
- **T100 Amplitude Reporting:** Enabled (tchaux = 0x2), but reports signal strength, NOT pressure. Highly variable based on finger size, moisture, skin condition, environment.
- **No Dedicated Pressure Registers:** MaxTouch lacks true pressure sensing like iPhone 3D Touch or MacBook Force Touch trackpad.

**Conclusion:** ❌ Hardware-based pressure detection unsuitable for tap detection. Use timing-based approach instead.

**Files Created:**
- `.planning/phases/06-smart-gesture-features/06-01-DISCOVERY.md` (420 lines)

### Task 2: ✅ Design Firmware-Based Force Click Detection Strategy

**Approach:** Timing-based long press detection (not pressure-based)

**Algorithm:**
```c
// Track finger down duration
uint32_t duration = timer_read32() - finger_down_time;

if (duration > 500ms) {
    // Force click (long press) detected
    register_code(press_and_hold_keycode);
} else if (duration < 200ms) {
    // Normal tap detected
    tap_detected = true;
}
```

**Thresholds:**
- Force click: 500ms (matches macOS long press)
- Normal tap: 200ms (existing DIGITIZER_MOUSE_TAP_DETECTION_TIMEOUT)
- Hysteresis: 300ms gap (prevents ambiguous gestures)

**Implementation:**
- Extend `quantum/digitizer_mouse_fallback.c` state machine
- Add `force_click_detected` flag
- Track duration using `timer_read32()`
- Use existing `press_and_hold_keycode` from VIA config (Bytes 16-17, Phase 3)
- **No EEPROM changes required** (already implemented in Phase 3)

**Edge Cases:**
- Disable during drag-scroll or pointer sniping modes
- Only detect with 1 finger (ignore 2+ fingers)
- Cancel if movement > tap distance (25px)

**Advantages:**
- ✅ No special hardware required
- ✅ Uses existing state machine timing infrastructure
- ✅ Backward compatible with existing tap detection
- ✅ VIA-configurable (already implemented in Phase 3)
- ✅ Consistent with OS expectations (500ms = macOS long press)

**Disadvantages:**
- ❌ Not true pressure detection (time-based proxy)
- ❌ May interfere with drag operations (careful state management required)

### Task 3: ✅ Document Smart Zoom as Already Implemented

**Status:** ✅ COMPLETE - No additional work needed

**Already Implemented in Phase 4 (Plan 04-05):**
- Pinch-to-zoom gesture detection functional
- Two-finger spread/pinch detection complete
- Integer-only distance calculation (squared comparison, no sqrt)
- Zoom in/out keycodes configurable via VIA
- `zoom_in_keycode` (Bytes 34-35)
- `zoom_out_keycode` (Bytes 36-37)
- `pinch_to_zoom_enabled` flag (Byte 7, bit 0)
- Firmware state machine includes `Zoom` state
- Timeout handling prevents stuck zoom state (300ms)

**Phase 6 Plan Status:**
- **06-01:** Research ✅ (this plan)
- **06-02:** Implement force click detection ⏳ (next)
- **06-03:** Update documentation ⏳
- **06-04:** ~~Smart zoom gesture~~ → CANCELED (already done in Phase 4)

---

## Key Achievements

### Technical Discoveries

1. **MaxTouch Pressure Sensing Limitations**
   - T65 Lens Bending: Not enabled by default, designed for damage prevention
   - T100 Amplitude: Signal strength, not pressure, highly variable
   - No reliable hardware-based pressure detection for gesture input
   - Recommendation: Use timing-based approach (firmware-only solution)

2. **Force Click Detection Strategy**
   - 500ms threshold matches macOS long press behavior
   - Uses existing Phase 3 VIA config (press_and_hold_keycode)
   - No EEPROM changes required
   - Clean separation: 200ms (tap) vs. 500ms (force click)
   - State machine extension without adding new states

3. **Smart Zoom Completion**
   - Already implemented in Phase 4 (Plan 04-05)
   - Functional pinch-to-zoom gesture detection
   - VIA-configurable zoom keycodes
   - Integer-only math (no floating-point sqrt)

### Documentation

- **DISCOVERY.md:** 420 lines documenting pressure sensing research, force click strategy, smart zoom status
- **Code references:** T65 object structure, T100 amplitude config, state machine analysis
- **Implementation roadmap:** Clear path forward for Plan 06-02

---

## Files Modified

```
.planning/phases/06-smart-gesture-features/06-01-DISCOVERY.md  (created, +420 lines)
.planning/phases/06-smart-gesture-features/06-01-SUMMARY.md   (created, this file)
```

---

## Next Steps

### Plan 06-02: Implement Force Click Detection

**Estimated Duration:** 15 minutes (4-5 atomic commits)

**Tasks:**
1. Add force click timing variables to state tracking
2. Extend `Down` state to check duration for force click (500ms)
3. Handle force click keycode (use existing press_and_hold_keycode)
4. Add conditional logic to disable during drag/pointer modes
5. Test and refine thresholds

**File:** `quantum/digitizer_mouse_fallback.c`

**No EEPROM Changes Required:**
- `press_and_hold_keycode` already exists (Bytes 16-17, Phase 3)
- `press_and_hold_enabled` flag already exists (Phase 3)

### Plan 06-03: Update Documentation

**Estimated Duration:** 5 minutes (1 atomic commit)

**Tasks:**
1. Mark Plan 06-04 as complete (reference Phase 4, Plan 04-05)
2. Update ROADMAP.md to reflect smart zoom completion
3. Document Phase 6 timeline adjustment

---

## Risk Assessment

### Technical Risks

| Risk | Impact | Mitigation | Status |
|------|--------|-----------|--------|
| Force click state machine bugs | Medium | Extend existing Down state, don't add new state | ⚠️ Monitor in 06-02 |
| Conflict with drag operations | Low | Disable force click during drag/pointer modes | ✅ Mitigated |
| User expectations (true pressure) | Low | Clear documentation: time-based long press | ✅ Documented |

### Integration Risks

| Risk | Impact | Mitigation | Status |
|------|--------|-----------|--------|
| Backward compatibility | Low | Add separate detection path, don't modify tap logic | ✅ Safe |
| VIA config space | None | Use existing press_and_hold_keycode from Phase 3 | ✅ No changes needed |
| Performance overhead | None | Timer reads are fast (~10 CPU cycles) | ✅ Negligible |

---

## Timeline

**Plan 06-01:** ✅ COMPLETE (10 minutes)
- Task 1: Pressure sensing research ✅
- Task 2: Force click strategy design ✅
- Task 3: Smart zoom documentation ✅

**Plan 06-02:** ⏳ NEXT (estimated 15 minutes)
- Implement timing-based force click detection
- Extend state machine in digitizer_mouse_fallback.c
- Test with VIA config integration

**Plan 06-03:** ⏳ PENDING (estimated 5 minutes)
- Update ROADMAP.md
- Mark Plan 06-04 as complete (smart zoom done in Phase 4)

---

## Summary

**Plan 06-01 successfully completed research phase for smart gesture features.**

**Key Outcomes:**
1. ✅ Confirmed MaxTouch lacks reliable pressure sensing for gestures
2. ✅ Designed firmware-based force click using 500ms long press detection
3. ✅ Identified smart zoom as already implemented (Phase 4)
4. ✅ Established clear path forward for Plan 06-02 implementation

**No blockers identified.** Ready to proceed with Plan 06-02 (force click implementation).

**Commits:**
- `f4d41335ab`: feat(06-01): research MaxTouch pressure/force sensing capabilities

**Total Duration:** 10 minutes (1 task, 1 atomic commit)

**Next:** Plan 06-02 - Implement force click detection
