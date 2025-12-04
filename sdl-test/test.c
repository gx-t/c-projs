// cc -g test.c -o test `pkg-config --cflags --libs sdl3`
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

int main(int argc, char *argv[])
{
  bool quit = false;
  SDL_Window *window = SDL_CreateWindow("Triangle Example", 800, 600, 0);

  SDL_Renderer *renderer = SDL_CreateGPURenderer(NULL, window);

  #define vertLen 3
  SDL_Vertex vert[vertLen];

  // center
  vert[0].position.x = 400;
  vert[0].position.y = 150;
  vert[0].color.r = 1.0;
  vert[0].color.g = 0.0;
  vert[0].color.b = 0.0;
  vert[0].color.a = 1.0;

  // left
  vert[1].position.x = 200;
  vert[1].position.y = 450;
  vert[1].color.r = 0.0;
  vert[1].color.g = 0.0;
  vert[1].color.b = 1.0;
  vert[1].color.a = 1.0;

  // right
  vert[2].position.x = 600;
  vert[2].position.y = 450;
  vert[2].color.r = 0.0;
  vert[2].color.g = 1.0;
  vert[2].color.b = 0.0;
  vert[2].color.a = 1.0;

  while (!quit) {
   SDL_Event ev;
   while (SDL_PollEvent(&ev) != 0) {
      switch(ev.type) {
        case SDL_EVENT_QUIT:
        quit = true;
        break;
      }
    }
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_RenderGeometry(renderer, NULL, vert, vertLen, NULL, 0);

    SDL_RenderPresent(renderer);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
