all: emu

emu: main.o chip8.o cpu.o
	g++ main.o chip8.o cpu.o -o emu

main.o: main.cpp
	g++ -c main.cpp

cpu.o: cpuBase.cpp
	g++ -c cpu/cpuBase.cpp

chip8.o: chip8.cpp
	g++ -c chip8/chip8.cpp

