#include <stdlib.h>
#include <time.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define VIEW_WIDTH      400
#define VIEW_HEIGHT     400
#define ATLAS_WIDTH     256
#define ATLAS_HEIGHT    256

static SDL_Window* win = NULL;
static SDL_Renderer* rend = NULL;
static SDL_Texture* atlas = NULL;

void fill_atlas(SDL_Surface* surface)
{
    Uint32* pixels = (Uint32*)surface->pixels;
    int pitch = surface->pitch / sizeof(*pixels);

    int center_x = ATLAS_WIDTH / 2;
    int center_y = ATLAS_HEIGHT / 2;
    for(int y = 0; y < ATLAS_HEIGHT; y++)
    {
        for(int x = 0; x < ATLAS_WIDTH; x++)
        {
            Uint8 val = (center_x * center_y - x * x + y * y) / 128;
            pixels[y * pitch + x] = (val << 24) | (val << 16) | (val << 8) | val;
        }
    }
}

static bool init_atlas()
{
    SDL_Surface* surface = SDL_CreateSurface(ATLAS_WIDTH, ATLAS_HEIGHT, SDL_PIXELFORMAT_RGBA8888);
    if(!surface)
        return false;
    fill_atlas(surface);
    atlas = SDL_CreateTextureFromSurface(rend, surface);
    SDL_DestroySurface(surface);
    if(!atlas)
        return false;

    return true;
}

SDL_AppResult SDL_AppInit(void** app_context, int argc, char* argv[])
{
    if(!SDL_Init(SDL_INIT_VIDEO)
            || !SDL_CreateWindowAndRenderer("Atlas test"
                , 400
                , 400
                , SDL_WINDOW_RESIZABLE
                , &win
                , &rend)
            || !SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED)
            || !SDL_SetRenderVSync(rend, 1)
            || !SDL_SetRenderLogicalPresentation(rend, VIEW_WIDTH, VIEW_HEIGHT, SDL_LOGICAL_PRESENTATION_STRETCH)
            || !init_atlas())
    {
        SDL_Log("Application initialization Error: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
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
    SDL_SetRenderTarget(rend, NULL);
    SDL_SetRenderDrawColorFloat(rend, 0.5, 0.5, 0.5, 1.0);
    SDL_RenderClear(rend);
    SDL_RenderTexture(rend, atlas, NULL, NULL); //for test
    SDL_RenderPresent(rend);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* app_context, SDL_AppResult result)
{
    SDL_DestroyTexture(atlas);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
}
