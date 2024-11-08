#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static SDL_bool running = SDL_TRUE;

const static int cellWidth = 50;
const static int boardX = 50;
const static int boardY = 50;
static int board[4][4] =
{
    {1, 2, 3, 4},
    {5, 6, 7, 8},
    {9, 10, 11, 12},
    {13, 14, 15, 0}
};

static int x_empty = 3;
static int y_empty = 3;

static void tileNumToRect(SDL_Rect* rc, int x, int y)
{
    rc->x = x * cellWidth;
    rc->y = y * cellWidth;
    rc->w = cellWidth;
    rc->h = cellWidth;
}

static void prepareSprite(SDL_Renderer* rend, SDL_Texture* sprite)
{
    TTF_Font* font = TTF_OpenFont(TTF_PATH, cellWidth / 2);
    const SDL_Color numColor = {0xFF, 0xFF, 0xFF, 0xFF};

    for(int y = 0; y < 4; y ++)
    {
        for(int x = 0; x < 4; x ++)
        {
            SDL_Rect rc;
            tileNumToRect(&rc, x, y);

            SDL_SetRenderDrawColor(rend, 0, 0xFF, 0x00, 0xFF);
            SDL_SetRenderTarget(rend, sprite);
            SDL_RenderDrawRect(rend, &rc);

            int num = x + 4 * y;
            if(!num)
                continue;

            char str[8] = {0};

            if(num)
                sprintf(str, "%d", num);
            SDL_Surface* surface = TTF_RenderText_Blended(font, str, numColor);
            SDL_Texture* tmpText = SDL_CreateTextureFromSurface(rend, surface);
            SDL_FreeSurface(surface);

            SDL_QueryTexture(tmpText, NULL, NULL, &rc.w, &rc.h);
            rc.x += (cellWidth - rc.w) / 2;
            rc.y += (cellWidth - rc.h) / 2;
            SDL_SetRenderTarget(rend, sprite);
            SDL_RenderCopy(rend, tmpText, NULL, &rc);
            SDL_DestroyTexture(tmpText);
        }
    }

    TTF_CloseFont(font);
}

static void swapTile(SDL_Renderer* rend, SDL_Texture* sprite, int x, int y)
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

static void handleLeftClick(SDL_Renderer* rend, SDL_Texture* sprite, int x, int y)
{
    x -= boardX;
    y -= boardY;
    if(0 > x || 0 > y)
        return;

    x /= cellWidth;
    y /= cellWidth;

    swapTile(rend, sprite, x, y);
}

static void shuffle(SDL_Renderer* rend, SDL_Texture* sprite)
{
    for(int i = 0; i < 10000; i ++)
        swapTile(rend, sprite, rand() % 4, rand() % 4);
}

static void drawBoard(SDL_Renderer* rend, SDL_Texture* sprite)
{
    SDL_SetRenderTarget(rend, NULL);
    SDL_SetRenderDrawColor(rend, 0x55, 0x55, 0x55, 0xFF);
    SDL_RenderClear(rend);
    for(int y = 0; y < 4; y ++)
    {
        for(int x = 0; x < 4; x ++)
        {
            SDL_Rect src, dst;
            tileNumToRect(&src, board[y][x] % 4, board[y][x] / 4);
            tileNumToRect(&dst, x, y);
            dst.x += boardX;
            dst.y += boardY;
            SDL_RenderCopy(rend, sprite, &src, &dst);
        }
    }
    SDL_RenderPresent(rend);
}

static void shuffleLoop(SDL_Renderer* rend, SDL_Texture* sprite)
{
    while(running)
    {
        SDL_Event evt;
        while(SDL_PollEvent(&evt))
        {
            if(SDL_QUIT == evt.type)
            {
                running = SDL_FALSE;
                return;
            }
            if(SDL_MOUSEBUTTONDOWN == evt.type
                    && (SDL_BUTTON_LEFT == evt.button.button
                        || SDL_BUTTON_RIGHT == evt.button.button))
                return;
        }
        drawBoard(rend, sprite);
        shuffle(rend, sprite);
    }
}

static void mainScreenEventLoop(SDL_Renderer* rend, SDL_Texture* sprite)
{
    SDL_Event evt;
    while(running && SDL_WaitEvent(&evt))
    {
        switch(evt.type)
        {
            case SDL_QUIT:
                running = SDL_FALSE;
                break;
            case SDL_MOUSEBUTTONDOWN:
                switch(evt.button.button)
                {
                    case SDL_BUTTON_LEFT:
                        handleLeftClick(rend, sprite, evt.button.x, evt.button.y);
                        break;
                    case SDL_BUTTON_RIGHT:
                        shuffleLoop(rend, sprite);
                        break;
                }
                drawBoard(rend, sprite);
                break;
            case SDL_WINDOWEVENT:
                if(SDL_WINDOWEVENT_EXPOSED == evt.window.event)
                    drawBoard(rend, sprite);
                break;
        }
    }
}

int main()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    TTF_Init();
    SDL_Window* win = SDL_CreateWindow("Puzzle-15"
            , SDL_WINDOWPOS_CENTERED
            , SDL_WINDOWPOS_CENTERED
            , 640
            , 480
            , SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    SDL_Renderer* rend = SDL_CreateRenderer(win
            , -1
            , SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_Texture* sprite = SDL_CreateTexture(rend
            , SDL_PIXELFORMAT_RGBA8888
            , SDL_TEXTUREACCESS_TARGET
            , 200
            , 200);

    prepareSprite(rend, sprite);

    srand(time(NULL));

     mainScreenEventLoop(rend, sprite);

    SDL_DestroyTexture(sprite);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
