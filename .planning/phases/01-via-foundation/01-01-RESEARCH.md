# VIA Protocol Research - Plan 01-01

**Research Phase:** Understanding VIA custom command protocol and EEPROM patterns

## Task 1: VIA Custom Command Packet Structure

### Protocol Overview
VIA uses USB Raw HID for communication between the VIA configurator and QMK firmware. The packet structure is defined in `quantum/via.c`:

```
data[0] = command_id
data[1] = channel_id
data[2] = value_id
data[3+] = value_data
```

### Command Types (from `quantum/via.h`)

**Custom Value Commands:**
- `id_custom_set_value = 0x07` - Set a custom value
- `id_custom_get_value = 0x08` - Get a custom value
- `id_custom_save = 0x09` - Save custom values to EEPROM

### Channel System

Channel IDs (from `enum via_channel_id`):
- `id_custom_channel = 0` - Keyboard-specific custom values
- `id_qmk_backlight_channel = 1` - QMK backlight
- `id_qmk_rgblight_channel = 2` - QMK RGB light
- `id_qmk_rgb_matrix_channel = 3` - QMK RGB matrix
- `id_qmk_audio_channel = 4` - QMK audio
- `id_qmk_led_matrix_channel = 5` - QMK LED matrix

### Implementation Pattern

**Override Point:** `via_custom_value_command_kb(uint8_t *data, uint8_t length)`

Default implementation (weak symbol in `quantum/via.c:216-221`):
```c
__attribute__((weak)) void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    // data = [ command_id, channel_id, value_id, value_data ]
    uint8_t *command_id = &(data[0]);
    // Return the unhandled state
    *command_id = id_unhandled;
}
```

**Return Protocol:**
- Set `data[0]` to response code before `raw_hid_send()`
- Use `id_unhandled (0xFF)` if value_id is not recognized
- Otherwise, keep command_id unchanged and populate value_data

### Command Flow

1. VIA sends `id_custom_set_value` with channel_id=0, value_id=X, value_data
2. Firmware parses value_id, sets in-memory value
3. VIA sends `id_custom_save` to persist to EEPROM
4. Firmware calls `nvm_via_update_custom_config()` to write EEPROM

## Task 2: EEPROM NVM API Constraints

### EEPROM Layout

VIA's EEPROM is divided into sections:
- **Layout Options:** `VIA_EEPROM_LAYOUT_OPTIONS_SIZE` (default 1 byte)
- **Custom Config:** `VIA_EEPROM_CUSTOM_CONFIG_SIZE` (default 0, must be defined)
- **Dynamic Keymaps:** Separate region
- **Macros:** Separate region

### Custom Config Constraints

**Maximum Size:** 32 bytes

This is found in multiple keyboards using `VIA_EEPROM_CUSTOM_CONFIG_SIZE 32`.

**Location:** After VIA's core data, defined by `VIA_EEPROM_CUSTOM_CONFIG_ADDR`

### NVM API Functions

**Read Function:**
```c
uint32_t nvm_via_read_custom_config(void *buf, uint32_t offset, uint32_t length);
```
- `buf`: Buffer to read into
- `offset`: Byte offset within custom config region (0-31)
- `length`: Number of bytes to read
- Returns: Number of bytes actually read

**Write Function:**
```c
uint32_t nvm_via_update_custom_config(const void *buf, uint32_t offset, uint32_t length);
```
- `buf`: Buffer containing data to write
- `offset`: Byte offset within custom config region (0-31)
- `length`: Number of bytes to write
- Returns: Number of bytes actually written

### Current Dilemma Usage vs. VIA Custom Config

**Current System:**
- Uses `eeconfig_read_kb()` and `eeconfig_update_kb()`
- Stores 1 byte in KB-level EEPROM (separate from VIA)
- Bit-packed: 4 bits default_dpi, 2 bits sniping_dpi, 1 bit dragscroll, 1 bit sniping

**New VIA System:**
- Separate 32-byte custom config region
- Accessed via `nvm_via_*_custom_config()` functions
- Can store all planned features (DPI, scroll divisors, gestures, etc.)

**Migration Strategy:**
1. Keep current `eeconfig_read_kb()` for backward compatibility during transition
2. Read from both old and new locations
3. Migrate values to new VIA custom config
4. Eventually deprecate old system

## Task 3: Ploopy VIA Menu Patterns

### Reference Repository
https://github.com/plodah/ploody_viamenus

**Note:** This task involves cloning and analyzing the Ploopy repository. Ploopy is a trackball with VIA customization, making it the closest reference for Dilemma's trackpad customization needs.

### Expected Findings (to be verified)

**via_menus.json Structure:**
- Menu organization (nested groups)
- Value types (uint8, uint16, uint32, enum, checkbox)
- Value ID mapping to custom value IDs
- Labels and descriptions for VIA UI

