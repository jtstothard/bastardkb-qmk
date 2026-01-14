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

- [x] **Phase 1: VIA Integration Foundation** - Setup VIA custom command infrastructure and EEPROM layout
- [x] **Phase 2: VIA Configuration Integration** - Bridge VIA config to device behavior, DPI and scroll divisors
- [x] **Phase 3: Basic Gesture Enablement** - Enable existing Azoteq gestures (tap, two-finger tap, scroll)
- [x] **Phase 4: Advanced Gesture Support** - Three-finger swipes, four-finger swipes, pinch-to-zoom
- [ ] **Phase 5: Gesture-to-Keycode Mapping** - VIA-configurable gesture actions mapping
- [ ] **Phase 6: Smart Gesture Features** - Tap-to-click with pressure, force click, smart zoom
- [ ] **Phase 7: Auto-Sniping Toggle** - VIA-configurable layer-based sniping enable/disable
- [ ] **Phase 8: VIA Menu UI Design** - Create VIA menus for all 30+ settings
- [ ] **Phase 9: Testing & Validation** - Hardware testing on all features
- [ ] **Phase 10: Documentation** - User guide for VIA customization

## Phase Details

### Phase 1: VIA Integration Foundation
**Goal**: Establish VIA custom command infrastructure and define EEPROM layout for all custom settings
**Depends on**: Nothing (first phase)
**Research**: Likely (VIA custom command protocol, EEPROM size limits)
**Research topics**: Current VIA custom command implementation in ploody repo, QMC EEPROM API, VIA custom menu size limits (32-byte constraint)
**Plans**: 3 plans

Plans:
- [x] 01-01: Research VIA custom command protocol and EEPROM patterns from ploody_viamenus
- [x] 01-02: Design EEPROM struct for 30+ settings within 32-byte limit
- [x] 01-03: Implement via_custom_value_command_kb() handler with read/write/validate

### Phase 2: VIA Configuration Integration
**Goal**: Bridge VIA config storage to device behavior, implement DPI and scroll divisor configuration
**Depends on**: Phase 1
**Research**: Unlikely (DPI API and scroll divisors already exist in codebase)
**Plans**: 3 plans

Plans:
- [x] 02-01: Create VIA config helper functions (get_dpi_from_preset, apply_via_dilemma_config)
- [x] 02-02: Implement scroll divisor configuration with automatic updates
- [x] 02-03: Integrate VIA config application on save and mode changes

### Phase 3: Basic Gesture Enablement
**Goal**: Enable existing Azoteq hardware gestures (single tap, two-finger tap, scroll, press-and-hold)
**Depends on**: Phase 1
**Research**: Likely (Azoteq gesture enable flags and configuration)
**Research topics**: Azoteq IQS5xx datasheet gesture enable flags, current gesture configuration in driver, gesture event handling in QMK
**Plans**: 4 plans

Plans:
- [x] 03-01: Research Azoteq gesture enable flags and configuration registers
- [x] 03-02: Enable single-tap, two-finger-tap, scroll, press-and-hold in driver
- [x] 03-03: Add gesture enable toggles to EEPROM struct
- [x] 03-04: Implement VIA commands for gesture enable/disable

### Phase 4: Advanced Gesture Support
**Goal**: Three-finger swipes (app switcher, expose), four-finger swipes (spaces, show desktop), pinch-to-zoom
**Depends on**: Phase 3
**Research**: Likely (multi-finger gesture detection and mapping)
**Research topics**: Azoteq swipe gesture X/Y detection, 3/4-finger tracking, gesture classification logic, macOS gesture equivalents
**Plans**: 5 plans

Plans:
- [x] 04-01: Research Azoteq 3/4-finger swipe and zoom gesture detection
- [x] 04-02: Implement swipe gesture state machine (3-finger X/Y, 4-finger X/Y)
- [x] 04-03: Implement pinch-to-zoom gesture detection
- [x] 04-04: Add advanced gesture enables and mappings to EEPROM
- [x] 04-05: Implement VIA commands for advanced gesture configuration

