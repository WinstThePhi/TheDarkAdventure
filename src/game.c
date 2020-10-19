
#define ERROR_LOG_PATH "../data/game_error.txt"

#include "ADarkEngine/ADarkEngine.h"
#include "game.h"

//~ NOTE(winston): runs when the game starts
START_GAME(Game_Start)
{
    DisableBuffering(stdout);
    DisableBuffering(stderr);
    
    gameState->fpsCap = 60;
    
    DE_ClearFile(ERROR_LOG_PATH);
}

//~ NOTE(winston): runs every frame
GAME_UPDATE_AND_RENDER(Game_UpdateAndRender)
{
#define GRAVITY -0.163f
#define SPEED 6
    {
        if(IsKeyDown(gameState, KEY_W))
        {
            gameState->y -= SPEED;
        }
        if(IsKeyDown(gameState, KEY_A))
        {
            gameState->x -= SPEED;
        }
        if(IsKeyDown(gameState, KEY_S))
        {
            gameState->y += SPEED;
        }
        if(IsKeyDown(gameState, KEY_D))
        {
            gameState->x += SPEED;
        }
    }
#undef SPEED
    
    DE2d_PushSolidBackground(renderGroup,
                             v3_color(0, 188, 255));
    
    DE2d_PushRectangle(renderGroup,
                       0, 550,
                       GetBackBufferWidth(renderGroup), 
                       GetBackBufferHeight(renderGroup) - 550,
                       v3_color(155, 118, 83));
    
    DE2d_PushRectangle(renderGroup,
                       gameState->x, gameState->y,
                       100, 100,
                       v3_color(255, 255, 255));
}

//~ NOTE(winston): Runs when unloaded or ended
END_GAME(Game_End)
{
    
}
