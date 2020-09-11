#include "chip8.h"
#include <memory.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>  // for random number
#include <time.h>  // for random number
#include <SDL2/SDL.h>  // wait for key


const char chip8_default_character_set[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


void chip8_init(struct chip8* chip8){
    memset(chip8, 0, sizeof(struct chip8));
    memcpy(&chip8->memory.memory, chip8_default_character_set, sizeof(chip8_default_character_set));
}

void chip8_load(struct chip8* chip8, const char* buf, size_t size){
    assert(size + CHIP8_PROGRAM_LOAD_ADDRESS < CHIP8_MEMORY_SIZE);
    memcpy(&chip8->memory.memory[CHIP8_PROGRAM_LOAD_ADDRESS], buf, size);
    chip8->registers.PC = CHIP8_PROGRAM_LOAD_ADDRESS;
}

static char chip8_wait_for_key_press(struct chip8* chip8){
    SDL_Event event;
    while(SDL_WaitEvent(&event)){
        if(event.type != SDL_KEYDOWN)
            continue;
        char c = event.key.keysym.sym;
        char chip8_key = chip8_keyboard_map(&chip8->keyboard, c);
        if (chip8_key != -1)
            return chip8_key;
    }
    return -1;
}

static void chip8_exec_extended(struct chip8* chip8, unsigned opcode){
    unsigned short nnn = opcode & 0x0FFF;
    unsigned char x = (opcode >> 8) & 0x000F;
    unsigned char y = (opcode >> 4) & 0x000F;
    unsigned char z = opcode & 0x000F;
    unsigned char kk = opcode & 0x00FF;
    unsigned short tmp = 0;
    unsigned char n = opcode & 0x000f;

    switch(opcode & 0xF000){
        case 0x1000:  // JP addr: 1nnn jump to address nnn
            chip8->registers.PC = nnn;
            break;
        
        case 0x2000:  // CALL addr: call subroutin at nnn
            chip8_stack_push(chip8, chip8->registers.PC);
            chip8->registers.PC = nnn;
            break;
        
        case 0x3000:  // SE Vx, bytes: skip next instruction if Vx = kk
            if(chip8->registers.V[x] == kk)
                chip8->registers.PC += 2;
            break;
        
        case 0x4000:  // SNE Vx, bytes: skip next instruction if Vx != kk
            if(chip8->registers.V[x] != kk)
                chip8->registers.PC += 2;
            break;
        
        
        case 0x5000:  // 5xy0 - SE Vx Vy: skip next instruction if Vx == Vy
            if(chip8->registers.V[x] == chip8->registers.V[y])
                chip8->registers.PC += 2;
            break;
        
        case 0x6000:  // 6xkk - Ld Vx, byte: Vx = kk
            chip8->registers.V[x] = kk;
            break;
        
        case 0x7000: // 7xkk - Add Vx, byte: Vx += kk
            chip8->registers.V[x] += kk;
            break;
        
        case 0x8000:  // 8xyz
            switch(z){
                case 0x0:  // 8xy0 - LD Vx, Vy: Vx = Vy
                    chip8->registers.V[x] = chip8->registers.V[y];
                    break;
                case 0x1:  // 8xy1 - OR Vx, Vy: Vx = Vx OR Vy
                    chip8->registers.V[x] |= chip8->registers.V[y];
                    break;
                case 0x2:  // 8xy2 - AND Vx, Vy: Vx = Vx AND Vy
                    chip8->registers.V[x] &= chip8->registers.V[y];
                    break;
                case 0x3:  // 8xy3 - XOR Vx, Vy: Vx = Vx XOR Vy
                    chip8->registers.V[x] ^= chip8->registers.V[y];
                    break;
                case 0x4:  // 8xy4 - ADD Vx, Vy: Vx = Vx + Vy, Vf = carry
                    tmp = chip8->registers.V[x] + chip8->registers.V[y];
                    chip8->registers.V[x] = tmp;
                    chip8->registers.V[0xf] = false;
                    if(tmp > 0xff)
                        chip8->registers.V[0xf] = true; 
                    break;
                case 0x5:  // 8xy5 - SUB Vx, Vy: Vx = Vx - Vy, Vf = borrow
                    chip8->registers.V[0xf] = false;
                    if(chip8->registers.V[x] > chip8->registers.V[y])
                        chip8->registers.V[0xf] = true;
                    chip8->registers.V[x] -= chip8->registers.V[y];
                    break;
                case 0x6:  // 6xy6 - SHR Vx {, Vy}: Vx = Vx SHR 1
                    chip8->registers.V[0xf] = chip8->registers.V[x] & 0x1;
                    chip8->registers.V[x] /= 2;
                    break;
                case 0x7:  // 8xy7 - SUBN Vx, Vy
                    chip8->registers.V[0xf] = chip8->registers.V[y] > chip8->registers.V[x];
                    chip8->registers.V[x] = chip8->registers.V[y] - chip8->registers.V[x];
                    break;
                case 0xE:  // 8xyE SHL Vx {, Vy}: Vx = Vx SHL 1
                    chip8->registers.V[0xf] = chip8->registers.V[x] & 0x80;
                    chip8->registers.V[x] *= 2;
                    break;
            }
            break;
        
        case 0x9000:  // 9xy0 - SNE Vx, Vy: skip next instruction if Vx != Vy
            if(chip8->registers.V[x] != chip8->registers.V[y])
                chip8->registers.PC += 2;
            break;
        
        case 0xA000:  // LD I, addr: T = nnn
            chip8->registers.I = nnn;
            break;
        
        case 0xB000:  // JP V0, addr: jupm to location nnn + V0
            chip8->registers.PC = chip8->registers.V[0x0] + nnn;
            break;
        
        case 0xC000:  // RND Vx, byte: Vx = rand AND kk
            srand(clock());
            chip8->registers.V[x] = kk & (rand() % 255);
            break;
        
        case 0xD000: {  // DRW Vx, Vy, nibble: display n-byte sprite at memory loacaten I at (Vx, Vy), Vf = collision
            const char* sprite = (const char*) &chip8->memory.memory[chip8->registers.I];
            chip8->registers.V[0xf] = chip8_screen_draw_sprite(&chip8->screen,
                                                                chip8->registers.V[x],
                                                                chip8->registers.V[y],
                                                                sprite,
                                                                n); }
            break;
        
        case 0xE000:  // keyboard operations
            switch(kk){
                case 0x9E:  // Ex9E - SKP Vx, skip next instruction if key with value of Vx is pressed
                    if(chip8_keyboard_is_down(&chip8->keyboard, chip8->registers.V[x]))
                        chip8->registers.PC += 2;
                    break;
                
                case 0xA1:  // ExA1 - SKNP Vx, skip next instruction if key with value of Vx is not pressed
                    if(!chip8_keyboard_is_down(&chip8->keyboard, chip8->registers.V[x]))
                        chip8->registers.PC += 2;
                    break;
            }
            break;

        case 0xF000:
            switch(kk){
                case 0x07:  // fx07 - LD Vx, Dt: Vx = deley timer
                    chip8->registers.V[x] = chip8->registers.deley_timer;        
                    break;
                
                case 0x0A:{  // fx0a - LD Vx, k
                    char pressed_key = chip8_wait_for_key_press(chip8);
                    chip8->registers.V[x] = pressed_key;}
                    break;
                
                case 0x15:  // LD DT, Vx: delay timer = Vx
                    chip8->registers.deley_timer = chip8->registers.V[x];
                    break;
                
                case 0x18:  // LD ST, V8: sound timer = Vx
                    chip8->registers.sound_timer = chip8->registers.V[x];
                    break;
                
                case 0x1E:  // ASS I, Vx
                    chip8->registers.I += chip8->registers.V[x];
                    break;
                
                case 0x29:  // LD F, Vx: location of sprite for digit Vx
                    chip8->registers.I = chip8->registers.V[x] * CHIP8_DEFAULT_SPRITE_HEGHT;
                    break;
                
                case 0x33:{  // LD B, Vx: store BSD of Vx in I, I+1, I+2
                    unsigned char hundreds = chip8->registers.V[x] / 100;
                    unsigned char tens = (chip8->registers.V[x] / 10) % 10;
                    unsigned char units = (chip8->registers.V[x]) % 10;
                    chip8_memory_set(&chip8->memory, chip8->registers.I, hundreds);
                    chip8_memory_set(&chip8->memory, chip8->registers.I + 1, tens);
                    chip8_memory_set(&chip8->memory, chip8->registers.I + 2, units);}
                    break;
                
                case 0x55:{  // LD [I], Vx: strore V0-VF in memory address indicated by I
                    for(int i = 0; i <= x; i++)
                        chip8_memory_set(&chip8->memory, chip8->registers.I+i, chip8->registers.V[i]);}
                    break;
                
                case 0x65:{  // LD Vx, [I]: read V0-VF from memory address indicated by I
                    for(int i = 0; i <= x; i++)
                        chip8->registers.V[i] = chip8_memory_get(&chip8->memory, chip8->registers.I+i);}
                    break;
            }   
            break;
        
    }
}

void chip8_exec(struct chip8* chip8, unsigned short opcode){
    switch(opcode){
        case 0x00E0:  // CLS: clear display
            chip8_screen_clear(&chip8->screen);
            break;
        case 0x00EE:  // RET: return from subroutin
            chip8->registers.PC = chip8_stack_pop(chip8);
            break;

        default:
            chip8_exec_extended(chip8, opcode);
    }
}