#include <stdio.h>
#include <stdlib.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
//#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_main.h>

static SDL_Window* win = NULL;
static SDL_Renderer* rend = NULL;
static SDL_Texture* tex = NULL;

#define WIDTH       640
#define HEIGHT      480

SDL_AppResult SDL_AppInit(void** app_context, int argc, char* argv[])
{
    SDL_SetAppMetadata("SDL3 test: ditect access to texture data", "0.0", "shah32768.sdf.org");
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

//    SDL_GPUDevice *gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_METALLIB, true, NULL);
    if(!SDL_CreateWindowAndRenderer("SDL test"
                , WIDTH
                , HEIGHT
                , SDL_WINDOW_RESIZABLE
                , &win
                , &rend))
    {
        fprintf(stderr, "SDL_CreateWindowAndRenderer Error: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_SetRenderTarget(rend, NULL);
    SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0x00, 0xFF);
    SDL_SetRenderLogicalPresentation(rend, WIDTH / 10, HEIGHT / 10, SDL_LOGICAL_PRESENTATION_STRETCH);
    SDL_SetRenderVSync(rend, 1);
    if(!(tex = SDL_CreateTexture(rend
                    , SDL_PIXELFORMAT_RGBA32
                    , SDL_TEXTUREACCESS_STREAMING
                    , WIDTH / 10
                    , HEIGHT / 10)))
    {
        fprintf(stderr, "SDL_CreateTexture Error: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    void* pixels = NULL;
    int pitch = 0;
    SDL_LockTexture(tex, NULL, &pixels, &pitch);
    uint8_t* row = (uint8_t*)pixels;
    for(int y = 0; y < HEIGHT / 10; y ++)
    {
        uint8_t* pp = (uint8_t*)row;
        for(int x = 0; x < WIDTH / 10; x ++)
        {
            *pp ++ = rand() % 0xFF;
            *pp ++ = rand() % 0xFF;
            *pp ++ = rand() % 0xFF;
            *pp ++ = rand() % 0xFF;
        }
        row += pitch;
    }
    SDL_UnlockTexture(tex);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* app_context, SDL_Event* evt)
{
    if(SDL_EVENT_QUIT == evt->type)
        return SDL_APP_SUCCESS;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* app_context)
{
//    void* pixels = NULL;
//    int pitch = 0;
//
//    SDL_LockTexture(tex, NULL, &pixels, &pitch);
//    uint64_t begin = SDL_GetPerformanceCounter();
//    uint8_t* row = (uint8_t*)pixels;
//    for(int y = 0; y < HEIGHT; y ++)
//    {
//        uint8_t* pp = (uint8_t*)row;
//        for(int x = 0; x < WIDTH; x ++)
//        {
//            *pp ++ = rand() % 0xFF;
//            *pp ++ = rand() % 0xFF;
//            *pp ++ = rand() % 0xFF;
//            *pp ++ = rand() % 0xFF;
//        }
//        row += pitch;
//    }
//    uint64_t end = SDL_GetPerformanceCounter();
//    double ms = (double)(end - begin) * 1000.0 / SDL_GetPerformanceFrequency();
//
//    SDL_UnlockTexture(tex);

//    SDL_RenderClear(rend);

    SDL_RenderTexture(rend, tex, NULL, NULL);

    SDL_RenderPresent(rend);
//    fprintf(stderr, "==>> %g\n", ms);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* app_context, SDL_AppResult result)
{
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
}

