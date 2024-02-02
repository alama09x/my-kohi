#include "core/event.h"

#include "core/kmemory.h"
#include "containers/darray.h"

typedef struct registered_event {
    void *listener;
    PFN_on_event callback;
} registered_event;

typedef struct event_code_entry {
    registered_event *events;
} event_code_entry;

// This should be more than enough codes...
#define MAX_MESSAGE_CODES 16384 // 0x4000

// State structure.
typedef struct event_system_state {
    // Lookup table for event codes.
    event_code_entry registered[MAX_MESSAGE_CODES];
} event_system_state;

/**
 * Event system internal state
 */
static b8 is_initialized = FALSE;
static event_system_state state;

b8 event_initialize()
{
    if (is_initialized) {
        return FALSE;
    }

    kzero_memory(&state, sizeof(state));
    return is_initialized = TRUE;
}

void event_shutdown() {
    // Free the events arrays. Objects pointed to should be destroyed on their own.
    for (u16 i = 0; i < MAX_MESSAGE_CODES; ++i) {
        if (state.registered[i].events != 0) {
            darray_destroy(state.registered[i].events);
            state.registered[i].events = 0;
        }
    }
}

b8 event_register(u16 code, void *listener, PFN_on_event on_event)
{
    if (!is_initialized) {
        return FALSE;
    }

    if (state.registered[code].events == 0) {
        state.registered[code].events = darray_create(registered_event);
    }

    const u64 registered_count = darray_length(state.registered[code].events);
    for (u64 i = 0; i < registered_count; ++i) {
        if (state.registered[code].events[i].listener == listener) {
            // TODO: warn
            return FALSE;
        }
    }

    // If at this point, no duplicate was found; proceed with registration.
    registered_event event = {
        .listener = listener,
        .callback = on_event,
    };
    darray_push(state.registered[code].events, event);

    return TRUE;
}

b8 event_unregister(u16 code, void *listener, PFN_on_event on_event)
{
    if (!is_initialized) {
        return FALSE;
    }

    // On nothing is registered for the code, boot out.
    if (state.registered[code].events == 0) {
        // TODO: warn
        return FALSE;
    }

    const u64 registered_count = darray_length(state.registered[code].events);
    for (u64 i = 0; i < registered_count; ++i) {
        const registered_event e = state.registered[code].events[i];
        if (e.listener == listener && e.callback == on_event) {
            // Found one, remove it
            registered_event popped_event;
            darray_pop_at(state.registered[code].events, i, &popped_event);
            return TRUE;
        }
    }

    // Not found.
    return FALSE;
}

b8 event_fire(u16 code, void *sender, event_context context)
{
    if (!is_initialized) {
        return FALSE;
    }

    // If nothing is registered for the code, boot out.
    if (state.registered[code].events == 0) {
        return FALSE;
    }

    const u64 registered_count = darray_length(state.registered[code].events);
    for (u64 i = 0; i < registered_count; ++i) {
        const registered_event e = state.registered[code].events[i];
        if (e.callback(code, sender, e.listener, context)) {
            // Message has been handled, do not send to other listeners.
            return TRUE;
        }
    }
    // Not found.
    return FALSE;
}
