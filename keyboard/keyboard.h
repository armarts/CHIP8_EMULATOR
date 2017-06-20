#ifndef __KEYBOARD__H__
#define __KEYBOARD__H__

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

#endif

