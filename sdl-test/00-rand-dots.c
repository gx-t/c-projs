#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>

static int running = 1;

int main(int argc, char *argv[])
{
    int res = 0;
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* win = 0;
    SDL_Renderer* rend = 0;
    SDL_Texture* texture = 0;

    do {
        if(!(win = SDL_CreateWindow("SDL test"
                        , 640
                        , 480
                        , SDL_WINDOW_RESIZABLE)))
        {
            fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
            res = 2;
            break;
        }
        SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

        if(!(rend = SDL_CreateRenderer(win, NULL)))
        {
            fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
            res = 3;
            break;
        }
        SDL_SetRenderVSync(rend, 1);
        if(!(texture = SDL_CreateTexture(rend
                        , SDL_PIXELFORMAT_RGBA8888
                        , SDL_TEXTUREACCESS_TARGET
                        , 64
                        , 48)))
        {
            fprintf(stderr, "SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
            res = 6;
            break;
        }

        while(running)
        {
            SDL_Event e;
            while(SDL_PollEvent(&e))
            {
                if(SDL_EVENT_QUIT == e.type)
                    running = 0;
            }

            int count = 8;
            const SDL_Color colorArr[] =
            {
                {0xFF, 0x00, 0x00, 0xFF}
                , {0x00, 0xFF, 0x00, 0xFF}
                , {0x00, 0x00, 0xFF, 0xFF}
                , {0xFF, 0xFF, 0x00, 0xFF}
                , {0xFF, 0x00, 0xFF, 0xFF}
                , {0x00, 0xFF, 0xFF, 0xFF}
                , {0xFF, 0xFF, 0xFF, 0xFF}
            };
            SDL_SetRenderTarget(rend, texture);
            SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0x00, 0xFF);
            SDL_RenderClear(rend);
            SDL_SetRenderScale(rend, 1.0, 1.0);
            while(count --)
            {
                const SDL_Color* clr = &colorArr[rand() % sizeof(colorArr) / sizeof(colorArr[0])];
                SDL_SetRenderDrawColor(rend, clr->r, clr->g, clr->b, clr->a);
                SDL_RenderPoint(rend, rand() % 64, rand() % 48);
            }

            //            SDL_RenderDrawPoints(rend, pointArr, sizeof(pointArr) / sizeof(pointArr[0]));
            SDL_SetRenderTarget(rend, NULL);

            SDL_RenderClear(rend);
            SDL_RenderTexture(rend, texture, NULL, NULL);
            SDL_RenderPresent(rend);
            SDL_Delay(50);
        }
    } while(0);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return res;
}
