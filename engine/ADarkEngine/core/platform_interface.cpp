
#include "ADarkEngine/core/platform_interface.h"

internal OS_event_list
GenerateEventList(memory_arena* arena, u32 size)
{
    OS_event_list result;
    
    result.events = 
        (OS_event*)ArenaAlloc(arena, size * sizeof(OS_event));
    result.writeOffset = 0;
    result.readOffset = 0;
    result.size = size;
    
    return result;
}

internal void
PushOSEvent(OS_event_list* list,
            OS_event event)
{
    list->events[list->writeOffset] = event;
    
    ++list->writeOffset;
}

internal OS_event
OSEvent(OS_event_type type)
{
    OS_event event = {};
    event.eventType = type;
    return event;
}

internal OS_event
KeyEvent(key_code keyCode,
         key_state keyState)
{
    OS_event event = {};
    
    event.eventType = EVENT_KEY;
    event.keyCode = keyCode;
    event.keyState = keyState; 
    
    return event;
}

internal OS_event
GetNextOSEvent(OS_event_list* list)
{
    ++list->readOffset;
    return list->events[list->readOffset - 1];
}

internal void
ClearEventList(OS_event_list* list)
{
    list->writeOffset = 0;
    list->readOffset = 0;
    
    MemorySet(list->events, 0, list->size * sizeof(OS_event));
}
