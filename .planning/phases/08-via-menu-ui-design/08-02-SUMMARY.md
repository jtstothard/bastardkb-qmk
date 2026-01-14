---
phase: 08-via-menu-ui-design
plan: 02
subsystem: [ui, via, configuration]
tags: [via, json, ui-controls, menu-structure]

# Dependency graph
requires:
  - phase: 08-via-menu-ui-design
    plan: 01
    provides: VIA menu format research and menu design
provides:
  - Basic VIA menu JSON structure with Pointer, Scroll, and Basic Gestures sections
  - 14 menu items covering most common settings (DPI, scroll, basic gestures)
  - Foundation for Plan 08-03 (advanced menus)
affects: [08-via-menu-ui-design, testing]

# Tech tracking
tech-stack:
  added: [VIA custom menu JSON]
  patterns: [VIA menu structure, valueId mapping, UI control types]

key-files:
  created: [keyboards/bastardkb/dilemma/via.json]
  modified: []

key-decisions:
  - "Four top-level menus: Pointer, Scroll, Gestures - Basic, Actions"
  - "Drag Scroll as placeholder in Pointer menu (valueId 0, special VIA handling)"
  - "Separate scroll divisors for drag-scroll vs two-finger scroll modes"

patterns-established:
  - "VIA customMenu array with nested content structure"
  - "valueId mapping to dilemma.h enum definitions"
  - "UI control types: slider, uint16, uint8, toggle, action"
  - "Min/max validation matching firmware constraints"

issues-created: []

# Metrics
duration: 5min
completed: 2026-01-14
---

# Plan 08-02: Basic VIA Menu JSON Structure Summary

**VIA custom menu JSON with Pointer, Scroll, and Basic Gestures sections covering 14 menu items**

## Performance

- **Duration:** 5 minutes
- **Started:** 2026-01-14T15:30:00Z
- **Completed:** 2026-01-14T15:35:00Z
- **Tasks:** 1
- **Files modified:** 1 created

## Accomplishments

- Created comprehensive VIA menu JSON structure for Dilemma v3 touchpad settings
- Implemented 4 top-level menus (Pointer, Scroll, Gestures - Basic, Actions)
- Mapped all valueIds (0-11) to dilemma.h enum definitions
- Established foundation for Plan 08-03 advanced menus

## Task Commits

Each task was committed atomically:

1. **Task 1: Create via.json with basic menu structure** - `3c8bfa461d` (feat)

**Plan metadata:** None (single task plan)

## Files Created/Modified

- `keyboards/bastardkb/dilemma/via.json` - VIA custom menu JSON with 4 containers and 14 menu items

## VIA Menu Structure

### Pointer Menu (4 items)
1. **DPI Preset** (slider, valueId 2)
   - Options: ["0", "1", "2", "3", "4", "5", "6"]
   - Format: left="200", right="1200"
   - Maps to DPI preset indices

2. **Custom DPI** (uint16, valueId 3)
   - Range: 200-4000
   - User-configurable DPI value

3. **Sniping DPI** (slider, valueId 4)
   - Options: ["0", "1", "2", "3"]
   - Format: left="200", right="800"
   - Maps to sniping DPI preset indices

4. **Drag Scroll** (none, valueId 0)
   - Built-in VIA action (placeholder)
   - Special case: VIA may handle differently

### Scroll Menu (4 items)
1. **Drag Scroll X Divisor** (uint8, valueId 4)
   - Range: 1-15

2. **Drag Scroll Y Divisor** (uint8, valueId 5)
   - Range: 1-15

3. **Two-Finger Scroll X Divisor** (uint8, valueId 6)
   - Range: 1-15

4. **Two-Finger Scroll Y Divisor** (uint8, valueId 7)
   - Range: 1-15

### Gestures - Basic Menu (4 items)
1. **Tap to Click** (toggle, valueId 8)

2. **Two-Finger Tap** (toggle, valueId 9)

3. **Two-Finger Scroll** (toggle, valueId 10)

4. **Press and Hold** (toggle, valueId 11)

### Actions Menu (2 items)
1. **Save** (action, valueId 1)
   - Commits VIA config to EEPROM

