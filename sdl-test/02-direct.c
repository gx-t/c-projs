#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static SDL_Window* win = NULL;
static SDL_Renderer* rend = NULL;
static SDL_Texture* texture = NULL;

#define WINDOW_WIDTH    640
#define WINDOW_HEIGHT   480
#define BOARD_SIZE      201
#define CELL_SIZE       20
#define CELL_COUNT      ((BOARD_SIZE / CELL_SIZE) * (BOARD_SIZE / CELL_SIZE))

#define ACCELERATION    0.1f

struct
{
    uint8_t clear_rgba[4];
    uint8_t border_rgba[4];
    struct
    {
        uint8_t draw : 1;
        uint8_t rgba[4];
    }cells[CELL_COUNT];

    struct
    {
        int x;
        int w;
        int h;
        uint8_t rgba[4];
    }pad;

    struct
    {
        float x;
        float y;
        int r;
        float vx;
        float vy;
        uint8_t rgba[4];
    }ball;
}static scene;

static void init_board()
{
    scene.clear_rgba[0] = 0x33;
    scene.clear_rgba[1] = 0x33;
    scene.clear_rgba[2] = 0x33;
    scene.clear_rgba[3] = 0xFF;

    scene.border_rgba[0] = 0x00;
    scene.border_rgba[1] = 0xFF;
    scene.border_rgba[2] = 0x00;
    scene.border_rgba[3] = 0xFF;

    int i = 0;
    for(; i < CELL_COUNT / 2; i ++)
    {
        scene.cells[i].draw = 1;
        scene.cells[i].rgba[0] = rand() % 200 + 55;
        scene.cells[i].rgba[1] = rand() % 200 + 55;
        scene.cells[i].rgba[2] = rand() % 200 + 55;
        scene.cells[i].rgba[3] = 0xFF;
        if(*(uint32_t*)scene.cells[i].rgba == *(uint32_t*)scene.clear_rgba)
            i --;
    }
    for(; i < CELL_COUNT; i ++)
    {
        scene.cells[i].draw = 0;
        *(uint32_t*)scene.cells[i].rgba = *(uint32_t*)scene.clear_rgba;
    }
    scene.pad.x = BOARD_SIZE / 2;
    scene.pad.w = CELL_SIZE * 2;
    scene.pad.h = CELL_SIZE / 2;
    scene.pad.rgba[0] = 0xFF;
    scene.pad.rgba[1] = 0xFF;
    scene.pad.rgba[2] = 0xFF;
    scene.pad.rgba[3] = 0xFF;

    scene.ball.x = 100;
    scene.ball.y = 150;
    scene.ball.r = 5;
    scene.ball.vx = 0.6;
    scene.ball.vy = 0.6;
    scene.ball.rgba[0] = 0xFF;
    scene.ball.rgba[1] = 0xFF;
    scene.ball.rgba[2] = 0x77;
    scene.ball.rgba[3] = 0xFF;
}

static float clamp_f(float v)
{
    if(v > 2.0)
        return 2.0;
    if(v < -2.0)
        return -2.0;
    return v;
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
            *(uint32_t*)pp = *(uint32_t*)scene.clear_rgba;
            if(0 == x_board || 0 == y_board || BOARD_SIZE - 1 == x_board || BOARD_SIZE - 1 == y_board)
                *(uint32_t*)pp = *(uint32_t*)scene.border_rgba;

            int x_cell = x_board / CELL_SIZE;
            int cell_idx = x_cell + y_cell * BOARD_SIZE / CELL_SIZE;
            // draw the cells
            if(scene.cells[cell_idx].draw && (y_board % CELL_SIZE) && (x_board % CELL_SIZE))
            {
                *(uint32_t*)pp = *(uint32_t*)scene.cells[cell_idx].rgba;
                pp[3] = (x_board % CELL_SIZE) * 255 / CELL_SIZE;
            }

            // draw the pad
            if((y_board > BOARD_SIZE - scene.pad.h)
                    && x_board > scene.pad.x - scene.pad.w / 2
                    && x_board < scene.pad.x + scene.pad.w / 2)
            {
                *(uint32_t*)pp = *(uint32_t*)scene.pad.rgba;
                pp[3] = 100 + ((x_board - scene.pad.x + scene.pad.w / 2) % scene.pad.w) * 155 / scene.pad.w;
            }

            // draw the ball
            float dx = x_board - scene.ball.x;
            float dy = y_board - scene.ball.y;
            if((dx * dx + dy * dy) < scene.ball.r * scene.ball.r)
            {
                if(*(uint32_t*)pp != *(uint32_t*)scene.clear_rgba) //collision
                {
                    scene.ball.vx -= dx * ACCELERATION;
                    scene.ball.vy -= dy * ACCELERATION;
                    scene.ball.vx = clamp_f(scene.ball.vx);
                    scene.ball.vy = clamp_f(scene.ball.vy);
                }
                else
                    *(uint32_t*)pp = *(uint32_t*)scene.ball.rgba;
            }
            pp += 4;
        }
        row += pitch;
    }
    scene.ball.x += scene.ball.vx;
    scene.ball.y += scene.ball.vy;
}

SDL_AppResult SDL_AppInit(void** app_context, int argc, char* argv[])
{
    SDL_SetAppMetadata("SDL3 test: direct access to texture data", "0.0", "shah32768.sdf.org");
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

