#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

#define PI				3.14159265358979323846

static SDL_Window* win = NULL;
static SDL_Renderer* rend = NULL;
static SDL_Texture* sprite = NULL;

const static int cellWidth = 50;
static int board[4][4] =
{
    {1, 2, 3, 4},
    {5, 6, 7, 8},
    {9, 10, 11, 12},
    {13, 14, 15, 0}
};

static void (*idle_proc)();
static void (*mouse_down)(SDL_Event* evt);

static void play_idle();
static void play_mouse_down(SDL_Event* evt);
static void shuffle_idle();
static void shuffle_mouse_down(SDL_Event* evt);
static void victory_idle();
static void victory_mouse_down(SDL_Event* evt);

static void set_play_mode()
{
    idle_proc = play_idle;
    mouse_down = play_mouse_down;
}

static void set_shuffle_mode()
{
    idle_proc = shuffle_idle;
    mouse_down = shuffle_mouse_down;
}

static void set_victory_mode()
{
    idle_proc = victory_idle;
    mouse_down = victory_mouse_down;
}

static void tileNumToRect(SDL_FRect* rc, int x, int y)
{
    rc->x = x * cellWidth;
    rc->y = y * cellWidth;
    rc->w = cellWidth;
    rc->h = cellWidth;
}

static bool prepareSprite()
{
    TTF_Font* font = NULL;
    if(!TTF_Init()
            || !(font = TTF_OpenFont(TTF_PATH, cellWidth / 2)))
    {
        fprintf(stderr, "TTF_OpenFont Error: %s\n", SDL_GetError());
        TTF_Quit();
        return false;
    }
    const SDL_Color numColor = {0xFF, 0xFF, 0xFF, 0xFF};

    for(int y = 0; y < 4; y ++)
    {
        for(int x = 0; x < 4; x ++)
        {
            SDL_FRect rc;
            tileNumToRect(&rc, x, y);

            SDL_SetRenderDrawColor(rend, 0, 0xFF, 0x00, 0xFF);
            SDL_SetRenderTarget(rend, sprite);
            SDL_RenderRect(rend, &rc);

            int num = x + 4 * y;
            if(!num)
                continue;

            char str[8] = {0};

            if(num)
                sprintf(str, "%d", num);
            SDL_Surface* surface = TTF_RenderText_Blended(font, str, 0, numColor);
            SDL_Texture* tmpText = SDL_CreateTextureFromSurface(rend, surface);
            SDL_DestroySurface(surface);

            SDL_GetTextureSize(tmpText, &rc.w, &rc.h);
            rc.x += (cellWidth - rc.w) / 2;
            rc.y += (cellWidth - rc.h) / 2;
            SDL_SetRenderTarget(rend, sprite);
            SDL_RenderTexture(rend, tmpText, NULL, &rc);
            SDL_DestroyTexture(tmpText);
        }
    }

    TTF_CloseFont(font);
    TTF_Quit();
    return true;
}

static bool check_victory()
{
    int val = 0;
    for(int y = 0; y < 4; y ++)
    {
        for(int x = 0; x < 4; x ++)
        {
            if(0 == board[y][x])
                continue;
            if(val > board[y][x])
                return false;
            val = board[y][x];
        }
    }
    fprintf(stderr, "==>> won\n");
    return true;
}

static void swapTile(int x, int y)
{
    static int x_empty = 3;
    static int y_empty = 3;

    if(x_empty == x)
    {
        int y_dist = y - y_empty;
        int step = (y_dist > 0) - (y_dist < 0);
        for(; y_empty != y; y_empty += step)
        {
            board[y_empty][x] = board[y_empty + step][x];
            board[y_empty + step][x] = 0;
        }
        return;
    }
    if(y_empty == y)
    {
        int x_dist = x - x_empty;
        int step = (x_dist > 0) - (x_dist < 0);
        for(; x_empty != x; x_empty += step)
        {
            board[y][x_empty] = board[y][x_empty + step];
            board[y][x_empty + step] = 0;
        }
        return;
    }
}

