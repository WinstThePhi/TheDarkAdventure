#include <stdlib.h>
#include <math.h>

#include "ADarkEngine/core/ADarkEngine_renderer.h"

#define BYTES_PER_PIXEL 4

// NOTE(winston): software rendering
inline void
DE_DrawPixel(void* temp)
{
    pixel_render_group* renderGroup = (pixel_render_group*)temp;
    
    u32* pixel = (u32*)((char*)renderGroup->backBuffer->memory + (renderGroup->x * BYTES_PER_PIXEL) + (renderGroup->backBuffer->pitch * renderGroup->y));
    
    *pixel = (renderGroup->color).r << 16 | (renderGroup->color).g << 8 | (renderGroup->color).b;
}

internal void*
DE_2d_FillBackground(void* temp)
{
    background_render_group* renderGroup = (background_render_group*)temp;
    
    u16 xMin = 0;
    u16 yMin = 0;
    
    u16 xMax = renderGroup->backBuffer->width;
    u16 yMax = renderGroup->backBuffer->height;
    
    if(xMin < 0)
        xMin = 0;
    if(yMin < 0)
        yMin = 0;
    if(xMax > renderGroup->backBuffer->width)
        xMax = renderGroup->backBuffer->width;
    if(yMax > renderGroup->backBuffer->height)
        yMax = renderGroup->backBuffer->height;
    
    u8* row = 
        (u8*)((u32*)renderGroup->backBuffer->memory + (yMin * renderGroup->backBuffer->pitch));
    
    for(u16 yCount = yMin; yCount < yMax; ++yCount)
    {
        u32* pixel = (u32*)((u32*)row + (u32)(xMin * BYTES_PER_PIXEL));
        
        for(u16 xCount = xMin; xCount < xMax; ++xCount)
        {
            *pixel++ = (renderGroup->color).r << 16 | (renderGroup->color).g << 8 | (renderGroup->color).b;
        }
        
        row += renderGroup->backBuffer->pitch;
    }
    
    return 0;
}

internal void*
DE_2d_DrawRectangle(void* temp)
{
    rect_render_group* renderGroup = (rect_render_group*)temp;
    
    u16 xMin = renderGroup->x;
    u16 yMin = renderGroup->y;
    
    u16 xMax = renderGroup->x + renderGroup->width;
    u16 yMax = renderGroup->y + renderGroup->height;
    
    if(xMin < 0)
        xMin = 0;
    if(yMin < 0)
        yMin = 0;
    if(xMax > renderGroup->backBuffer->width)
        xMax = renderGroup->backBuffer->width;
    if(yMax > renderGroup->backBuffer->height)
        yMax = renderGroup->backBuffer->height;
    
    u8* row = 
        (u8*)((u8*)renderGroup->backBuffer->memory + (yMin * renderGroup->backBuffer->pitch));
    
    for(u16 yCount = yMin; yCount < yMax; ++yCount)
    {
        u32* pixel = (u32*)(row + (xMin * BYTES_PER_PIXEL));
        
        for(u16 xCount = xMin; xCount < xMax; ++xCount)
        {
            *pixel++ = renderGroup->color.r << 16 | renderGroup->color.g << 8 | renderGroup->color.b;
        }
        
        row += renderGroup->backBuffer->pitch;
    }
    
    return 0;
}


internal void
DE_2d_DrawRectangleOutline(back_buffer* backBuffer,
                           u16 x, u16 y,
                           u16 width, u16 height,
                           v3 color)
{
    u16 xMin = x;
    u16 yMin = y;
    
    u16 xMax = x + width;
    u16 yMax = y + height;
    
    if(xMin < 0)
        xMin = 0;
    if(yMin < 0)
        yMin = 0;
    if(xMax > backBuffer->width)
        xMax = backBuffer->width;
    if(yMax > backBuffer->height)
        yMax = backBuffer->height;
    
    u8* row = (u8*)((u8*)backBuffer->memory + (yMin * backBuffer->pitch));
    
    for(u16 yCount = yMin; yCount < yMax; ++yCount)
    {
        u32* pixel = (u32*)(row + (xMin * BYTES_PER_PIXEL));
        
        for(u16 xCount = xMin; xCount < xMax; ++xCount)
        {
            *pixel++ = color.r << 16 | color.g << 8 | color.b;
        }
        
        row += backBuffer->pitch;
    }
}

