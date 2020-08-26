#ifndef CONFIG_H
#define CONFIG_H

/* monitor */
#define EMULATOR_WINDOW_TITLE "Chip8 Emulator v0.1"
#define CHIP8_WIDTH 64
#define CHIP8_HEIGHT 32
#define CHIP8_WINDOW_SCALE 10

#define BACKGROUND_COLOR_R 0
#define BACKGROUND_COLOR_G 0
#define BACKGROUND_COLOR_B 255
#define BACKGROUND_COLOR_A 0

#define FOREGROUND_COLOR_R 255
#define FOREGROUND_COLOR_G 255
#define FOREGROUND_COLOR_B 0
#define FOREGROUND_COLOR_A 0


/* memory */
#define CHIP8_MEMORY_SIZE 4096

/* registers */
#define CHIP8_TOTAL_DATA_REGISTERS 16

/* stack */
#define CHIP8_TOTAL_STACK_DEPTH 16

/* keyboard */
#define CHIP8_TOTAL_KEYS 16

#endif