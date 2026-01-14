# Roadmap: Dilemma v3 VIA Customization Enhancement

## Overview

Transform the Dilemma v3 firmware into a fully VIA-configurable platform where users can customize all trackpad and pointer settings without rebuilding firmware. The journey starts with VIA/EEPROM infrastructure, progresses through individual feature implementation (DPI, scroll speeds, gestures), and culminates in a production-ready firmware that matches macOS trackpad capabilities.

## Domain Expertise

None (QMK/firmware development - no specialized expertise available)

## Phases

**Phase Numbering:**
- Integer phases (1, 2, 3): Planned milestone work
- Decimal phases (2.1, 2.2): Urgent insertions (marked with INSERTED)

Decimal phases appear between their surrounding integers in numeric order.

- [ ] **Phase 1: VIA Integration Foundation** - Setup VIA custom command infrastructure and EEPROM layout
- [ ] **Phase 2: EEPROM Configuration System** - Data structures, persistence layer, and settings management
- [ ] **Phase 3: DPI Configuration** - VIA-configurable DPI presets and custom values
- [ ] **Phase 4: Scroll Speed Controls** - Separate divisors for drag-scroll vs two-finger scroll
- [ ] **Phase 5: Auto-Sniping Toggle** - VIA-configurable layer-based sniping enable/disable
- [ ] **Phase 6: Basic Gesture Enablement** - Enable existing Azoteq gestures (tap, two-finger tap, scroll)
- [ ] **Phase 7: Advanced Gesture Support** - Three-finger swipes, four-finger swipes, pinch-to-zoom
- [ ] **Phase 8: Gesture-to-Keycode Mapping** - VIA-configurable gesture actions mapping
- [ ] **Phase 9: Smart Gesture Features** - Tap-to-click with pressure, force click, smart zoom
- [ ] **Phase 10: VIA Menu UI Design** - Create VIA menus for all 30+ settings
- [ ] **Phase 11: Testing & Validation** - Hardware testing on all features
- [ ] **Phase 12: Documentation** - User guide for VIA customization

## Phase Details

### Phase 1: VIA Integration Foundation
**Goal**: Establish VIA custom command infrastructure and define EEPROM layout for all custom settings
**Depends on**: Nothing (first phase)
**Research**: Likely (VIA custom command protocol, EEPROM size limits)
**Research topics**: Current VIA custom command implementation in ploody repo, QMC EEPROM API, VIA custom menu size limits (32-byte constraint)
**Plans**: 3 plans

Plans:
- [ ] 01-01: Research VIA custom command protocol and EEPROM patterns from ploody_viamenus
- [ ] 01-02: Design EEPROM struct for 30+ settings within 32-byte limit
- [ ] 01-03: Implement via_custom_value_command_kb() handler with read/write/validate

### Phase 2: EEPROM Configuration System
**Goal**: Build persistence layer for reading/writing settings from EEPROM with defaults and validation
**Depends on**: Phase 1
**Research**: Unlikely (standard EEPROM patterns in QMK)
**Plans**: 4 plans

Plans:
- [ ] 02-01: Implement settings struct with default values
- [ ] 02-02: Create EEPROM read/write functions with validation
- [ ] 02-03: Implement settings reset to defaults command
- [ ] 02-04: Add EEPROM versioning for future compatibility

### Phase 3: DPI Configuration
**Goal**: VIA-configurable DPI presets (200/400/600/800/1000) and custom DPI values
**Depends on**: Phase 2
**Research**: Unlikely (DPI API already exists in codebase)
**Plans**: 3 plans

Plans:
- [ ] 03-01: Add DPI preset enum and custom DPI fields to EEPROM struct
- [ ] 03-02: Implement VIA commands for DPI preset selection
- [ ] 03-03: Implement custom DPI value input via VIA

### Phase 4: Scroll Speed Controls
**Goal**: Separate scroll divisors for drag-scroll and two-finger scroll, configurable via VIA
**Depends on**: Phase 2
**Research**: Likely (need to modify MaxTouch driver to detect scroll mode)
**Research topics**: MaxTouch driver gesture state machine, how to distinguish drag-scroll from two-finger scroll in driver code
**Plans**: 4 plans

Plans:
- [ ] 04-01: Research driver gesture detection to separate scroll modes
- [ ] 04-02: Add separate divisor fields to EEPROM (drag_scroll_divisor, two_finger_divisor)
- [ ] 04-03: Modify driver to apply correct divisor based on scroll mode
- [ ] 04-04: Implement VIA commands for X/Y divisor adjustment

### Phase 5: Auto-Sniping Toggle
**Goal**: VIA-configurable auto-sniping enable/disable on specific layers
**Depends on**: Phase 2
**Research**: Unlikely (auto-sniping pattern exists in keymap)
**Plans**: 3 plans

Plans:
- [ ] 05-01: Add auto_snipe_enabled and auto_snipe_layer to EEPROM struct
- [ ] 05-02: Modify layer_state_set_user() to check EEPROM setting
- [ ] 05-03: Implement VIA command to toggle auto-sniping

### Phase 6: Basic Gesture Enablement
**Goal**: Enable existing Azoteq hardware gestures (single tap, two-finger tap, scroll, press-and-hold)
**Depends on**: Phase 1
**Research**: Likely (Azoteq gesture enable flags and configuration)
**Research topics**: Azoteq IQS5xx datasheet gesture enable flags, current gesture configuration in driver, gesture event handling in QMK
**Plans**: 4 plans