### Phase 5: Gesture-to-Keycode Mapping
**Goal**: VIA-configurable mapping of gestures to keycodes (e.g., two-finger tap → KC_BTN2, swipe → KC_TAB)
**Depends on**: Phase 4
**Research**: Unlikely (keycode mapping is standard QMK pattern)
**Plans**: 3 plans

Plans:
- [ ] 05-01: Add keycode fields to EEPROM for each gesture type
- [ ] 05-02: Implement gesture keycode lookup and execution
- [ ] 05-03: Implement VIA command for keycode selection per gesture

### Phase 6: Smart Gesture Features
**Goal**: Tap-to-click with pressure detection, force click (long press), smart zoom gesture
**Depends on**: Phase 4
**Research**: Likely (pressure sensing and gesture timing)
**Research topics**: Azoteq pressure/force measurements, long-press detection timing, smart zoom gesture recognition
**Plans**: 4 plans

Plans:
- [ ] 06-01: Research Azoteq pressure sensing and tap force detection
- [ ] 06-02: Implement tap-to-click with pressure threshold
- [ ] 06-03: Implement force click (long press) detection
- [ ] 06-04: Implement smart zoom gesture and add settings to EEPROM

### Phase 7: Auto-Sniping Toggle
**Goal**: VIA-configurable auto-sniping enable/disable on specific layers
**Depends on**: Phase 2
**Research**: Unlikely (auto-sniping pattern exists in keymap)
**Plans**: 3 plans

Plans:
- [ ] 07-01: Add auto_snipe_enabled and auto_snipe_layer to EEPROM struct
- [ ] 07-02: Modify layer_state_set_user() to check EEPROM setting
- [ ] 07-03: Implement VIA command to toggle auto-sniping

### Phase 8: VIA Menu UI Design
**Goal**: Create VIA menus for all 30+ settings organized by category (DPI, scroll, gestures, advanced)
**Depends on**: Phases 2-7 (all feature implementations)
**Research**: Likely (VIA menu JSON format and organization)
**Research topics**: VIA menu JSON schema, menu organization patterns from ploody repo, nested menu structures
**Plans**: 3 plans

Plans:
- [ ] 08-01: Research VIA menu JSON format and ploody menu organization
- [ ] 08-02: Design menu hierarchy (Pointer, Scroll, Gestures, Advanced)
- [ ] 08-03: Implement VIA menu JSON with all 30+ settings

### Phase 9: Testing & Validation
**Goal**: Comprehensive hardware testing of all features on actual Dilemma v3 hardware
**Depends on**: Phases 1-8
**Research**: Unlikely (standard testing procedures)
**Plans**: 5 plans

Plans:
- [ ] 09-01: Test DPI switching and custom values
- [ ] 09-02: Test separate scroll divisors in both modes
- [ ] 09-03: Test all gesture types (tap, multi-finger, swipe, zoom)
- [ ] 09-04: Test EEPROM persistence across reboots
- [ ] 09-05: Test VIA menu navigation and setting changes

### Phase 10: Documentation
**Goal**: User guide for VIA customization covering all features and troubleshooting
**Depends on**: Phase 9
**Research**: Unlikely (documentation writing)
**Plans**: 3 plans

Plans:
- [ ] 10-01: Write VIA menu navigation guide
- [ ] 10-02: Document all configurable settings with recommendations
- [ ] 10-03: Create troubleshooting guide for common issues

## Progress

**Execution Order:**
Phases execute in numeric order: 1 → 2 → 3 → 4 → 5 → 6 → 7 → 8 → 9 → 10

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. VIA Integration Foundation | 3/3 | Complete | 2026-01-14 |
| 2. VIA Configuration Integration | 3/3 | Complete | 2026-01-14 |
| 3. Basic Gesture Enablement | 4/4 | Complete | 2026-01-14 |
| 4. Advanced Gesture Support | 5/5 | Complete | 2026-01-14 |
| 5. Gesture-to-Keycode Mapping | 0/3 | Not started | - |
| 6. Smart Gesture Features | 0/4 | Not started | - |
| 7. Auto-Sniping Toggle | 0/3 | Not started | - |
| 8. VIA Menu UI Design | 0/3 | Not started | - |
| 9. Testing & Validation | 0/5 | Not started | - |
| 10. Documentation | 0/3 | Not started | - |