static void draw_victory()
{
    static float osc_x[2] = {1.0, 1.0};
    static float osc_y[2] = {0.0, 0.0};
    SDL_SetRenderTarget(rend, NULL);
    SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0x00, 0xFF);
    SDL_FPoint buf5[2][5];
    SDL_FPoint star[2][6];
    SDL_RenderClear(rend);
    float f[2] = {2.0 * PI / 25.2, f[0] * 1.001};
    for(int j = 0; j < 5; j ++)
    {
        buf5[0][j].x = osc_x[0];
        buf5[0][j].y = osc_y[0];
        buf5[1][j].x = osc_y[1];
        buf5[1][j].y = osc_x[1];
        for(int i = 0; i < 5; i ++)
        {
            osc_x[0] += osc_y[0] * f[0];
            osc_y[0] -= osc_x[0] * f[0];
            osc_x[1] += osc_y[1] * f[1];
            osc_y[1] -= osc_x[1] * f[1];
        }
    }
    int idx_tbl_star[] = {0, 2, 4, 1, 3, 0};
    for(int i = 0; i < 5; i ++)
    {
        star[0][i].x = 100.0 + 50.0 * buf5[0][idx_tbl_star[i]].x;
        star[0][i].y = 100.0 + 50.0 * buf5[1][idx_tbl_star[i]].y;
    }
    star[0][5] = star[0][0];
    for(int i = 0; i < 5; i ++)
    {
        star[1][i].x = 30.0 + 25.0 * buf5[1][idx_tbl_star[i]].x;
        star[1][i].y = 30.0 + 25.0 * buf5[1][idx_tbl_star[i]].y;
    }
    star[1][5] = star[1][0];
    SDL_SetRenderDrawColor(rend, 0x00, 0xFF, 0x00, 0xFF);
    SDL_RenderLines(rend, star[0], 6);
    SDL_SetRenderDrawColor(rend, 0xFF, 0x00, 0x00, 0xFF);
    SDL_RenderLines(rend, star[1], 6);
    for(int i = 0; i < 6; i ++)
        star[1][i].x += 130.0;
    SDL_RenderLines(rend, star[1], 6);
    for(int i = 0; i < 6; i ++)
        star[1][i].y += 130.0;
    SDL_RenderLines(rend, star[1], 6);
    for(int i = 0; i < 6; i ++)
        star[1][i].x -= 130.0;
    SDL_RenderLines(rend, star[1], 6);
}

static void draw_board()
{
    SDL_SetRenderTarget(rend, NULL);
    SDL_SetRenderDrawColor(rend, 0x55, 0x55, 0x55, 0xFF);
    SDL_RenderClear(rend);
    for(int y = 0; y < 4; y ++)
    {
        for(int x = 0; x < 4; x ++)
        {
            SDL_FRect src, dst;
            tileNumToRect(&src, board[y][x] % 4, board[y][x] / 4);
            tileNumToRect(&dst, x, y);
            SDL_RenderTexture(rend, sprite, &src, &dst);
        }
    }
}

static void play_idle()
{
    draw_board();
    SDL_RenderPresent(rend);
    SDL_Delay(100);
}

static void play_mouse_down(SDL_Event* evt)
{
    if(SDL_BUTTON_LEFT == evt->button.button)
    {
        SDL_ConvertEventToRenderCoordinates(rend, evt);
        swapTile((int)(evt->button.x / cellWidth), (int)(evt->button.y / cellWidth));
        if(check_victory())
            set_victory_mode();
        return;
    }
    if(SDL_BUTTON_RIGHT == evt->button.button)
        set_shuffle_mode();
}

static void shuffle_idle()
{
    for(int i = 0; i < 10000; i ++)
        swapTile(rand() % 4, rand() % 4);
    draw_board();
    SDL_RenderPresent(rend);
}

static void shuffle_mouse_down(SDL_Event* evt)
{
    set_play_mode();
}

static void victory_idle()
{
    draw_victory();
    SDL_RenderPresent(rend);
}

static void victory_mouse_down(SDL_Event* evt)
{
    set_play_mode();
}

SDL_AppResult SDL_AppInit(void** app_context, int argc, char* argv[])
{
    if(!SDL_Init(SDL_INIT_VIDEO)
            || !SDL_CreateWindowAndRenderer("Puzzle-15"
                , 400
                , 400
                , SDL_WINDOW_RESIZABLE
                , &win
                , &rend)
            || !SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED)
            || !SDL_SetRenderVSync(rend, 1)
            || !SDL_SetRenderLogicalPresentation(rend, cellWidth * 4, cellWidth * 4, SDL_LOGICAL_PRESENTATION_STRETCH)
            || !(sprite = SDL_CreateTexture(rend
                    , SDL_PIXELFORMAT_RGBA8888
                    , SDL_TEXTUREACCESS_TARGET
                    , cellWidth * 4
                    , cellWidth * 4))
            || !prepareSprite())
    {
        fprintf(stderr, "Application initialization Error: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    srand(time(NULL));
    set_play_mode();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* app_context, SDL_Event* evt)
{
    if(SDL_EVENT_QUIT == evt->type)
        return SDL_APP_SUCCESS;

    if(SDL_EVENT_MOUSE_BUTTON_DOWN != evt->type)
        return SDL_APP_CONTINUE;
    mouse_down(evt);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* app_context)
{
    idle_proc();
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* app_context, SDL_AppResult result)
{
    SDL_DestroyTexture(sprite);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
}
