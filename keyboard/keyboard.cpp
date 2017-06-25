/*
 *  Mega emulator chip8
 *  Copyright (C) 2017 Atayan Garik <olympg@yandex.ru>
 *
 *  This program is free software
 *  may 2017
 */


#include "keyboard.h"

Chip8Keyboard::Chip8Keyboard()
{
    for(int i = 0; i < 16; i++)
        m_key[i] = false;
}

void Chip8Keyboard::pressKey(uint8_t keyNumber)
{
    m_key[keyNumber] = true;
}

bool Chip8Keyboard::isKeyPressed(uint8_t keyNumber)
{
    return m_key[keyNumber];
}

void Chip8Keyboard::releaseKey(uint8_t keyNumber)
{
    m_key[keyNumber] = false;
}


int Chip8Keyboard::isAnyKeyPressed()
{

  for (int i = 0; i < 16; i++)
  {
    if (isKeyPressed(i))
      return i;
  }

  /* if any key is not pressed */

  return -1;

}