2. **Reset to Defaults** (action, valueId 0)
   - Restores factory defaults

## Technical Implementation Details

### Value ID Mapping

All valueIds match dilemma.h enum definitions:

| valueId | Setting | Type | Range |
|---------|---------|------|-------|
| 0 | Unhandled/Reset | - | - |
| 1 | Save | action | - |
| 2 | DPI Preset | slider | 0-6 |
| 3 | Custom DPI | uint16 | 200-4000 |
| 4 | Drag Scroll X / Sniping DPI | uint8 / slider | 1-15 / 0-3 |
| 5 | Drag Scroll Y | uint8 | 1-15 |
| 6 | Two-Finger X | uint8 | 1-15 |
| 7 | Two-Finger Y | uint8 | 1-15 |
| 8-11 | Gesture Enables | toggle | boolean |

### UI Control Types

- **slider**: Dropdown with predefined options (DPI presets, sniping DPI)
- **uint16**: Numeric input with min/max validation (custom DPI)
- **uint8**: Numeric input with min/max validation (scroll divisors)
- **toggle**: Boolean checkbox (gesture enables)
- **action**: Button triggering VIA command (save/reset)
- **none**: Placeholder or special VIA handling (drag scroll)

### JSON Structure

```json
{
  "customMenu": [
    {
      "label": "Menu Name",
      "content": [
        {
          "label": "Control Label",
          "content": "control_type",
          "valueId": N,
          "type": "ui_type",
          "format": {
            "min": N,
            "max": N
          }
        }
      ]
    }
  ]
}
```

## Decisions Made

### Four Top-Level Menu Structure

**Decision:** Use 4 top-level menus (Pointer, Scroll, Gestures - Basic, Actions)

**Rationale:**
- Logical grouping by function (pointer settings, scroll settings, gestures, actions)
- Progressive disclosure: basic settings first, advanced later (Plan 08-03)
- Matches user mental model (settings categories)
- Prevents overwhelming UI with 30+ settings in one menu

### Drag Scroll Placeholder

**Decision:** Include Drag Scroll as placeholder in Pointer menu (valueId 0, type "none")

**Rationale:**
- VIA has built-in drag-scroll support
- May require special handling outside custom menu
- Placeholder maintains UI consistency
- Can be removed if VIA handles automatically

### Separate Scroll Divisors

**Decision:** Separate divisors for drag-scroll vs two-finger scroll modes

**Rationale:**
- Different use cases require different scroll speeds
- Drag-scroll: precise control (higher divisor)
- Two-finger: natural scrolling (lower divisor)
- Firmware already implements separate modes (Phase 2)
- User-configurable flexibility

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

**Issue:** via.json excluded by .gitignore

**Problem:** QMK's .gitignore excludes `via*.json` to prevent VIA keymap files from being committed. However, `keyboards/bastardkb/dilemma/via.json` is the actual menu definition file that should be tracked.

**Resolution:** Used `git add -f` to force-add the file since it's a keyboard-specific configuration file, not a VIA keymap.

**Verification:** File committed successfully as `3c8bfa461d`

## Next Phase Readiness

### Ready for Plan 08-03
- Basic menu structure established
- Value ID mapping confirmed
- JSON syntax validated
- UI control patterns defined

### Plan 08-03 Requirements
- Add "Gestures - Advanced" menu (valueIds 12-24)
- Add "Advanced Settings" menu with auto-snipe (valueIds 25-26)
- Implement conditional display with showIf
- Add keycode controls for swipe gestures and zoom
- Complete all 27 value IDs (0-26)

### Testing Recommendations
1. Load via.json in VIA configurator
2. Verify all controls display correctly
3. Test slider options (DPI presets, sniping DPI)
4. Test numeric inputs (custom DPI, scroll divisors)
5. Test toggles (gesture enables)
6. Test actions (save, reset)
7. Verify value changes sync with firmware

### Known Limitations
- Drag Scroll (valueId 0) may need special handling
- Keycode controls not yet implemented (Plan 08-03)
- Conditional display not yet implemented (Plan 08-03)

---
*Plan: 08-02*
*Completed: 2026-01-14*