**Bit-Packing Patterns:**
- Multiple settings stored in single bytes
- Bit-field definitions
- Mask/shift operations for packing/unpacking

**via_custom_value_command_kb() Implementation:**
- Value ID switch statement
- Read/write to custom config struct
- Bit manipulation for packed values

### Value ID Mapping Pattern

**Expected Pattern:**
```
value_id 0x01: setting_a
value_id 0x02: setting_b
value_id 0x03: setting_c
...
```

Each value ID corresponds to:
- A field in the config struct (direct or bit-packed)
- An entry in via_menus.json
- Handling in via_custom_value_command_kb()

## Task 4: Dilemma's Current EEPROM Usage

### Current Config Structure (dilemma.c:61-69)

```c
typedef union {
    uint8_t raw;
    struct {
        uint8_t pointer_default_dpi : 4;  // 4 bits = 16 steps
        uint8_t pointer_sniping_dpi : 2;  // 2 bits = 4 steps
        bool    is_dragscroll_enabled : 1;
        bool    is_sniping_enabled : 1;
    } __attribute__((packed));
} dilemma_config_t;
```

**Total:** 1 byte

### EEPROM Functions

**Read (dilemma.c:81-85):**
```c
static void read_dilemma_config_from_eeprom(dilemma_config_t *config) {
    config->raw = eeconfig_read_kb() & 0xff;
    config->is_dragscroll_enabled = false;  // Don't persist runtime state
    config->is_sniping_enabled = false;
}
```

**Write (dilemma.c:95-96):**
```c
static void write_dilemma_config_to_eeprom(dilemma_config_t *config) {
    eeconfig_update_kb(config->raw);
}
```

### Current Settings

**Default DPI:**
- 4 bits (16 steps: 0-15)
- Formula: `step * DILEMMA_DEFAULT_DPI_CONFIG_STEP (200) + DILEMMA_MINIMUM_DEFAULT_DPI (400)`
- Range: 400 to 3400 DPI
- Default: step 3 = 1000 DPI

**Sniping DPI:**
- 2 bits (4 steps: 0-3)
- Formula: `step * DILEMMA_SNIPING_DPI_CONFIG_STEP (100) + DILEMMA_MINIMUM_SNIPING_DPI (200)`
- Range: 200 to 500 DPI
- Default: step 0 = 200 DPI

**Dragscroll DPI:**
- Fixed at `DILEMMA_DRAGSCROLL_DPI (100)`
- Not configurable

**Runtime Flags:**
- `is_dragscroll_enabled` - NOT persisted (reset on boot)
- `is_sniping_enabled` - NOT persisted (reset on boot)

### Migration Strategy

**Phase 1: Coexistence**
1. Define VIA custom config struct with expanded fields
2. Set `VIA_EEPROM_CUSTOM_CONFIG_SIZE 32` in config.h
3. Implement `via_custom_value_command_kb()` for new values
4. Keep reading old `eeconfig_read_kb()` for compatibility
5. On boot, migrate old values to new struct if new config is empty

**Phase 2: Transition**
1. VIA app uses new custom values
2. Both old and new values work in parallel
3. User can switch between them seamlessly

**Phase 3: Deprecation**
1. Remove old `eeconfig_read_kb()` reading
2. Remove old dilemma_config_t union
3. Use only VIA custom config

### Planned New Config Features

**With 32 bytes available, can store:**

1. **DPI Settings (2 bytes):**
   - Default DPI: uint16 (200-12000 range)
   - Sniping DPI: uint16 (200-12000 range)
   - Dragscroll DPI: uint16 (fixed 100)

2. **Scroll Divisors (2 bytes):**
   - Horizontal divisor: uint8 (1-255)
   - Vertical divisor: uint8 (1-255)

3. **Gesture Settings (4 bytes):**
   - Tap-to-click enable: bool
   - Tap drag enable: bool
   - Middle click timeout: uint8
   - Other gesture flags: 5 bits reserved

4. **Advanced Settings (remaining bytes):**
   - Acceleration curves
   - Angle snapping
   - Liftoff distance
   - Future features

**Total Planned:** ~10-15 bytes, leaving room for expansion

## Summary

### Key Findings

1. **VIA Protocol:** Well-defined packet structure with command/channel/value IDs
2. **EEPROM Constraint:** 32-byte max for custom config
3. **Ploopy Reference:** Trackball with similar customization needs (to be analyzed)
4. **Current Dilemma:** 1-byte config with bit-packed fields
5. **Migration Path:** Coexistence → Transition → Deprecation

### Next Steps

- **Plan 01-02:** Design EEPROM layout and value ID mapping
- **Plan 01-03:** Implement via_custom_value_command_kb()
- **Analyze Ploopy:** Clone and study ploody_viamenus implementation

### Implementation Guidance

This research provides the foundation for:
1. Designing Dilemma's custom config struct
2. Mapping value IDs for VIA menu
3. Implementing the command handler
4. Ensuring backward compatibility during migration
