/*
 *  Mega emulator chip8
 *  Copyright (C) 2017 Atayan Garik <olympg@yandex.ru>
 *
 *  This program is free software
 *  may 2017
 */


#include "cpuBase.h"

BaseCPU::BaseCPU(int RegCount, int TimerCount) :
    m_RegCount(RegCount),
    m_TimerCount(TimerCount)
{}

BaseCPU::~BaseCPU()
{}

void BaseCPU::doCycle()
{
    uint16_t instruction;
    uint16_t decodedInstruction;

    instruction = fetch();
    decodedInstruction = decode(instruction);
    execute(decodedInstruction, instruction);

}
