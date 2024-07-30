#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>

static int running = 1;

static void draw_rand_points(SDL_Texture* texture, int count)
{
}

int main(int argc, char *argv[])
{
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    int res = 0;
    SDL_Window* win = 0;
    SDL_Renderer* rend = 0;
    TTF_Font* font = 0;
    SDL_Texture* texture = 0;

    do {
        if(TTF_Init() != 0)
        {
            fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
            res = 1;
            break;
        }

        if(!(win = SDL_CreateWindow("SDL test"
                        , SDL_WINDOWPOS_CENTERED
                        , SDL_WINDOWPOS_CENTERED
                        , 640
                        , 480
                        , SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)))
        {
            fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
            res = 2;
            break;
        }

        if(!(rend = SDL_CreateRenderer(win
                        , -1
                        , SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)))
        {
            fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
            res = 3;
            break;
        }

        if(!(font = TTF_OpenFont(TTF_PATH, 24)))
        {
            fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
            res = 4;
            break;
        }

        SDL_Color color = {255, 0, 0, 128};
//        SDL_Surface* surface = TTF_RenderText_Solid(font, "SDL Test", color);
//        if(!surface)
//        {
//            fprintf(stderr, "TTF_RenderText_Solid Error: %s\n", TTF_GetError());
//            res = 5;
//            break;
//        }
//
//        texture = SDL_CreateTextureFromSurface(rend, surface);
//        SDL_FreeSurface(surface);
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

//        SDL_Rect dst;
//        dst.x = 100;
//        dst.y = 100;
//        TTF_SizeText(font, "   ", &dst.w, &dst.h);


        while(running)
        {
            SDL_Event e;
            while(SDL_PollEvent(&e))
            {
                if(SDL_QUIT == e.type)
                    running = 0;
            }

//            SDL_Point pointArr[8] = {0};
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
            SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0x00, 0x00);
            SDL_RenderClear(rend);
            SDL_RenderSetScale(rend, 1, 1);
            while(count --)
            {
//                pointArr[count].x = rand() % 16;
//                pointArr[count].y = rand() % 16;
                const SDL_Color* clr = &colorArr[rand() % sizeof(colorArr) / sizeof(colorArr[0])];
                SDL_SetRenderDrawColor(rend, clr->r, clr->g, clr->b, clr->a);
                SDL_RenderDrawPoint(rend, rand() % 64, rand() % 48);
            }

//            SDL_SetRenderDrawColor(rend, 0xFF, 0xFF, 0xFF, 0xFF);
//            SDL_RenderDrawPoints(rend, pointArr, sizeof(pointArr) / sizeof(pointArr[0]));
            SDL_SetRenderTarget(rend, NULL);

            SDL_RenderClear(rend);
            SDL_RenderCopy(rend, texture, NULL, NULL);
            SDL_RenderPresent(rend);
            SDL_Delay(50);
        }
    } while(0);

    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    
    return 0;
}
