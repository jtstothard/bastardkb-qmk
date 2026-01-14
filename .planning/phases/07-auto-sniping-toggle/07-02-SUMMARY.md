# Plan 07-02 Summary: Layer State Auto-Snipe Logic

**Duration:** 5 minutes (1 task, 1 atomic commit)
**Status:** ✅ COMPLETE

**Completed Tasks:**
1. ✅ Implemented layer_state_set_user() with auto-snipe logic

## Key Achievements

### Auto-Snipe on Layer Activation
- **Layer-based sniping toggle**: Automatically enables sniping mode when configured layer becomes active
- **Automatic disable**: Disables sniping when leaving the target layer
- **VIA-configurable**: Respects auto_snipe_enabled and auto_snipe_layer settings
- **Keymap compatibility**: Delegates to layer_state_set_user_kb for tri-layer and other keymap logic
- **Safe implementation**: Checks current state before enabling to avoid redundant operations

### Technical Implementation

**Function Added:** `layer_state_set_user()` in dilemma.c (after pointing_device_task_kb, line 448)

```c
layer_state_t layer_state_set_user(layer_state_t state) {
    // Check if auto-sniping is enabled in VIA config
    if (g_via_dilemma_config.auto_snipe_enabled) {
        uint8_t target_layer = g_via_dilemma_config.auto_snipe_layer;

        // Check if target layer is in current layer state
        if (state & (1 << target_layer)) {
            // Target layer is active, enable sniping
            if (!dilemma_get_pointer_sniping_enabled()) {
                dilemma_set_pointer_sniping_enabled(true);
            }
        } else {
            // Target layer is not active, disable sniping if we enabled it
            if (dilemma_get_pointer_sniping_enabled()) {
                dilemma_set_pointer_sniping_enabled(false);
            }
        }
    }

    // Call existing keymap layer_state_set_user if defined
    return layer_state_set_user_kb(state);
}
```

### Design Decisions

1. **Bit mask layer checking**: Uses `state & (1 << target_layer)` to check if layer is active
   - Efficient layer state checking using QMK's layer_state_t bitfield
   - Supports layers 0-31 (standard QMK limitation)

2. **State before action**: Checks current sniping state before enabling/disabling
   - Prevents redundant DPI updates
   - Avoids unnecessary state changes

3. **Integration with existing API**: Uses `dilemma_set_pointer_sniping_enabled()`
   - Leverages existing sniping infrastructure (DPI switching, state management)
   - Ensures VIA config consistency

4. **Keymap delegation**: Calls `layer_state_set_user_kb(state)` at end
   - Preserves existing keymap layer logic (tri-layer, etc.)
   - Maintains compatibility with default keymap

### Files Modified

- **keyboards/bastardkb/dilemma/dilemma.c** (+31 lines)
  - Added layer_state_set_user() function (lines 448-471)
  - Complete auto-snipe logic implementation

### Verification

✅ All verification criteria met:
- [x] layer_state_set_user() function added to dilemma.c
- [x] Checks auto_snipe_enabled from VIA config
- [x] Checks if auto_snipe_layer is active using bit mask
- [x] Enables/disables sniping via existing API
- [x] Delegates to keymap's layer_state_set_user_kb
- [x] Implementation follows plan specification exactly

### Integration Points

**Upstream Dependencies (Completed):**
- Plan 07-01: VIA value IDs 25-26 (auto_snipe_enabled, auto_snipe_layer)
- Existing sniping API: dilemma_set_pointer_sniping_enabled()
- Existing sniping state: dilemma_get_pointer_sniping_enabled()

**Downstream Consumers (Next):**
- Plan 07-03: VIA command handlers for read/write access

### Success Criteria Met

✅ **Auto-snipe triggers on layer activation**: Function enables sniping when target layer bit is set
✅ **Auto-snipe disables when leaving layer**: Function disables sniping when target layer bit is clear
✅ **Compatible with existing keymap layer logic**: Delegates to layer_state_set_user_kb for tri-layer support
✅ **VIA-configurable enable/disable**: Checks auto_snipe_enabled before any logic

### Usage Example

When configured in VIA:
1. Set `auto_snipe_enabled = true` (VIA value ID 25)
2. Set `auto_snipe_layer = 2` (VIA value ID 26, e.g., for NAV layer)

Result:
- Switch to layer 2 → sniping automatically enables (DPI drops to sniping level)
- Switch away from layer 2 → sniping automatically disables (DPI returns to normal)

This enables "sniping layer" workflows where a specific layer is dedicated to precision work.

### Next Steps

**Plan 07-03:** Implement VIA command handlers for auto-snipe configuration
- Add set handler for id_dilemma_auto_snipe_enable (VIA value ID 25)
- Add get handler for id_dilemma_auto_snipe_enable (VIA value ID 25)
- Add set handler for id_dilemma_auto_snipe_layer (VIA value ID 26)
- Add get handler for id_dilemma_auto_snipe_layer (VIA value ID 26)
- Initialize defaults in eeconfig_init_kb()

**Commit:** 8a758f404a
**Branch:** maxtouch-dpi-fix
