#include <cstdio>
#include <SFML/Graphics.hpp>
#include "chip8/chip8.h"


int main(int argc, char **argv)
{

  Chip8 emulator;

  if (argc != 2)
  {
    printf("Usage: ROM file is missing\n");
    exit(1);
  }

  emulator.loadBinary(argv[1]);

  emulator.Run();

    /*
  while (true)
  {
    doCycle();
    if (DelayTimer > 0)
      DelayTimer--;
  }*/

  return 0;

}
