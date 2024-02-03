#include "core/input.h"
#include "core/event.h"
#include "core/kmemory.h"
#include "core/logger.h"

typedef struct keyboard_state {
    b8 keys[256];
} keyboard_state;

typedef struct mouse_state {
    i16 x;
    i16 y;
    u8 buttons[BUTTON_MAX_BUTTONS];
} mouse_state;

typedef struct input_state {
    keyboard_state keyboard_current;
    keyboard_state keyboard_previous;
    mouse_state mouse_current;
    mouse_state mouse_previous;
} input_state;

// Internal input state
static b8 initialized = FALSE;
static input_state state = {};

void input_initialize()
{
    kzero_memory(&state, sizeof(input_state));
    initialized = TRUE;
    KINFO("Input subsystem initialized");
}

void input_shutdown()
{
    // TODO: Add shutdown routines when needed.
    initialized = FALSE;
}

void input_update(f64 delta_time)
{
    if (!initialized) {
        return;
    }

    // Copy current states to previous states.
    kcopy_memory(&state.keyboard_previous, &state.keyboard_current, sizeof(keyboard_state));
    kcopy_memory(&state.mouse_previous, &state.mouse_current, sizeof(mouse_state));
}

void input_process_key(keys key, b8 pressed)
{
    // Only handle this if the state actually changed.
    if (state.keyboard_current.keys[key] != pressed) {
        // Update internal state.
        state.keyboard_current.keys[key] = pressed;

        // Fire off an event for immediate processing.
        const event_context context = { .data.u16[0] = key };
        event_fire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, context);
    }
}

void input_process_button(buttons button, b8 pressed)
{
    // Only handle this if the state actually changed.
    if (state.mouse_current.buttons[button] != pressed) {
        // Update internal state.
        state.mouse_current.buttons[button] = pressed;

        // Fire off an event for immediate processing.
        const event_context context = { .data.u16[0] = button };
        event_fire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, 0, context);
    }
}

void input_process_mouse_move(i16 x, i16 y)
{
    // Only handle this if the state actually changed.
    if (state.mouse_current.x != x || state.mouse_current.y != y) {
        // NOTE: Enable this if debugging.
        // KDEBUG("Mouse pos: %i, %i!", x, y);
        
        // Update internal state.
        state.mouse_current.x = x;
        state.mouse_current.y = y;

        // Fire the event.
        const event_context context = { .data.u16 = { x, y } };
        event_fire(EVENT_CODE_MOUSE_MOVED, 0, context);
    }
}

void input_process_mouse_wheel(i8 z_delta)
{
    // NOTE: no internal state to update.
    
    // Fire the event.
    const event_context context = { .data.u8[0] = z_delta };
    event_fire(EVENT_CODE_MOUSE_WHEEL, 0, context);
}

b8 input_is_key_down(keys key)
{
    return initialized && state.keyboard_current.keys[key];
}

b8 input_is_key_up(keys key)
{
    return !initialized || !state.keyboard_current.keys[key];
}

b8 input_was_key_down(keys key)
{
    return initialized && state.keyboard_previous.keys[key];
}

b8 input_was_key_up(keys key)
{
    return !initialized || !state.keyboard_previous.keys[key];
}

b8 input_is_button_down(buttons button)
{
    return initialized && state.mouse_current.buttons[button];
}

b8 input_is_button_up(buttons button)
{
    return !initialized || !state.mouse_current.buttons[button];
}

b8 input_was_button_down(buttons button)
{
    return initialized && state.mouse_previous.buttons[button];
}

b8 input_was_button_up(buttons button)
{
    return !initialized || !state.mouse_previous.buttons[button];
}

void input_get_mouse_position(i32 *x, i32 *y)
{
    if (!initialized) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = state.mouse_current.x;
    *y = state.mouse_current.y;
}

void input_get_previous_mouse_position(i32 *x, i32 *y)
{
    if (!initialized) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = state.mouse_previous.x;
    *y = state.mouse_previous.y;
}
