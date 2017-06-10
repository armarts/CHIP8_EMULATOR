#include <stdio.h>
#include "chip8.h"

int main(int argc, char **argv)
{
    Chip8 my;

    my.loadBinary(argv[1]);

    my.Run();

    return 0;
}
