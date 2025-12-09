#include <stdio.h>
#include <stdlib.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static SDL_Window* win = NULL;
static SDL_Renderer* rend = NULL;
static SDL_Texture* tex = NULL;

#define WINDOW_WIDTH    640
#define WINDOW_HEIGHT   480
#define BOARD_WIDTH     200
#define BOARD_HEIGHT    200
#define BREAK_WIDTH     20
#define BREAK_HEIGHT    20
#define COLOR_COUNT     6

static const uint8_t empty_color[] = {0x33, 0x33, 0x33, 0xFF};
static uint8_t color_tbl[COLOR_COUNT][3];

static void init_color_tbl()
{
    for(int i = 0; i < COLOR_COUNT; i ++)
    {
        color_tbl[i][0] = rand() % 200 + 55;
        color_tbl[i][1] = rand() % 200 + 55;
        color_tbl[i][2] = rand() % 200 + 55;
    }
};


static void draw_board(void* fb, int pitch)
{
    uint8_t* row = fb;
    for(int y = 0; y < BOARD_HEIGHT / 2; y ++)
    {
        uint8_t* pp = (uint8_t*)row;
        for(int x = 0; x < BOARD_WIDTH; x ++)
        {
            if((y % BREAK_HEIGHT) && (x % BREAK_WIDTH))
            {
                *pp ++ = color_tbl[(x / BREAK_WIDTH + y / BREAK_HEIGHT) % COLOR_COUNT][0];
                *pp ++ = color_tbl[(x / BREAK_WIDTH + y / BREAK_HEIGHT) % COLOR_COUNT][1];
                *pp ++ = color_tbl[(x / BREAK_WIDTH + y / BREAK_HEIGHT) % COLOR_COUNT][2];
                *pp ++ = rand() % 100 + 155;
            }
            else
            {
                *(uint32_t*)pp = *(uint32_t*)empty_color;
                pp += 4;
            }
        }
        row += pitch;
    }
    for(int y = BOARD_HEIGHT / 2; y < BOARD_HEIGHT; y ++)
    {
        uint8_t* pp = (uint8_t*)row;
        for(int x = 0; x < BOARD_WIDTH; x ++)
        {
            *(uint32_t*)pp = *(uint32_t*)empty_color;
            pp += 4;
        }
        row += pitch;
    }
}

SDL_AppResult SDL_AppInit(void** app_context, int argc, char* argv[])
{
    SDL_SetAppMetadata("SDL3 test: ditect access to texture data", "0.0", "shah32768.sdf.org");
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if(!SDL_CreateWindowAndRenderer("SDL test"
                , WINDOW_WIDTH
                , WINDOW_HEIGHT
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
    SDL_SetRenderLogicalPresentation(rend, BOARD_WIDTH, BOARD_HEIGHT, SDL_LOGICAL_PRESENTATION_STRETCH);
    SDL_SetRenderVSync(rend, 1);
    if(!(tex = SDL_CreateTexture(rend
                    , SDL_PIXELFORMAT_RGBA32
                    , SDL_TEXTUREACCESS_STREAMING
                    , BOARD_WIDTH
                    , BOARD_HEIGHT)))
    {
        fprintf(stderr, "SDL_CreateTexture Error: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    srand(time(NULL));
    init_color_tbl();
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

    void* pixels = NULL;
    int pitch = 0;
    SDL_LockTexture(tex, NULL, &pixels, &pitch);
    draw_board(pixels, pitch);
    SDL_UnlockTexture(tex);

    SDL_RenderTexture(rend, tex, NULL, NULL);
    SDL_RenderPresent(rend);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* app_context, SDL_AppResult result)
{
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
}

