#include<stdio.h>
#include "SDL3/SDL.h"
#include "brand/var.h"

#define aspect getAspect();

SDL_Window *window;
SDL_Renderer *renderer;

int WindowWidth = 800;
int WindowHeight = 600;

SDL_FRect urlBar;

float getAspect() {
    return (float)WindowWidth / (float)1920;
}

void resizeWindow(const int newWidth, const int newHeight) {
    WindowWidth = newWidth;
    WindowHeight = newHeight;
    urlBar.w = newWidth;
    urlBar.h = newHeight / 6 * aspect;
}

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL Init failed: %s", SDL_GetError());
    }

    if (!SDL_CreateWindowAndRenderer("Katzi Browser" , WindowWidth, WindowHeight, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Create Window and Renderer failed: %s", SDL_GetError());
    }

    resizeWindow(800, 600);
    
    bool running = true;
    while (running) {

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                running = false;
            }

            if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                resizeWindow(e.window.data1, e.window.data2);
            }
        }

        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderFillRect(renderer, &urlBar);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);

    return 0;
}