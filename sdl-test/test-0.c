#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>

static int running = 1;

int main(int argc, char *argv[])
{
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    int res = 0;
    SDL_Window* win = 0;
    SDL_Renderer* ren = 0;
    TTF_Font* font = 0;
    SDL_Texture* texture = 0;

    do {
        if(TTF_Init() != 0)
        {
            fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
            res = 1;
            break;
        }

        win = SDL_CreateWindow("SDL test"
        , SDL_WINDOWPOS_CENTERED
        , SDL_WINDOWPOS_CENTERED
        , 640
        , 480
        , SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        if(!win)
        {
            fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
            res = 2;
            break;
        }

        ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if(!ren)
        {
            fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
            res = 3;
            break;
        }

        font = TTF_OpenFont(TTF_PATH, 24);
        if(!font)
        {
            fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
            res = 4;
            break;
        }

        SDL_Color color = {255, 0, 0, 128}; // white color
        SDL_Surface* surface = TTF_RenderText_Solid(font, "Hello, World!", color);
        if(!surface)
        {
            fprintf(stderr, "TTF_RenderText_Solid Error: %s\n", TTF_GetError());
            res = 5;
            break;
        }

        texture = SDL_CreateTextureFromSurface(ren, surface);
        SDL_FreeSurface(surface);
        if(texture == NULL)
        {
            fprintf(stderr, "SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
            res = 6;
            break;
        }

        SDL_Rect dst;
        dst.x = 100;
        dst.y = 100;
        TTF_SizeText(font, "Hello, World!", &dst.w, &dst.h);

        while(running)
        {
            SDL_Event e;
            while(SDL_PollEvent(&e))
            {
                if(SDL_QUIT == e.type)
                    running = 0;
            }

            SDL_RenderClear(ren);
            SDL_RenderCopy(ren, texture, NULL, NULL);
            SDL_RenderPresent(ren);
            SDL_Delay(100);
        }
    } while(0);

    if(texture)
        SDL_DestroyTexture(texture);
    if(font)
        TTF_CloseFont(font);
    if(ren)
        SDL_DestroyRenderer(ren);
    if(win)
        SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    
    return 0;
}
