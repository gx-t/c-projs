#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

enum
{
    STATE_MAIN
    , STATE_SHUFFLE
}static state = STATE_MAIN;

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

static int x_empty = 3;
static int y_empty = 3;

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

static void check_if_won()
{
    int val = 0;
    for(int y = 0; y < 4; y ++)
    {
        for(int x = 0; x < 4; x ++)
        {
            if(0 == board[y][x])
                continue;
            if(val > board[y][x])
                return;
            val = board[y][x];
        }
    }
    fprintf(stderr, "==>> won\n");
}

static void swapTile(int x, int y)
{
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

static void shuffle()
{
    for(int i = 0; i < 10000; i ++)
        swapTile(rand() % 4, rand() % 4);
}

static void drawBoard()
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
    SDL_RenderPresent(rend);
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
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* app_context, SDL_Event* evt)
{
    if(SDL_EVENT_QUIT == evt->type)
        return SDL_APP_SUCCESS;

    if(SDL_EVENT_MOUSE_BUTTON_DOWN != evt->type)
        return SDL_APP_CONTINUE;
    if(STATE_SHUFFLE == state)
    {
        state = STATE_MAIN;
        return SDL_APP_CONTINUE;
    }
    if(SDL_BUTTON_LEFT == evt->button.button)
    {
        SDL_ConvertEventToRenderCoordinates(rend, evt);
        swapTile((int)(evt->button.x / cellWidth), (int)(evt->button.y / cellWidth));
        check_if_won();
        return SDL_APP_CONTINUE;
    }

    if(SDL_BUTTON_RIGHT == evt->button.button)
        state = STATE_SHUFFLE;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* app_context)
{
    if(STATE_SHUFFLE == state)
        shuffle();

    drawBoard();

    if(STATE_MAIN == state)
        SDL_WaitEvent(NULL);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* app_context, SDL_AppResult result)
{
}
