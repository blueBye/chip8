INCLUDES= -I ./include
FLAGS= -g
LIBRARIES= -lSDL2 -lSDL2main
OBJECTS= ./build/chip8memory.o ./build/chip8stack.o ./build/chip8keyboard.o	

all: ${OBJECTS}
	gcc ${FLAGS} ${INCLUDES} ./src/main.c ${OBJECTS} ${LIBRARIES} -o ./bin/main

./build/chip8memory.o: ./src/chip8memory.c
	gcc ${FLAGS} ${INCLUDES} ./src/chip8memory.c -c -o ./build/chip8memory.o

./build/chip8stack.o: ./src/chip8stack.c
	gcc ${FLAGS} ${INCLUDES} ./src/chip8stack.c -c -o ./build/chip8stack.o

./build/chip8keyboard.o: ./src/chip8stack.c
	gcc ${FLAGS} ${INCLUDES} ./src/chip8keyboard.c -c -o ./build/chip8keyboard.o


clean:
	rm -rf ./build/*
	rm -rf ./bin/*