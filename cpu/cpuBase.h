/*
 *  Mega emulator chip8
 *  Copyright (C) 2017 Atayan Garik <olympg@yandex.ru>
 *
 *  This program is free software
 *  may 2017
 */


#ifndef __BASECPU__H__
#define __BASECPU__H__

#include <stdint.h>

class BaseCPU
{
    public:

        BaseCPU() = delete;
        BaseCPU(int RegCount, int TimerCount);
        virtual ~BaseCPU();
        virtual int loadBinary(const char *path) = 0;

        void doCycle();

        virtual uint16_t fetch() = 0;
        virtual uint16_t decode(uint16_t cmd) = 0;
        virtual void execute(uint16_t decodedCmd, uint16_t cmd) = 0;

    protected:

        int m_RegCount;
        int m_TimerCount;


};

#endif
