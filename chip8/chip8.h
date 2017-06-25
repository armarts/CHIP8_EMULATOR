/*
 *  Mega emulator chip8
 *  Copyright (C) 2017 Atayan Garik <olympg@yandex.ru>
 *
 *  This program is free software
 *  may 2017
 */

#ifndef __Chip8__H__
#define __Chip8__H__

#include "../systemData.h"
#include "../cpu/cpuBase.h"
#include "../keyboard/keyboard.h"
#include <cstring>
#include <fstream>
#include <cstdio>

#define BYTESIZE 8
#define BYTE 0x100
#define MEMORYSIZE 0x1000
#define VIDEOMEMORY 0xF00
#define NEXT 2
#define ENTRYPOINT 0x200
#define STACKSIZE 16
#define SCREENSIZE 2048
#define HEIGHT 64
#define WIDTH 32
#define REGNUM 16
#define TIMERSNUM 2
#define FONTSIZE 80
#define ADDRESSMASK(arg) (arg & 0x0FFF)
#define XMASK(arg) ((arg & 0x0F00) >> 8)
#define YMASK(arg) ((arg & 0x00F0) >> 4)
#define NIBBLE(arg) (arg & 0x000F)
#define CONSTMASK(arg) (arg & 0x00FF)
#define NUMBERLENGTH 0x5

#define V0 0x0
#define V1 0x1
#define V2 0x2
#define V3 0x3
#define V4 0x4
#define V5 0x5
#define V6 0x6
#define V7 0x7
#define V8 0x8
#define V9 0x9
#define VA 0xA
#define VB 0xB
#define VC 0xC
#define VD 0xD
#define VE 0xE
#define VF 0xF

class Chip8 : public BaseCPU
{

    public :

        Chip8();

        typedef int (Chip8::*transaction_callBack) (int);

        struct transaction
        {
            command code;
            transaction_callBack worker;
        };

        struct transaction FSM[34] =
        {
            [0]  = {CLS,         &Chip8::Cls},
            [1]  = {RET,         &Chip8::Ret},
            [2]  = {JP,          &Chip8::Jp},
            [3]  = {CALL,        &Chip8::Call},
            [4]  = {SE_CONST,    &Chip8::Se_Const},
            [5]  = {SNE_CONST,   &Chip8::Sne_Const},
            [6]  = {SE_REG,      &Chip8::Se_Reg},
            [7]  = {LD_CONST,    &Chip8::Ld_Const},
            [8]  = {ADD_CONST,   &Chip8::Add_Const},
            [9]  = {LD_REG,      &Chip8::Ld_Reg},
            [10] = {OR,          &Chip8::Or},
            [11] = {AND,         &Chip8::And},
            [12] = {XOR,         &Chip8::Xor},
            [13] = {ADD_REG,     &Chip8::Add_Reg},
            [14] = {SUB,         &Chip8::Sub},
            [15] = {SHR,         &Chip8::Shr},
            [16] = {SUBN,        &Chip8::SubN},
            [17] = {SHL,         &Chip8::Shl},
            [18] = {SNE_REG,     &Chip8::Sne_Reg},
            [19] = {LD_I,        &Chip8::Ld_I},
            [20] = {JP_REG,      &Chip8::Jp_Reg},
            [21] = {RND,         &Chip8::Rnd},
            [22] = {DRW,         &Chip8::Drw},
            [23] = {SKP,         &Chip8::Skp},
            [24] = {SKNP,        &Chip8::Sknp},
            [25] = {LD_REG_DT,   &Chip8::Ld_Reg_Dt},
            [26] = {LD_KEY,      &Chip8::Ld_Key},
            [27] = {LD_DT,       &Chip8::Ld_Dt},
            [28] = {LD_ST,       &Chip8::Ld_St},
            [29] = {ADD_I,       &Chip8::Add_I},
            [30] = {LD_SPR,      &Chip8::Ld_Spr},
            [31] = {LD_BCD,      &Chip8::Ld_Bcd},
            [32] = {LD_REG_MEM,  &Chip8::Ld_Reg_Mem},
            [33] = {LD_REG_LOAD, &Chip8::Ld_Reg_Load}
        };


        virtual ~Chip8();

        virtual int loadBinary(const char* path);

        /* List of function chip-8 */

        int        Cls(int opcode);
        int        Ret(int opcode);
        int         Jp(int opcode);
        int       Call(int opcode);
        int   Se_Const(int opcode);
        int  Sne_Const(int opcode);
        int     Se_Reg(int opcode);
        int   Ld_Const(int opcode);
        int  Add_Const(int opcode);
        int     Ld_Reg(int opcode);
        int         Or(int opcode);
        int        And(int opcode);
        int        Xor(int opcode);
        int    Add_Reg(int opcode);
        int        Sub(int opcode);
        int        Shr(int opcode);
        int       SubN(int opcode);
        int        Shl(int opcode);
        int    Sne_Reg(int opcode);
        int       Ld_I(int opcode);
        int     Jp_Reg(int opcode);
        int        Rnd(int opcode);
        int        Drw(int opcode);
        int        Skp(int opcode);
        int       Sknp(int opcode);
        int     Ld_Key(int opcode);
        int  Ld_Reg_Dt(int opcode);
        int      Ld_Dt(int opcode);
        int      Ld_St(int opcode);
        int     Ld_Spr(int opcode);
        int      Add_I(int opcode);
        int Ld_Reg_Mem(int opcode);
        int     Ld_Bcd(int opcode);
        int Ld_Reg_Load(int opcode);

        /* @-------------------@  */

        void dump();

        virtual uint16_t fetch();
        virtual uint16_t decode(uint16_t cmd);
        virtual void execute(uint16_t decodedCmd, uint16_t cmd);

        bool drawStatus() const;
        void decreaseTimers();

        bool okConstruct;

        Chip8Keyboard keyboard;
        int *m_gfx;

    private :

        uint8_t* m_register;
        uint8_t* m_memory;

        uint16_t* m_stack;

        uint16_t m_PC;
        uint16_t m_SP;
        uint16_t m_I;

       
        bool drawFlag;

        int m_DelayTimer;
        int m_SoundTimer;

};

#endif
