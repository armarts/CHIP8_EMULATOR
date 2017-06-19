
#include <stdint.h>

class Keyboard
{
public:
    Keyboard();
    //~Keyboard();
    void pressKey(uint8_t keyNumber);
    void releaseKey(uint8_t keyNumber);
    bool isKeyPressed(uint8_t keyNumber);
    int isAnyKeyPressed();
    void dump();


private:
    bool key[16];
};

//#include "keyboard.cpp"
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
