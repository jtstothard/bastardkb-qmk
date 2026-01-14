# VIA Menu UI Design Discovery

## Research Date: 2026-01-14

## Objective

Research VIA's custom menu JSON format to design a comprehensive menu structure for Dilemma v3's 30+ configurable settings.

---

## Part 1: VIA Custom Menu JSON Format

### Overview

VIA uses JSON-based keyboard definitions stored in the [VIA keyboards repository](https://github.com/the-via/keyboards). Custom menus are defined using the `menus` property in the keyboard definition file.

### Key Resources

- [VIA Custom UI Documentation](https://caniusevia.com/docs/custom_ui/) - Complete reference for custom UI definitions
- [VIA Specification](https://caniusevia.com/docs/specification/) - Keyboard definition format
- [VIA Built-in UI Definitions](https://github.com/the-via/keyboards/tree/master/common-menus) - Reference implementations

### Menu Structure

A VIA menu definition has a **maximum depth of three**:

1. **Top-level menu** - Appears in VIA's top-left menu (e.g., "Lighting", "Display")
2. **Sub-menu** - Appears in VIA's bottom-left menu (e.g., "Underglow", "Indicators")
3. **UI control** - The actual input widget (e.g., slider, toggle, dropdown)

### JSON Structure

```json
{
  "menus": [
    {
      "label": "Top Level Menu",
      "content": [
        {
          "label": "Sub Menu",
          "content": [
            {
              "label": "Control Label",
              "type": "range",
              "options": [0, 255],
              "content": ["id_control_name", channel_id, value_id]
            }
          ]
        }
      ]
    }
  ]
}
```

### UI Control Types

#### 1. **Range** (Slider)

Continuous numeric value control.

```json
{
  "label": "Brightness",
  "type": "range",
  "options": [0, 255],
  "content": ["id_qmk_rgblight_brightness", 2, 1]
}
```

- `options`: [min, max] - numeric range limits
- Data size: 1 byte if max ≤ 255, 2 bytes if max > 255
- **Use for**: DPI values, scroll divisors, sensitivity thresholds

#### 2. **Toggle**

Boolean on/off switch.

```json
{
  "label": "God Mode",
  "type": "toggle",
  "content": ["id_god_mode", 0, 1]
}
```

- Default values: 0 (off) or 1 (on)
- Can use `options` to define custom two-value mapping
- **Use for**: Gesture enables, feature toggles

#### 3. **Dropdown**

Selection menu mapping strings to integers.

```json
{
  "label": "Date Format",
  "type": "dropdown",
  "content": ["id_date_format", 0, 5],
  "options": [
    ["yyyy-mm-dd", 0],
    ["dd/mm/yyyy", 1],
    ["mm/dd/yyyy", 2]
  ]
}
```

- Numbers can be omitted (implicitly assigned starting at 0)
- **Use for**: DPI presets, effect selections

#### 4. **Button**

Clickable button that sends a numeric value.

```json
{
  "label": "Test button",
  "type": "button",
  "options": [10],
  "content": ["id_test_button", 0, 3]
}
```

- Sends value defined in `options`
- **Use for**: Save, Reset, Apply actions

#### 5. **Keycode**

QMK keycode input control (16-bit values).

```json
{
  "label": "Head Slam",
  "type": "keycode",
  "content": ["id_head_slam", 0, 6]
}
```

- Always 2 bytes (16-bit)
- **Use for**: Gesture keycode mappings

#### 6. **Color**

Color picker for hue and saturation (brightness controlled separately).

```json
{
  "label": "Color",
  "type": "color",
  "content": ["id_qmk_rgblight_color", 2, 4]
}
```

- **Use for**: Lighting colors (not needed for Dilemma gestures)

### Advanced Features

#### **showIf** - Conditional Display

Show/hide controls based on other control values:

```json
{
  "showIf": "{id_god_mode} == 1",
  "content": [
    {
      "label": "Audacity",
      "type": "range",
      "options": [0, 255],
      "content": ["id_audacity", 0, 2]
    }
  ]
}
```

- Operators: `==`, `!=`, `<`, `<=`, `>`, `>=`, `||`, `&&`, `!`, `(`, `)`
- Values referenced as `{value_key}`
- **Use for**: Showing keycode controls only when gesture is enabled

#### **Array Values**

Index-based values to avoid multiple enum definitions:

```json
{
  "label": "Color 1",
  "type": "color",
  "content": ["id_buttglow_color[0]", 0, 4, 0]
},
{
  "label": "Color 2",
  "type": "color",
  "content": ["id_buttglow_color[1]", 0, 4, 1]
}
```

- Index appended after `value_id` in `content` array
- `value_key` must be unique (include index in string)
- **Use for**: Multiple similar values (swipe keycodes, zoom keycodes)

### Protocol Communication

#### **Channel ID**

Routes commands to feature-specific handlers.

- Built-in channels start at 1 (lighting, audio, etc.)
- Custom features use `id_custom_channel = 0`
- Allows overlapping `value_id` ranges across features

#### **Value ID**

Identifies specific values within a channel.

- Must be unique per channel
- Enum values start at 1 (0 = `id_unhandled`)
- Maps to firmware enum definitions

#### **Content Array Format**

```json
"content": ["value_key_string", channel_id, value_id, optional_index]
```

- `value_key`: Unique string matching firmware enum name
- `channel_id`: Routing identifier (0 = custom)
- `value_id`: Enum value from firmware
- `optional_index`: Array index for array values

### Firmware Implementation

Custom UI requires three VIA protocol command handlers:

1. **`id_custom_set_value`** - Update config from VIA
2. **`id_custom_get_value`** - Send config to VIA
3. **`id_custom_save`** - Persist config to EEPROM

**Example Handler:**

```c
void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    // data = [command_id, channel_id, value_id, value_data]
    uint8_t *command_id = &(data[0]);
    uint8_t *channel_id = &(data[1]);
    uint8_t *value_id_and_data = &(data[2]);

    if (*channel_id == id_custom_channel) {
        switch (*command_id) {
            case id_custom_set_value:
                dilemma_config_set_value(value_id_and_data);
                break;
            case id_custom_get_value:
                dilemma_config_get_value(value_id_and_data);
                break;
            case id_custom_save:
                dilemma_config_save();
                break;
            default:
                *command_id = id_unhandled;
                break;
        }
        return;
    }
    *command_id = id_unhandled;
}
```

### 16-bit Value Handling

For values > 255 (DPI, keycodes), handle byte order [high, low]:

```c
// Set 16-bit value
uint16_t value = value_data[0] << 8 | value_data[1];

// Get 16-bit value
value_data[0] = value >> 8;      // High byte
value_data[1] = value & 0xFF;    // Low byte
```

### Top-Level Menu Conventions

VIA has standardized top-level menu names with icons:

- **"Lighting"** - All lighting features (underglow, backlight, RGB)
- **"Audio"** - Audio/buzzer features
- **"Display"** - OLED/LCD features
- Generic icon for custom labels

**Best Practice:** Use existing conventions or create descriptive names like:
- **"Pointer"** - DPI, sniping settings
- **"Gestures"** - Gesture enables and keycodes
- **"Advanced"** - Expert configuration

### Size and Character Limits

- No explicit character limit mentioned in VIA docs
- JSON must be valid (syntax errors prevent loading)
- Practical limit: Keep menus concise for usability
- VIA app can handle large definitions (built-in RGB menus are extensive)

---

## Part 2: Dilemma v3 Menu Hierarchy Design

### Current VIA Value ID Mapping

From `dilemma.h` enum `via_dilemma_value_id`:

| ID | Name | Type | EEPROM Location |
|----|------|------|-----------------|
| 0 | `id_dilemma_unhandled` | System | - |
| 1 | `id_dilemma_config_save` | Action | - |
| 2 | `id_dilemma_dpi_preset` | uint8 (0-7) | Byte 0, bits 0-2 |
| 3 | `id_dilemma_custom_dpi` | uint16 (0-4095) | Bytes 1-2, bits 0-11 |
| 4 | `id_dilemma_drag_scroll_x` | uint8 (0-15) | Byte 3, bits 0-3 |
| 5 | `id_dilemma_drag_scroll_y` | uint8 (0-15) | Byte 3, bits 4-7 |
| 6 | `id_dilemma_two_finger_x` | uint8 (0-15) | Byte 4, bits 0-3 |
| 7 | `id_dilemma_two_finger_y` | uint8 (0-15) | Byte 4, bits 4-7 |
| 8 | `id_dilemma_tap_to_click_enable` | toggle | Byte 6, bit 0 |
| 9 | `id_dilemma_two_finger_tap_enable` | toggle | Byte 6, bit 1 |
| 10 | `id_dilemma_two_finger_scroll_enable` | toggle | Byte 6, bit 2 |
| 11 | `id_dilemma_press_and_hold_enable` | toggle | Byte 6, bit 3 |
| 12 | `id_dilemma_three_finger_swipe_enable` | toggle | Byte 7, bit 0 |
| 13 | `id_dilemma_four_finger_swipe_enable` | toggle | Byte 7, bit 1 |
| 14 | `id_dilemma_pinch_to_zoom_enable` | toggle | Byte 7, bit 2 |
| 15 | `id_dilemma_three_finger_swipe_left_keycode` | keycode (uint16) | Bytes 18-19 |
| 16 | `id_dilemma_three_finger_swipe_right_keycode` | keycode (uint16) | Bytes 20-21 |
| 17 | `id_dilemma_three_finger_swipe_up_keycode` | keycode (uint16) | Bytes 22-23 |
| 18 | `id_dilemma_three_finger_swipe_down_keycode` | keycode (uint16) | Bytes 24-25 |
| 19 | `id_dilemma_four_finger_swipe_left_keycode` | keycode (uint16) | Bytes 26-27 |
| 20 | `id_dilemma_four_finger_swipe_right_keycode` | keycode (uint16) | Bytes 28-29 |
| 21 | `id_dilemma_four_finger_swipe_up_keycode` | keycode (uint16) | Bytes 30-31 |
| 22 | `id_dilemma_four_finger_swipe_down_keycode` | keycode (uint16) | Bytes 32-33 |
| 23 | `id_dilemma_zoom_in_keycode` | keycode (uint16) | Bytes 34-35 |
| 24 | `id_dilemma_zoom_out_keycode` | keycode (uint16) | Bytes 36-37 |
| 25 | `id_dilemma_auto_snipe_enable` | toggle | Byte 5, bit 0 |
| 26 | `id_dilemma_auto_snipe_layer` | uint8 (0-127) | Byte 5, bits 1-7 |

**Total: 27 value IDs (0-26) implementing 30+ settings**

### Proposed Menu Structure

```
Dilemma Settings (root top-level menu)
├── Pointer
│   ├── DPI Preset (dropdown: 0-7)
│   ├── Custom DPI (range: 200-4000)
│   ├── Drag Scroll Speed X (range: 1-15)
│   └── Drag Scroll Speed Y (range: 1-15)
├── Scrolling
│   ├── Two-Finger Scroll Speed X (range: 1-15)
│   └── Two-Finger Scroll Speed Y (range: 1-15)
├── Basic Gestures
│   ├── Tap to Click (toggle)
│   ├── Two-Finger Tap (toggle)
│   ├── Two-Finger Scroll (toggle)
│   └── Press and Hold (toggle)
├── Advanced Gestures
│   ├── Three-Finger Swipe (toggle)
│   │   ├── Left Keycode (keycode)
│   │   ├── Right Keycode (keycode)
│   │   ├── Up Keycode (keycode)
│   │   └── Down Keycode (keycode)
│   ├── Four-Finger Swipe (toggle)
│   │   ├── Left Keycode (keycode)
│   │   ├── Right Keycode (keycode)
│   │   ├── Up Keycode (keycode)
│   │   └── Down Keycode (keycode)
│   └── Pinch to Zoom (toggle)
│       ├── Zoom In Keycode (keycode)
│       └── Zoom Out Keycode (keycode)
└── Advanced
    ├── Auto-Snipe Enable (toggle)
    ├── Auto-Snipe Layer (range: 0-127)
    ├── Save (button)
    └── Reset (button)
```

### Menu Design Principles

1. **Logical Grouping**: Related settings together (Pointer, Scrolling, Gestures, Advanced)
2. **Progressive Disclosure**: Advanced features nested separately, shown when enabled
3. **Clear Labels**: User-friendly names, not technical terms
4. **Smart Defaults**: All settings have sensible defaults marked in firmware
5. **Action Buttons**: Save and Reset at bottom of Advanced menu
6. **Conditional Display**: Keycode controls only show when gesture is enabled (using `showIf`)

### Detailed Menu Specifications

#### Top-Level Menu 1: **Pointer**

**Sub-menu:** Pointer Settings

| Control | Type | Options | Value ID | Default |
|---------|------|---------|----------|---------|
| DPI Preset | dropdown | [800, 1200, 1600, 2000, 2400, 2800, Custom] | 2 | 3 (1600) |
| Custom DPI | range | [200, 4000] | 3 | 1600 |
| Drag Scroll X | range | [1, 15] | 4 | 8 |
| Drag Scroll Y | range | [1, 15] | 5 | 8 |

**Notes:**
- DPI preset dropdown maps to enum values 0-6 (0=800, 1=1200, 2=1600, 3=2000, 4=2400, 5=2800, 6=Custom)
- Custom DPI only applies when DPI preset = 6 (Custom)
- Could add `showIf` to show Custom DPI only when preset = 6

#### Top-Level Menu 2: **Scrolling**

**Sub-menu:** Two-Finger Scroll

| Control | Type | Options | Value ID | Default |
|---------|------|---------|----------|---------|
| Two-Finger X | range | [1, 15] | 6 | 8 |
| Two-Finger Y | range | [1, 15] | 7 | 8 |

**Notes:**
- Independent from drag-scroll divisors
- Separate X/Y control for different horizontal/vertical scroll speeds

#### Top-Level Menu 3: **Basic Gestures**

**Sub-menu:** Gesture Enables

| Control | Type | Options | Value ID | Default |
|---------|------|---------|----------|---------|
| Tap to Click | toggle | - | 8 | 1 (enabled) |
| Two-Finger Tap | toggle | - | 9 | 1 (enabled) |
| Two-Finger Scroll | toggle | - | 10 | 1 (enabled) |
| Press and Hold | toggle | - | 11 | 0 (disabled) |

**Notes:**
- All basic gestures enabled by default (backward compatible)
- Press and Hold disabled by default (opt-in force click feature)

#### Top-Level Menu 4: **Advanced Gestures**

**Sub-menu 1:** Three-Finger Swipe

| Control | Type | Options | Value ID | Default |
|---------|------|---------|----------|---------|
| Three-Finger Swipe Enable | toggle | - | 12 | 1 (enabled) |
| Left Keycode | keycode | - | 15 | LCTL(KC_TAB) |
| Right Keycode | keycode | - | 16 | LCTL(LSFT(KC_TAB)) |
| Up Keycode | keycode | - | 17 | KC_F11 |
| Down Keycode | keycode | - | 18 | KC_F10 |

**Sub-menu 2:** Four-Finger Swipe

| Control | Type | Options | Value ID | Default |
|---------|------|---------|----------|---------|
| Four-Finger Swipe Enable | toggle | - | 13 | 0 (disabled) |
| Left Keycode | keycode | - | 19 | LCTL(LALT(KC_LEFT)) |
| Right Keycode | keycode | - | 20 | LCTL(LALT(KC_RIGHT)) |
| Up Keycode | keycode | - | 21 | LCTL(KC_UP) |
| Down Keycode | keycode | - | 22 | KC_Desktop |

**Sub-menu 3:** Pinch to Zoom

| Control | Type | Options | Value ID | Default |
|---------|------|---------|----------|---------|
| Pinch to Zoom Enable | toggle | - | 14 | 0 (disabled) |
| Zoom In Keycode | keycode | - | 23 | KC_EQUAL |
| Zoom Out Keycode | keycode | - | 24 | KC_MINUS |

**Notes:**
- Use `showIf` to hide keycode controls when gesture is disabled
- Example: `showIf: "{id_dilemma_three_finger_swipe_enable} == 1"`
- Default keycodes match macOS behavior

#### Top-Level Menu 5: **Advanced**

**Sub-menu:** Expert Configuration

| Control | Type | Options | Value ID | Default |
|---------|------|---------|----------|---------|
| Auto-Snipe Enable | toggle | - | 25 | 0 (disabled) |
| Auto-Snipe Layer | range | [0, 127] | 26 | 2 |
| Save Config | button | [1] | 1 | - |
| Reset to Defaults | button | [2] | 1 | - |

**Notes:**
- Auto-Snipe Layer only relevant when Auto-Snipe Enable is on
- Could add `showIf` for layer control
- Save button sends value 1, Reset sends value 2 (firmware distinguishes by value)

### Menu JSON Example (Pointer Section)

```json
{
  "label": "Pointer",
  "content": [
    {
      "label": "Pointer Settings",
      "content": [
        {
          "label": "DPI Preset",
          "type": "dropdown",
          "content": ["id_dilemma_dpi_preset", 0, 2],
          "options": [
            "800 DPI",
            "1200 DPI",
            "1600 DPI",
            "2000 DPI",
            "2400 DPI",
            "2800 DPI",
            "Custom"
          ]
        },
        {
          "label": "Custom DPI",
          "type": "range",
          "options": [200, 4000],
          "content": ["id_dilemma_custom_dpi", 0, 3]
        },
        {
          "label": "Drag Scroll Speed X",
          "type": "range",
          "options": [1, 15],
          "content": ["id_dilemma_drag_scroll_x", 0, 4]
        },
        {
          "label": "Drag Scroll Speed Y",
          "type": "range",
          "options": [1, 15],
          "content": ["id_dilemma_drag_scroll_y", 0, 5]
        }
      ]
    }
  ]
}
```

### Conditional Display Example (Advanced Gestures)

```json
{
  "label": "Advanced Gestures",
  "content": [
    {
      "label": "Three-Finger Swipe",
      "content": [
        {
          "label": "Enable Three-Finger Swipe",
          "type": "toggle",
          "content": ["id_dilemma_three_finger_swipe_enable", 0, 12]
        },
        {
          "showIf": "{id_dilemma_three_finger_swipe_enable} == 1",
          "content": [
            {
              "label": "Left Keycode",
              "type": "keycode",
              "content": ["id_dilemma_three_finger_swipe_left_keycode", 0, 15]
            },
            {
              "label": "Right Keycode",
              "type": "keycode",
              "content": ["id_dilemma_three_finger_swipe_right_keycode", 0, 16]
            },
            {
              "label": "Up Keycode",
              "type": "keycode",
              "content": ["id_dilemma_three_finger_swipe_up_keycode", 0, 17]
            },
            {
              "label": "Down Keycode",
              "type": "keycode",
              "content": ["id_dilemma_three_finger_swipe_down_keycode", 0, 18]
            }
          ]
        }
      ]
    }
  ]
}
```

---

## Part 3: Implementation Strategy

### File Structure

```
keyboards/bastardkb/dilemma/
└── via.json  (NEW FILE)
```

### via.json Top-Level Structure

```json
{
  "menus": [
    {
      "label": "Pointer",
      "content": [ /* Pointer settings */ ]
    },
    {
      "label": "Scrolling",
      "content": [ /* Two-finger scroll settings */ ]
    },
    {
      "label": "Basic Gestures",
      "content": [ /* Basic gesture enables */ ]
    },
    {
      "label": "Advanced Gestures",
      "content": [ /* Advanced gestures with keycodes */ ]
    },
    {
      "label": "Advanced",
      "content": [ /* Auto-snipe + Save/Reset */ ]
    }
  ]
}
```

### Plan 08-02: Basic Menu Implementation

**Scope:** Pointer, Scrolling, Basic Gestures, Actions (10-15 items)

**Tasks:**
1. Create `keyboards/bastardkb/dilemma/via.json`
2. Define Pointer container (4 items: DPI preset, custom DPI, drag scroll X/Y)
3. Define Scrolling container (2 items: two-finger scroll X/Y)
4. Define Basic Gestures container (4 items: tap enables)
5. Define Advanced container (3 items: auto-snipe + Save/Reset buttons)
6. Validate JSON syntax
7. Test in VIA (if possible)

**Validation Checklist:**
- [ ] JSON syntax valid
- [ ] Value IDs 2-11 mapped correctly
- [ ] Value IDs 25-26 mapped correctly
- [ ] Channel ID = 0 (custom channel)
- [ ] Min/max values match firmware validation
- [ ] Defaults match `eeconfig_init_kb()`

**Estimated Duration:** 15 minutes

### Plan 08-03: Advanced Menu Implementation

**Scope:** Advanced Gestures with keycodes (remaining 15-20 items)

**Tasks:**
1. Add Advanced Gestures top-level menu
2. Define Three-Finger Swipe container (5 items: enable + 4 keycodes)
3. Define Four-Finger Swipe container (5 items: enable + 4 keycodes)
4. Define Pinch to Zoom container (3 items: enable + 2 keycodes)
5. Add `showIf` conditional display for keycode controls
6. Verify all 27 value IDs represented
7. Test menu navigation in VIA

**Validation Checklist:**
- [ ] All 26 VIA value IDs (0-26) represented in menus
- [ ] Conditional display working (keycodes hidden when gestures disabled)
- [ ] Keycode controls functional (16-bit value handling)
- [ ] Menu hierarchy navigable in VIA
- [ ] No VIA size/character limits exceeded
- [ ] Labels clear and user-friendly
- [ ] Organization logical (Pointer, Scrolling, Gestures, Advanced)

**Estimated Duration:** 15 minutes

### Implementation Order

1. **Plan 08-02 (Basic)**
   - Create via.json file
   - Add Pointer container (4 items)
   - Add Scrolling container (2 items)
   - Add Basic Gestures container (4 items)
   - Add Advanced container (3 items)
   - Test basic menu works

2. **Plan 08-03 (Advanced)**
   - Add Advanced Gestures top-level menu
   - Add Three-Finger Swipe container (5 items)
   - Add Four-Finger Swipe container (5 items)
   - Add Pinch to Zoom container (3 items)
   - Implement `showIf` for all keycode controls
   - Verify all 30+ settings accessible

### Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| VIA menu size/character limits | LOW | MEDIUM | Incremental implementation, monitor during testing |
| JSON syntax errors | LOW | HIGH | Validate JSON before testing, use linter |
| Value ID mismatches | LOW | HIGH | Verify against enum in dilemma.h |
| `showIf` expression errors | LOW | MEDIUM | Test conditional display thoroughly |
| 16-bit keycode handling | LOW | MEDIUM | Follow VIA documentation for high/low byte order |

**Overall Risk: LOW**

All risks are mitigated by incremental implementation, validation at each step, and following VIA documentation patterns.

### Testing Strategy

1. **JSON Validation:** Use online JSON validator to catch syntax errors
2. **VIA Loading:** Load via.json in VIA configurator (if possible)
3. **Menu Navigation:** Navigate all menus, verify structure
4. **Value Changes:** Test changing values in VIA, verify firmware receives them
5. **Conditional Display:** Enable/disable gestures, verify keycode controls show/hide
6. **Save/Reset:** Test Save and Reset buttons functionality

---

## Summary

### Key Findings

1. **VIA Menu System:** Well-documented with clear JSON structure
2. **UI Control Types:** 6 types available (range, toggle, dropdown, button, keycode, color)
3. **Conditional Display:** `showIf` enables progressive disclosure
4. **16-bit Values:** Proper byte order handling documented
5. **No Size Limits:** No explicit character limits in VIA docs
6. **Channel System:** Custom features use channel_id = 0
7. **Top-Level Conventions:** "Lighting", "Audio", "Display" have icons

### Menu Design Decisions

1. **5 Top-Level Menus:** Pointer, Scrolling, Basic Gestures, Advanced Gestures, Advanced
2. **Logical Grouping:** Related settings together for usability
3. **Conditional Display:** Keycode controls hidden when gestures disabled
4. **User-Friendly Labels:** Clear names, not technical terms
5. **Progressive Disclosure:** Basic features first, advanced nested separately

### Implementation Readiness

- ✅ VIA menu format fully understood
- ✅ All 27 value IDs mapped to menu controls
- ✅ Menu hierarchy designed with 30+ settings
- ✅ JSON structure examples documented
- ✅ Implementation strategy clear for Plans 08-02 and 08-03
- ✅ Risks identified with mitigations

**Next Steps:**
- Plan 08-02: Implement basic menus (Pointer, Scrolling, Basic Gestures, Advanced)
- Plan 08-03: Implement advanced gesture menus with conditional display

---

**Sources:**

- [VIA Custom UI Documentation](https://caniusevia.com/docs/custom_ui/)
- [VIA Specification](https://caniusevia.com/docs/specification/)
- [VIA Built-in UI Definitions Repository](https://github.com/the-via/keyboards/tree/master/common-menus)
