#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "SDL2/SDL.h"
#include "beep.h"
#include "chip8.h"
#include "chip8keyboard.h"


const char keyboard_map[CHIP8_TOTAL_KEYS] = {
    SDLK_0, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5,
    SDLK_6, SDLK_7, SDLK_8, SDLK_9, SDLK_a, SDLK_b,
    SDLK_c, SDLK_d, SDLK_e, SDLK_f};

int main(int argc, char **argv){
    struct chip8 chip8;
    chip8_init(&chip8);

    // chip8_screen_set(&chip8.screen, 10, 1);
    chip8_screen_draw_sprite(&chip8.screen, 30, 15, &chip8.memory.memory[0x05], 5);
    chip8_screen_draw_sprite(&chip8.screen, 35, 15, &chip8.memory.memory[0x00], 5);
    chip8.registers.deley_timer = 20;
    chip8.registers.sound_timer = 1;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow(
        EMULATOR_WINDOW_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        CHIP8_SCREEN_WIDTH * CHIP8_WINDOW_SCALE,
        CHIP8_SCREEN_HEIGHT * CHIP8_WINDOW_SCALE, SDL_WINDOW_SHOWN);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_TEXTUREACCESS_TARGET);
    
    bool condition = true;

    while(condition){
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type) {
            case SDL_QUIT:
                condition = false;
                break;

            case SDL_KEYDOWN:
            {
                char key = event.key.keysym.sym;
                int vkey = chip8_keyboard_map(keyboard_map, key);
                if (vkey != -1)
                {
                    chip8_keyboard_down(&chip8.keyboard, vkey);
                }
            }
            break;

            case SDL_KEYUP:
            {
                char key = event.key.keysym.sym;
                int vkey = chip8_keyboard_map(keyboard_map, key);
                if (vkey != -1)
                {
                    chip8_keyboard_up(&chip8.keyboard, vkey);
                }
            }
            break;
            };
        }

        SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR_R, BACKGROUND_COLOR_G, BACKGROUND_COLOR_B, BACKGROUND_COLOR_A);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, FOREGROUND_COLOR_R, FOREGROUND_COLOR_G, FOREGROUND_COLOR_B, FOREGROUND_COLOR_A);

        for (int x = 0; x < CHIP8_SCREEN_WIDTH; x++){
            for (int y = 0; y < CHIP8_SCREEN_HEIGHT; y++){
                if (chip8_screen_is_set(&chip8.screen, x, y)){
                    SDL_Rect r;
                    r.x = x * CHIP8_WINDOW_SCALE;
                    r.y = y * CHIP8_WINDOW_SCALE;
                    r.w = CHIP8_WINDOW_SCALE;
                    r.h = CHIP8_WINDOW_SCALE;
                    SDL_RenderFillRect(renderer, &r);
                }
            }
        }

        SDL_RenderPresent(renderer);

        /* delay timer */
        while (chip8.registers.deley_timer > 0){
            usleep(20000);  
            chip8.registers.deley_timer -= 1;
            printf("%d\n", chip8.registers.deley_timer);
            SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
        }

        /* sound timer */
        if (chip8.registers.sound_timer > 0){
            beep(4400, 20, 28000);
            chip8.registers.sound_timer -= 1;
        }
    }


    SDL_DestroyWindow(window);
    return 0;
}