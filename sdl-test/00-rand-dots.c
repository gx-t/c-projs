#include <stdio.h>
#include <stdlib.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static SDL_Window* win = NULL;
static SDL_Renderer* rend = NULL;

SDL_AppResult SDL_AppInit(void** app_context, int argc, char* argv[])
{
    static const SDL_Color colorArr[] =
    {
        {0xFF, 0x00, 0x00, 0xFF}
        , {0x00, 0xFF, 0x00, 0xFF}
        , {0x00, 0x00, 0xFF, 0xFF}
        , {0xFF, 0xFF, 0x00, 0xFF}
        , {0xFF, 0x00, 0xFF, 0xFF}
        , {0x00, 0xFF, 0xFF, 0xFF}
        , {0xFF, 0xFF, 0xFF, 0xFF}
    };
    *app_context = (void*)colorArr;

    SDL_SetAppMetadata("SDL3 test: random dots", "0.0", "shah32768.sdf.org");
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    if(!SDL_CreateWindowAndRenderer("SDL test"
                , 640
                , 480
                , SDL_WINDOW_RESIZABLE
                , &win
                , &rend))
    {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_SetRenderLogicalPresentation(rend, 64, 48, SDL_LOGICAL_PRESENTATION_STRETCH);
    SDL_SetRenderVSync(rend, 1);
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
    const SDL_Color* color_arr = (const SDL_Color*)app_context;
    SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(rend);
    int count = 8;
    while(count --)
    {
        const SDL_Color* clr = &color_arr[rand() % 7];
        SDL_SetRenderDrawColor(rend, clr->r, clr->g, clr->b, clr->a);
        SDL_RenderPoint(rend, rand() % 64, rand() % 48);
    }
    SDL_RenderPresent(rend);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* app_context, SDL_AppResult result)
{
}

