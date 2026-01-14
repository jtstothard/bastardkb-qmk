# Dilemma v3 VIA Customization Enhancement

## What This Is

A comprehensive VIA customization interface for the Bastard Keyboards Dilemma v3 (3x5_3_procyon) firmware that enables users to configure all trackpad and pointer settings without rebuilding firmware. The goal is "set and forget" firmware where VIA becomes the primary customization interface instead of editing keymap files and recompiling.

## Core Value

**VIA customization interface** - If this fails, nothing else matters. Every feature (gestures, scroll divisors, DPI controls) must be configurable through VIA without requiring firmware rebuilds.

## Requirements

### Validated

(None yet - ship to validate)

### Active

- [ ] VIA-configurable DPI settings (presets and custom values)
- [ ] VIA-configurable drag-scroll speed (X/Y divisors)
- [ ] VIA-configurable two-finger scroll speed (independent from drag-scroll)
- [ ] VIA-configurable auto-sniping toggle (enable/disable on specific layer)
- [ ] VIA-configurable gesture keycodes (map gestures to any action)
- [ ] Enhanced gesture support matching macOS native trackpad:
  - [ ] Two-finger tap (right click)
  - [ ] Two-finger scroll (already working, needs speed control)
  - [ ] Three-finger swipe (app switcher, expose, mission control)
  - [ ] Four-finger swipe (desktop spaces, show desktop)
  - [ ] Pinch-to-zoom (smart zoom)
  - [ ] Tap-to-click with pressure detection
  - [ ] Force click equivalent (long press)
- [ ] Separate scroll divisors for drag-scroll vs two-finger scroll
- [ ] EEPROM persistence for all custom settings
- [ ] All settings changeable without firmware rebuild

### Out of Scope

- [Custom gesture creation] — Users can customize preset gestures via VIA but cannot define entirely new gesture types. This keeps the implementation manageable and focused on delivering excellent implementations of standard gestures rather than a generic gesture creation system.

## Context

**Technical Environment:**
- Target hardware: Bastard Keyboards Dilemma v3 (3x5_3_procyon) with Azoteq IQS5xx trackpad
- Firmware: QMK with BastardKB customizations
- Configuration tool: VIA (already in use)
- Driver: Azoteq IQS5xx supports up to 5-finger tracking with hardware gesture recognition

**Existing Work:**
- PR #79 (maxtouch-dpi-fix branch): Cursor jump fix when DPI changes - uses spike detection
- Current MaxTouch driver uses single `DIGITIZER_SCROLL_DIVISOR` for both drag-scroll and two-finger scroll
- BastardKB dilemma.c has `DILEMMA_DRAGSCROLL_DPI` (100) and `DILEMMA_DRAGSCROLL_BUFFER_SIZE` for drag-scroll mode
- ploody_viamenus repo demonstrates 30+ VIA-configurable settings including DPI presets, drag-scroll divisors, gesture actions

**Known Issues:**
- Scroll speed too fast at 1000 DPI - currently using `DIGITIZER_SCROLL_DIVISOR: 100`
- Two-finger scroll and drag-scroll share same divisor - need independent control
- No VIA integration for DPI, scroll speed, or gesture configuration
- Users must rebuild firmware to change any pointer/trackpad settings

**Research Findings:**
- Azoteq IQS5xx driver supports these gestures (may need enabling):
  - `AZOTEQ_IQS5XX_TAP_ENABLE` (single finger tap)
  - `AZOTEQ_IQS5XX_TWO_FINGER_TAP_ENABLE` (right click)
  - `AZOTEQ_IQS5XX_SCROLL_ENABLE` (two finger scroll)
  - `AZOTEQ_IQS5XX_PRESS_AND_HOLD_ENABLE` (text selection)
  - `AZOTEQ_IQS5XX_SWIPE_X_ENABLE` / `SWIPE_Y_ENABLE` (swipe gestures)
  - `AZOTEQ_IQS5XX_ZOOM_ENABLE` (pinch-to-zoom)
- MaxTouch digitizer can track up to 5 fingers simultaneously
- ploody uses `VIA_EEPROM_CUSTOM_CONFIG_SIZE 32` with struct for 30+ settings
- VIA custom command channel: `via_custom_value_command_kb(uint8_t *data, uint8_t *length)`

## Constraints

- **Must work with existing VIA** — Cannot require users to switch from VIA to another configuration tool. All customization must integrate into VIA's existing interface.
- **QMK/BastardKB upstream compatibility** — Changes should be upstreamable to QMK core or BastardKB fork, not fork-specific hacks that can't be merged
- **Hardware limits** — Azoteq IQS5xx tracks up to 5 fingers, gesture recognition limited to hardware capabilities
- **EEPROM size** — VIA custom config space limited (typically 32 bytes for custom menu data)
- **No breaking changes** — Existing keymaps and configurations must continue working

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| VIA as primary interface | Users already know VIA, no alternative tools accepted | — Pending |
| Separate scroll divisors | Two-finger and drag-scroll have different use cases and DPI scaling | — Pending |
| Preset gestures over custom creation | Manageable implementation, excellent UX for standard gestures | — Pending |
| EEPROM persistence | Settings must survive reboots without firmware rebuild | — Pending |

---
*Last updated: 2025-01-14 after initialization*
