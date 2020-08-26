#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "chip8.h"
#include "chip8keyboard.h"
#include <stdbool.h>

// SDL_keyboard.h
const char keyboard_map[CHIP8_TOTAL_KEYS] = {
    SDLK_0, SDLK_1, SDLK_2, SDLK_3,
    SDLK_4, SDLK_5, SDLK_6, SDLK_7,
    SDLK_8, SDLK_9, SDLK_a, SDLK_b,
    SDLK_c, SDLK_d, SDLK_e, SDLK_f
};

int main(){
    struct chip8 chip8;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow(
        EMULATOR_WINDOW_TITLE, 
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        CHIP8_WIDTH * CHIP8_WINDOW_SCALE,
        CHIP8_HEIGHT * CHIP8_WINDOW_SCALE,
        SDL_WINDOW_SHOWN 
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_TEXTUREACCESS_TARGET);
    bool condition = true;

    while(condition){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    condition = false;
                    break;
                case SDL_KEYDOWN: {
                        char key = event.key.keysym.sym;
                        int vkey = chip8_keyboard_map(keyboard_map, key);
                        if (vkey != -1){
                            chip8_keyboard_down(&chip8.keyboard, vkey);
                            // printf("vkey: %x\n", vkey);
                        }
                    }
                    break;
                case SDL_KEYUP: {
                        char key = event.key.keysym.sym;
                        int vkey = chip8_keyboard_map(keyboard_map, key);
                        if (vkey != -1){
                            chip8_keyboard_up(&chip8.keyboard, vkey);
                        }
                    }
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR_R, 
                                         BACKGROUND_COLOR_G, 
                                         BACKGROUND_COLOR_B, 
                                         BACKGROUND_COLOR_A);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, FOREGROUND_COLOR_R,
                                         FOREGROUND_COLOR_G, 
                                         FOREGROUND_COLOR_B, 
                                         FOREGROUND_COLOR_A);
        SDL_Rect r;
        r.x = 50;
        r.y = 50;
        r.w = 40;
        r.h = 40;
        // SDL_RenderDrawRect(renderer, &r);
        SDL_RenderFillRect(renderer, &r);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    
    return 0;
}