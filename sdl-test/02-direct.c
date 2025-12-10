#include <stdio.h>
#include <stdlib.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static SDL_Window* win = NULL;
static SDL_Renderer* rend = NULL;
static SDL_Texture* texture = NULL;

#define WINDOW_WIDTH    640
#define WINDOW_HEIGHT   480
#define BOARD_SIZE      200
#define CELL_SIZE       20
#define CELL_COUNT      ((BOARD_SIZE / CELL_SIZE) * (BOARD_SIZE / CELL_SIZE))

struct
{
    uint8_t clear_rgb[3];
    struct
    {
        uint8_t draw : 1;
        uint8_t rgb[3];
    }cells[CELL_COUNT];

    struct
    {
        int x;
        int w;
        int h;
        uint8_t rgb[3];
    }pad;

    struct
    {
        int x;
        int y;
        int r;
        uint8_t rgb[3];
    }ball;
}static scene;

static void init_board()
{
    scene.clear_rgb[0] = 0x33;
    scene.clear_rgb[1] = 0x33;
    scene.clear_rgb[2] = 0x33;

    int i = 0;
    for(; i < CELL_COUNT / 2; i ++)
    {
        scene.cells[i].draw = 1;
        scene.cells[i].rgb[0] = rand() % 200 + 55;
        scene.cells[i].rgb[1] = rand() % 200 + 55;
        scene.cells[i].rgb[2] = rand() % 200 + 55;
    }
    for(; i < CELL_COUNT; i ++)
    {
        scene.cells[i].draw = 0;
        scene.cells[i].rgb[0] = scene.clear_rgb[0];
        scene.cells[i].rgb[1] = scene.clear_rgb[1];
        scene.cells[i].rgb[2] = scene.clear_rgb[2];
    }
    scene.pad.x = BOARD_SIZE / 2;
    scene.pad.w = CELL_SIZE * 2;
    scene.pad.h = CELL_SIZE / 2;
    scene.pad.rgb[0] = 0xFF;
    scene.pad.rgb[1] = 0xFF;
    scene.pad.rgb[2] = 0xFF;
}

static void draw_board(void* fb, int pitch)
{
    uint8_t* row = fb;

    float mx, my;
    SDL_GetMouseState(&mx, &my);
    SDL_RenderCoordinatesFromWindow(rend, mx, my, &mx, &my);
    scene.pad.x = (int)mx;

    for(int y_board = 0; y_board < BOARD_SIZE; y_board ++)
    {
        int y_cell = y_board / CELL_SIZE;
        uint8_t* pp = (uint8_t*)row;
        for(int x_board = 0; x_board < BOARD_SIZE; x_board ++)
        {
            int x_cell = x_board / CELL_SIZE;
            int cell_idx = x_cell + y_cell * BOARD_SIZE / CELL_SIZE;

            // draw the cells
            if(scene.cells[cell_idx].draw && (y_board % CELL_SIZE) && (x_board % CELL_SIZE))
            {
                pp[0] = scene.cells[cell_idx].rgb[0];
                pp[1] = scene.cells[cell_idx].rgb[1];
                pp[2] = scene.cells[cell_idx].rgb[2];
                pp[3] = (x_board % CELL_SIZE) * 255 / CELL_SIZE;
            }

            // draw the pad
            if((y_board > BOARD_SIZE - scene.pad.h)
                    && x_board > scene.pad.x - scene.pad.w / 2
                    && x_board < scene.pad.x + scene.pad.w / 2)
            {
                pp[0] = scene.pad.rgb[0];
                pp[1] = scene.pad.rgb[1];
                pp[2] = scene.pad.rgb[2];
                pp[3] = 100 + ((x_board - scene.pad.x + scene.pad.w / 2) % scene.pad.w) * 155 / scene.pad.w;
            }
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
    SDL_SetRenderLogicalPresentation(rend, BOARD_SIZE, BOARD_SIZE, SDL_LOGICAL_PRESENTATION_STRETCH);
    SDL_SetRenderVSync(rend, 1);
    if(!(texture = SDL_CreateTexture(rend
                    , SDL_PIXELFORMAT_RGBA32
                    , SDL_TEXTUREACCESS_STREAMING
                    , BOARD_SIZE
                    , BOARD_SIZE)))
    {
        fprintf(stderr, "SDL_CreateTexture Error: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    srand(time(NULL));
    init_board();
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
    SDL_LockTexture(texture, NULL, &pixels, &pitch);
    draw_board(pixels, pitch);
    SDL_UnlockTexture(texture);

    SDL_SetRenderTarget(rend, NULL);
    SDL_SetRenderDrawColor(rend
            , scene.clear_rgb[0] = 0x33
            , scene.clear_rgb[1] = 0x33
            , scene.clear_rgb[2] = 0x33
            , 0xFF);

    SDL_RenderClear(rend);
    SDL_RenderTexture(rend, texture, NULL, NULL);
    SDL_RenderPresent(rend);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* app_context, SDL_AppResult result)
{
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
}

