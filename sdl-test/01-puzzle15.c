#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const static int cellWidth = 50;
const static int boardX = 50;
const static int boardY = 50;
static int boardState[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0};

static void tileNumToRect(SDL_Rect* rc, int tileNum)
{
    rc->x = (tileNum % 4) * cellWidth;
    rc->y = (tileNum / 4) * cellWidth;
    rc->w = cellWidth;
    rc->h = cellWidth;
}

static void prepareSprite(SDL_Renderer* rend, SDL_Texture* sprite)
{
    TTF_Font* font = TTF_OpenFont(TTF_PATH, cellWidth / 2);
    const SDL_Color numColor = {0xFF, 0xFF, 0xFF, 0xFF};

    for(int num = 0; num < 16; num ++)
    {
        SDL_Rect rc;
        tileNumToRect(&rc, num);

        SDL_SetRenderDrawColor(rend, 0, 0xFF, 0x00, 0xFF);
        SDL_SetRenderTarget(rend, sprite);
        SDL_RenderDrawRect(rend, &rc);

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

    TTF_CloseFont(font);
}

static void copyTile(SDL_Renderer* rend, SDL_Texture* sprite, int idxTile, int posBoard)
{
    SDL_Rect src, dst;
    tileNumToRect(&src, idxTile);
    tileNumToRect(&dst, posBoard);
    dst.x += boardX;
    dst.y += boardY;
    SDL_RenderCopy(rend, sprite, &src, &dst);
}

static void drawBoard(SDL_Renderer* rend, SDL_Texture* sprite)
{
    SDL_SetRenderTarget(rend, NULL);
    SDL_SetRenderDrawColor(rend, 0x55, 0x55, 0x55, 0xFF);
    SDL_RenderClear(rend);
    for(int num = 0; num < 16; num ++)
        copyTile(rend, sprite, boardState[num], num);
}

static int numFromCoord(int x, int y)
{
    x -= boardX;
    y -= boardY;
    if(0 > x || 0 > y)
        return -1;

    int num = (x / cellWidth) + 4 * (y / cellWidth);

    if(15 < num)
        return -1;

    return num;
}

static void swapTile(SDL_Renderer* rend, SDL_Texture* sprite, int num)
{
    int numEmpty = 0;
    while(boardState[numEmpty])
        numEmpty ++;

    if(((num / 4 == numEmpty / 4) && (1 == abs(num - numEmpty)))
            || ((num % 4 == numEmpty % 4) && (4 == abs(num - numEmpty))))
    {
        int tmp = boardState[num];
        boardState[num] = boardState[numEmpty];
        boardState[numEmpty] = tmp;
    }
}

static void handleLeftClick(SDL_Renderer* rend, SDL_Texture* sprite, int x, int y)
{
    int numClicked = numFromCoord(x, y);
    if(0 > numClicked)
        return;

    swapTile(rend, sprite, numClicked);
}

static void shuffle(SDL_Renderer* rend, SDL_Texture* sprite)
{
    for(int i = 0; i < 10000; i ++)
        swapTile(rend, sprite, rand() % 16);
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

    SDL_bool running = SDL_TRUE;
    SDL_bool shuffling = SDL_FALSE;

    while(running)
    {
        SDL_Event evt;
        while(SDL_PollEvent(&evt))
        {
            if(SDL_QUIT == evt.type)
            {
                running = SDL_FALSE;
                continue;
            }
            if(SDL_MOUSEBUTTONDOWN == evt.type)
            {
                if(SDL_BUTTON_LEFT == evt.button.button)
                {
                    shuffling = SDL_FALSE;
                    handleLeftClick(rend, sprite, evt.button.x, evt.button.y);
                    continue;
                }
                if(SDL_BUTTON_RIGHT == evt.button.button)
                {
                    shuffling = !shuffling;
                    handleLeftClick(rend, sprite, evt.button.x, evt.button.y);
                    continue;
                }
            }
        }
        drawBoard(rend, sprite);
        SDL_RenderPresent(rend);

        if(shuffling)
            shuffle(rend, sprite);

        if(!shuffling)
            SDL_Delay(300);
    }

    SDL_DestroyTexture(sprite);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