Plans:
- [ ] 06-01: Research Azoteq gesture enable flags and configuration registers
- [ ] 06-02: Enable single-tap, two-finger-tap, scroll, press-and-hold in driver
- [ ] 06-03: Add gesture enable toggles to EEPROM struct
- [ ] 06-04: Implement VIA commands for gesture enable/disable

### Phase 7: Advanced Gesture Support
**Goal**: Three-finger swipes (app switcher, expose), four-finger swipes (spaces, show desktop), pinch-to-zoom
**Depends on**: Phase 6
**Research**: Likely (multi-finger gesture detection and mapping)
**Research topics**: Azoteq swipe gesture X/Y detection, 3/4-finger tracking, gesture classification logic, macOS gesture equivalents
**Plans**: 5 plans

Plans:
- [ ] 07-01: Research Azoteq 3/4-finger swipe and zoom gesture detection
- [ ] 07-02: Implement swipe gesture state machine (3-finger X/Y, 4-finger X/Y)
- [ ] 07-03: Implement pinch-to-zoom gesture detection
- [ ] 07-04: Add advanced gesture enables and mappings to EEPROM
- [ ] 07-05: Implement VIA commands for advanced gesture configuration

### Phase 8: Gesture-to-Keycode Mapping
**Goal**: VIA-configurable mapping of gestures to keycodes (e.g., two-finger tap → KC_BTN2, swipe → KC_TAB)
**Depends on**: Phase 7
**Research**: Unlikely (keycode mapping is standard QMK pattern)
**Plans**: 3 plans

Plans:
- [ ] 08-01: Add keycode fields to EEPROM for each gesture type
- [ ] 08-02: Implement gesture keycode lookup and execution
- [ ] 08-03: Implement VIA command for keycode selection per gesture

### Phase 9: Smart Gesture Features
**Goal**: Tap-to-click with pressure detection, force click (long press), smart zoom gesture
**Depends on**: Phase 7
**Research**: Likely (pressure sensing and gesture timing)
**Research topics**: Azoteq pressure/force measurements, long-press detection timing, smart zoom gesture recognition
**Plans**: 4 plans

Plans:
- [ ] 09-01: Research Azoteq pressure sensing and tap force detection
- [ ] 09-02: Implement tap-to-click with pressure threshold
- [ ] 09-03: Implement force click (long press) detection
- [ ] 09-04: Implement smart zoom gesture and add settings to EEPROM

### Phase 10: VIA Menu UI Design
**Goal**: Create VIA menus for all 30+ settings organized by category (DPI, scroll, gestures, advanced)
**Depends on**: Phases 3-9 (all feature implementations)
**Research**: Likely (VIA menu JSON format and organization)
**Research topics**: VIA menu JSON schema, menu organization patterns from ploody repo, nested menu structures
**Plans**: 3 plans

Plans:
- [ ] 10-01: Research VIA menu JSON format and ploody menu organization
- [ ] 10-02: Design menu hierarchy (Pointer, Scroll, Gestures, Advanced)
- [ ] 10-03: Implement VIA menu JSON with all 30+ settings

### Phase 11: Testing & Validation
**Goal**: Comprehensive hardware testing of all features on actual Dilemma v3 hardware
**Depends on**: Phases 1-10
**Research**: Unlikely (standard testing procedures)
**Plans**: 5 plans

Plans:
- [ ] 11-01: Test DPI switching and custom values
- [ ] 11-02: Test separate scroll divisors in both modes
- [ ] 11-03: Test all gesture types (tap, multi-finger, swipe, zoom)
- [ ] 11-04: Test EEPROM persistence across reboots
- [ ] 11-05: Test VIA menu navigation and setting changes

### Phase 12: Documentation
**Goal**: User guide for VIA customization covering all features and troubleshooting
**Depends on**: Phase 11
**Research**: Unlikely (documentation writing)
**Plans**: 3 plans

Plans:
- [ ] 12-01: Write VIA menu navigation guide
- [ ] 12-02: Document all configurable settings with recommendations
- [ ] 12-03: Create troubleshooting guide for common issues

## Progress

**Execution Order:**
Phases execute in numeric order: 1 → 2 → 3 → 4 → 5 → 6 → 7 → 8 → 9 → 10 → 11 → 12

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. VIA Integration Foundation | 0/3 | Not started | - |
| 2. EEPROM Configuration System | 0/4 | Not started | - |
| 3. DPI Configuration | 0/3 | Not started | - |
| 4. Scroll Speed Controls | 0/4 | Not started | - |
| 5. Auto-Sniping Toggle | 0/3 | Not started | - |
| 6. Basic Gesture Enablement | 0/4 | Not started | - |
| 7. Advanced Gesture Support | 0/5 | Not started | - |
| 8. Gesture-to-Keycode Mapping | 0/3 | Not started | - |
| 9. Smart Gesture Features | 0/4 | Not started | - |
| 10. VIA Menu UI Design | 0/3 | Not started | - |
| 11. Testing & Validation | 0/5 | Not started | - |
| 12. Documentation | 0/3 | Not started | - |
