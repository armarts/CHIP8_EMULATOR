#include "keyboard.h"
#include <stdio.h>

Keyboard::Keyboard()
{
    for(int i = 0; i < 16; i++)
        key[i] = false;
}

void Keyboard::pressKey(uint8_t keyNumber)
{
    key[keyNumber] = true;
}

bool Keyboard::isKeyPressed(uint8_t keyNumber)
{
    /*if(KeyNumber > 16)
        return */
    return key[keyNumber];
}

void Keyboard::releaseKey(uint8_t keyNumber)
{
    key[keyNumber] = false;
}

void Keyboard::dump()
{
    for(int i = 0; i < 16; i++)
        printf("[%d] = %d\n", i, key[i]);
}

int Keyboard::isAnyKeyPressed()
{

  for (int i = 0; i < 16; i++)
  {
    if (isKeyPressed(i))
      return i;
  }

  /* if any key is not pressed */

  return -1;

}
