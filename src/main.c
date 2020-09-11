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
    if(argc < 2){
        printf("you must provide a file to load\n");
        return -1;
    }

    const char* filename = argv[1];
    printf("filename: %s\n", filename);
    FILE* f = fopen(filename, "rb");
    if(!f){
        printf("cant open file");
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0 , SEEK_SET);

    char buf[size];
    int res = fread(buf, size, 1, f);

    if (res != 1){
        printf("failed to read file");
        return -1;
    }


    struct chip8 chip8;
    chip8_init(&chip8);
    chip8_load(&chip8, buf, size);
    chip8_keyboard_set_map(&chip8.keyboard, keyboard_map);

    // chip8.registers.PC = 0;
    // chip8.registers.I = 0x00;    
    // chip8.registers.V[0] = 10;
    // chip8.registers.V[1] = 10;
    // chip8_exec(&chip8, 0xD015);
 
    // init timers
    chip8.registers.deley_timer = 0;
    chip8.registers.sound_timer = 0;

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
                int vkey = chip8_keyboard_map(&chip8.keyboard, key);
                if (vkey != -1)
                {
                    chip8_keyboard_down(&chip8.keyboard, vkey);
                }
            }
            break;

            case SDL_KEYUP:
            {
                char key = event.key.keysym.sym;
                int vkey = chip8_keyboard_map(&chip8.keyboard, key);
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
            usleep(50000);  
            chip8.registers.deley_timer -= 1;
            SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
        }

        /* sound timer */
        if (chip8.registers.sound_timer > 0){
            beep(3400, 2 * chip8.registers.sound_timer, 28000);
            chip8.registers.sound_timer = 0;
        }

        unsigned short opcode = chip8_memory_get_short(&chip8.memory, chip8.registers.PC);
        chip8.registers.PC += 2;
        chip8_exec(&chip8, opcode);
    }


    SDL_DestroyWindow(window);
    return 0;
}