// TODO(winston): fix this to use new file read function
#if 0
// TODO(winston): pretty broken here
// NOTE(winston): Could be a read error
internal back_buffer
DE_LoadBMP(memory_arena* arena, 
           char* filename)
{
    back_buffer result = {0};
    
    char* readResult = DE_ReadFile(arena, filename);
    
    if(readResult)
    {
        bitmap_header* header = (bitmap_header*)readResult;
        
        Assert(header->fileType == 0x4d42);
        
        u32* pixels = (u32*)((u8*)readResult + header->bitmapOffset);
        u32 bytesPerPixel = header->bitsPerPixel / 8;
        
        result.bytesPerPixel = bytesPerPixel;
        
        i32 pitch = header->width * bytesPerPixel;
        
        result.memory = (void*)pixels;
        
        result.width = (u16)header->width;
        result.height = (u16)header->height;
        
        result.pitch = (u16)pitch;
    }
    Assert(result.memory);
    
    return result;
}
#endif

internal void
DE_PrintBackBuffer(back_buffer* backBuffer)
{
    if(backBuffer)
    {
        printf("Memory: %p\n", backBuffer->memory);
        printf("Pitch: %d\n", backBuffer->pitch);
        printf("Bytes Per Pixel: %d\n", backBuffer->bytesPerPixel);
        printf("Width: %d\n", backBuffer->width);
        printf("Height: %d\n", backBuffer->height);
    }
}

// TODO(winston): there is a grid-like pattern in the picture that is undesirable
internal void 
DE_2d_DrawBMP(back_buffer* dest, 
              back_buffer* src)
{
    Assert(dest);
    Assert(src);
    if(src->bytesPerPixel == 3)
    {
        u32 size = src->bytesPerPixel * src->width * src->height;
        
        u8* destRow = (u8*)dest->memory;
        u8* srcRow = (u8*)src->memory + (src->width * src->bytesPerPixel) + ((src->height - 1) * src->pitch);
        
        for(i32 y = 0; y < src->height; ++y)
        {
            u8* srcPixel = (u8*)srcRow;
            u32* destPixel = (u32*)destRow;
            
            for(i32 x = 0; x < src->width; ++x)
            {
                u8 blue = *srcPixel++;
                u8 green = *srcPixel++;
                u8 red = *srcPixel++;
                
                *destPixel++ = (red << 16) | (green << 8) | (blue);
            }
            
            srcRow -= src->pitch;
            destRow += dest->pitch;
        }
    }
}

#ifdef WEIRD
internal void
RenderWeirdGradient(back_buffer* backBuffer, u16 xOffset, u16 yOffset)
{
    u8* row = (u8*)backBuffer->memory;
    
    for(u16 y = 0; y < backBuffer->height; ++y)
    {
        u32* pixel = (u32*)row;
        
        for(u16 x = 0; x < backBuffer->width; ++x)
        {
            u8 green = (u8)(x + xOffset);
            u8 blue = (u8)(y + yOffset);
            *pixel++ = (green << 16) | blue;
        }
        
        row += backBuffer->pitch;
    }
}
#endif

//~ NOTE(winston): renderer queueing interface (pass in parameter storage arena)
internal void 
DE2d_PushSolidBackground(memory_arena* globalArena, 
                         worker_thread_queue* workerThreadQueue,  
                         back_buffer* backBuffer, 
                         v3 color)
{
    // TODO(winston): maybe change to use a custom stack allocator 
    background_render_group* renderGroup = 
        WT_PushStruct(&workerThreadQueue->parameterStorage, 
                      background_render_group);
    
    renderGroup->backBuffer = backBuffer;
    renderGroup->color = color;
    
    PushWorkQueue(globalArena, workerThreadQueue, 
                  DE_2d_FillBackground, (void*)renderGroup);
}

internal void 
DE2d_PushRectangle(memory_arena* globalArena, 
                   worker_thread_queue* workerThreadQueue, 
                   back_buffer* backBuffer, 
                   u16 x, u16 y, 
                   u16 width, u16 height, 
                   v3 color)
{
    // TODO(winston): maybe change to use a custom stack allocator 
    
    rect_render_group* renderGroup = WT_PushStruct(&workerThreadQueue->parameterStorage, 
                                                   rect_render_group);
    
    renderGroup->backBuffer = backBuffer;
    renderGroup->x = x;
    renderGroup->y = y;
    renderGroup->width = width;
    renderGroup->height = height;
    renderGroup->color = color;
    
    PushWorkQueue(globalArena, workerThreadQueue, 
                  DE_2d_DrawRectangle, (void*)renderGroup);
}