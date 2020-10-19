
//whatever events you want to handle
internal void
ProcessOSMessages(game_state* gameState)
{
    OS_event event = {0};
    event = GetNextOSEvent(&gameState->eventList);
    
    while(event.eventType)
    {
        switch(event.eventType)
        {
            case WINDOW_CLOSE:
            {
                gameState->isRunning = 0;
            } break;
            case EVENT_KEY:
            {
                switch(event.keyState)
                {
#define Key(keyID) case KEY_##keyID: \
{ \
gameState->keyData[KEY_##keyID].isDown = 1;\
} break;
                    case KEY_PRESS:
                    {
                        switch(event.keyCode)
                        {
#include "ADarkEngine/key_list.inc"
                            default:
                            {
                            } break;
                        }
                    } break;
#undef Key
#define Key(keyID) case KEY_##keyID: \
{ \
gameState->keyData[KEY_##keyID].isDown = 0;\
} break;
                    case KEY_RELEASE:
                    {
                        switch(event.keyCode)
                        {
#include "ADarkEngine/key_list.inc"
                            default:
                            {
                            } break;
                        }
                    } break;
#undef Key
                }
            } break;
            default:
            {
                // TODO(winston): logging
            } break;
        };
        event = GetNextOSEvent(&gameState->eventList);
    }
    
    ClearEventList(&gameState->eventList);
}
