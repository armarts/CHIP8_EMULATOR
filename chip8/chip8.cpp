/*
 *  Mega emulator chip8
 *  Copyright (C) 2017 Atayan Garik <olympg@yandex.ru>
 *
 *  This program is free software
 *  may 2017
 */

#include <stdlib.h>
#include <time.h>
#include "chip8.h"

#define ind(x, y) ( ((y + WIDTH) % WIDTH) * HEIGHT + ((x + HEIGHT) % HEIGHT) )

Chip8::Chip8() : BaseCPU(REGNUM, TIMERSNUM),
                 m_PC(ENTRYPOINT),
                 m_SP(0),
                 m_I(0),
                 m_DelayTimer(0),
                 m_SoundTimer(0)
{
    okConstruct = true;

    m_memory   = (uint8_t*)  calloc(MEMORYSIZE, sizeof(uint8_t));
    m_stack    = (uint16_t*) calloc(STACKSIZE, sizeof(uint16_t));
    m_register = (uint8_t*)  calloc(m_RegCount, sizeof(uint8_t));

    m_gfx = (int*) calloc(SCREENSIZE, sizeof(int));

    m_PC = ENTRYPOINT;
    m_SP = 0;

    drawFlag = true;

    if(m_memory == NULL || m_stack == NULL || m_register == NULL || m_gfx == NULL)
      okConstruct = false;

    if (okConstruct)
    {
      for(int i = 0; i < FONTSIZE; i++)
        m_memory[i] = Chip8_fontset[i];
    }

}

Chip8::~Chip8()
{
    free(m_memory);
    free(m_stack);
    free(m_register);

    m_memory = NULL;
    m_stack = NULL;
    m_register = NULL;
}

void Chip8::dump()
{

    printf("DUMP\n");

    printf("RegCount = %d\n", m_RegCount);
    printf("TimerCount = %d\n", m_TimerCount);
    printf("I = %d\n", m_I);
    printf("okConstruct = %d\n", okConstruct);
    printf("PC = %d\n", m_PC);
    printf("SP = %d\n", m_SP);

}

int FileSize(FILE *file)
{
    if (!file)
        return BADARGUMENT;

    fseek(file, 0, SEEK_END);

    size_t size = ftell(file);

    rewind(file);
    return size;
}

int Chip8::loadBinary(const char* path)
{

    FILE *rom = fopen(path, "rb");

    if (!rom)
        return BADOPEN;

    size_t romSize = FileSize(rom);

    uint8_t *romBuffer = (uint8_t*) calloc(romSize, sizeof(uint8_t));
    if (romBuffer == NULL)
        return BADALLOC;

    /* Load Chip8 file to buffer */

    size_t result = fread(romBuffer, sizeof(uint8_t), romSize, rom);

    fclose(rom);

    if (result != romSize)
        return BADREAD;

    /* Valid memory */
    if (romSize > MEMORYSIZE - ENTRYPOINT)
        return BIGFILE;

    for (size_t i = 0; i < romSize; i++)
        m_memory[m_PC + i] = romBuffer[i];

    free(romBuffer);

    return OK;

}

bool Chip8::drawStatus() const
{
    return drawFlag;
}

/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
/* /---------------------------------------------------------------- */
/*             List of function chip-8                               */

/* 00E0 - CLS */

int Chip8::Cls(int opcode)
{
    for (int i = 0; i < SCREENSIZE; i++)
      m_gfx[i] = 0;

    return 0;
}

// 00EE - RET

int Chip8::Ret(int opcode)
{

  if (m_SP <= 0 || m_SP >= STACKSIZE)
  {
    error = STACKERROR;
    return 0;
  }

  m_PC = m_stack[--m_SP];
  return 0;
}

// 1nnn - JP addr

int Chip8::Jp(int opcode)
{
  uint16_t address = ADDRESSMASK(opcode);

  if (address < ENTRYPOINT || address >= MEMORYSIZE)
  {
    error = ADDRESSERR;
    return 0;
  }

  m_PC = address;
  return 1;
}

// 2nnn - CALL addr

int Chip8::Call(int opcode)
{
  uint16_t address = ADDRESSMASK(opcode);

  if (address < ENTRYPOINT || address >= MEMORYSIZE)
  {
    error = ADDRESSERR;
    return 0;
  }
  m_stack[m_SP++] = m_PC;
  m_PC = address;
  return 1;
}

// 3xkk - SE Vx, byte

int Chip8::Se_Const(int opcode)
{
  int x_reg = XMASK(opcode);
  uint8_t kk = CONSTMASK(opcode);

  if (m_register[x_reg] == kk)
    m_PC += NEXT;
  return 0; 
}

// 4xkk - SNE Vx, byte

int Chip8::Sne_Const(int opcode)
{
  int x_reg = XMASK(opcode);
  uint8_t kk = CONSTMASK(opcode);

  if (m_register[x_reg] != kk)
    m_PC += NEXT;
  return 0;
}

// 5xy0 - SE Vx, Vy

int Chip8::Se_Reg(int opcode)
{
  int x_reg = XMASK(opcode);
  int y_reg = YMASK(opcode);

  if (m_register[x_reg] == m_register[y_reg])
   m_PC += NEXT;
  return 0;
}

// 6xkk - LD Vx, byte

int Chip8::Ld_Const(int opcode)
{
  int x_reg = XMASK(opcode);
  uint8_t kk = CONSTMASK(opcode);

  m_register[x_reg] = kk;
  return 0;
}

// 7xkk - ADD Vx, byte

int Chip8::Add_Const(int opcode)
{
  int x_reg = XMASK(opcode);
  uint8_t kk = CONSTMASK(opcode);

  m_register[x_reg] += kk;
  return 0;
}

// 8xy0 - LD Vx, Vy

int Chip8::Ld_Reg(int opcode)
{
  int x_reg = XMASK(opcode);
  int y_reg = YMASK(opcode);

  m_register[x_reg] = m_register[y_reg];
  return 0;
}

// 8xy1 - OR Vx, Vy

int Chip8::Or(int opcode)
{
  int x_reg = XMASK(opcode);
  int y_reg = YMASK(opcode);

  m_register[x_reg] |= m_register[y_reg];
  return 0;
}

// 8xy2 - AND Vx, Vy

int Chip8::And(int opcode)
{
  int x_reg = XMASK(opcode);
  int y_reg = YMASK(opcode);

  m_register[x_reg] &= m_register[y_reg];
  return 0;
}

// 8xy3 - XOR Vx, Vy

int Chip8::Xor(int opcode)
{
  int x_reg = XMASK(opcode);
  int y_reg = YMASK(opcode);

  m_register[x_reg] ^= m_register[y_reg];
  return 0;
}

// 8xy4 - ADD Vx, Vy

int Chip8::Add_Reg(int opcode)
{
  int x_reg = XMASK(opcode);
  int y_reg = YMASK(opcode);

  if (int(m_register[x_reg]) + int(m_register[y_reg]) < BYTE)
    m_register[VF] = 0;
  else
    m_register[VF] = 1;

  m_register[x_reg] += m_register[y_reg];
  return 0;
}

// 8xy5 - SUB Vx, Vy

int Chip8::Sub(int opcode)
{
  int x_reg = XMASK(opcode);
  int y_reg = YMASK(opcode);

  if (m_register[x_reg] >= m_register[y_reg])
    m_register[VF] = 1;
  else
    m_register[VF] = 0;

  m_register[x_reg] -= m_register[y_reg];
  return 0;
}

// 8xy6 - SHR Vx {, Vy}

int Chip8::Shr(int opcode)
{
  int x_reg = XMASK(opcode);

  m_register[VF] = m_register[x_reg] & 1;
  m_register[x_reg] >>= 1;
  return 0;
}

// 8xy7 - SUBN Vx, Vy

int Chip8::SubN(int opcode)
{
  int x_reg = XMASK(opcode);
  int y_reg = YMASK(opcode);

  if (m_register[y_reg] >= m_register[x_reg])
    m_register[VF] = 1;
  else
    m_register[VF] = 0;

  m_register[x_reg] = m_register[y_reg] - m_register[x_reg];
  return 0;
}

// 8xyE - SHL Vx, {, Vy}

int Chip8::Shl(int opcode)
{
  int x_reg = XMASK(opcode);

  m_register[VF] = m_register[x_reg] >> 7; // TO DO
  m_register[x_reg] <<= 1;
  return 0;
}

// 9xy0 - SNE Vx, Vy

int Chip8::Sne_Reg(int opcode)
{
  int x_reg = XMASK(opcode);
  int y_reg = YMASK(opcode);

  if (m_register[x_reg] != m_register[y_reg])
    m_PC += NEXT;
  return 0;
}

// Annn - LD I, addr

int Chip8::Ld_I(int opcode)
{
  uint16_t address =ADDRESSMASK(opcode);

  m_I = address;
  return 0;
}

// Bnnn - JP V0, addr

int Chip8::Jp_Reg(int opcode)
{
  uint16_t address = ADDRESSMASK(opcode);

  if (address < ENTRYPOINT || address >= MEMORYSIZE)
  {
    error = ADDRESSERR;
    return 0;
  }

  m_PC = m_register[V0] + address;
  return 1;
}

// Cxkk - RND, Vx, byte

int Chip8::Rnd(int opcode)
{
  int x_reg  = XMASK(opcode);
  int kk = (CONSTMASK(opcode));

  srand( time(NULL) );
  m_register[x_reg] = (rand() % 255) & kk; // TO DO
  return 0;
}

// DXYN - Drw sprite(N bytes) begining Vx, Vy

int Chip8::Drw(int opcode)
{
  int x_reg = XMASK(opcode);
  int y_reg = YMASK(opcode);
  int n = NIBBLE(opcode);

  int startX = m_register[x_reg];
  int startY = m_register[y_reg];
  drawFlag = false;

  m_register[VF] = 0;
  for (int y = 0; y < n; y++)
  {
    int pixels = m_memory[m_I + y];
    for (int x = 0; x < BYTESIZE; x++)
    {

      int *pixel = m_gfx + ind(startX + x, startY + y);
      int bit = (pixels >> (7 - x)) & 0x1;
      if (*pixel ^ bit != 0)
        drawFlag = true;

      if (bit)
      {
        if (*pixel)
          m_register[VF] = 1;
        *pixel ^= 1;
      }
    }
  }
 
  return 0;
}

// Ex9E - SKP Vx

int Chip8::Skp(int opcode)
{
  int x_reg = XMASK(opcode);

  if (keyboard.isKeyPressed(m_register[x_reg]))
    m_PC += NEXT;
  return 0;
}

// ExA1 - SKNP Vx

int Chip8::Sknp(int opcode)
{
  int x_reg = XMASK(opcode);

  if (!keyboard.isKeyPressed(m_register[x_reg]))
    m_PC += NEXT;
  return 0;
}

// Fx07 - LD Vx, DT

int Chip8::Ld_Reg_Dt(int opcode)
{
  int x_reg = XMASK(opcode);

  m_register[x_reg] = m_DelayTimer;
  return 0;
}

// Fx0A - LD Vx, K

int Chip8::Ld_Key(int opcode)
{
  int x_reg = XMASK(opcode);

  int key = keyboard.isAnyKeyPressed();
  if (key != -1)
  {
    m_register[x_reg] = key;
    return 0;
  }
  return 1;
}

// Fx15 - LD DT, Vx

int Chip8::Ld_Dt(int opcode)
{
  int x_reg = XMASK(opcode);

  m_DelayTimer = m_register[x_reg];
  return 0;
}

// Fx18 - LD ST, Vx

int Chip8::Ld_St(int opcode)
{
  int x_reg = XMASK(opcode);
  m_SoundTimer = m_register[x_reg];
  return 0;
}

// Fx1E - ADD I, Vx

int Chip8::Add_I(int opcode)
{
  int x_reg = XMASK(opcode);

  m_I += m_register[x_reg];
  return 0;
}

// Fx29 - LD F, Vx

int Chip8::Ld_Spr(int opcode)
{
  int x_reg = XMASK(opcode);

  m_I = m_register[x_reg] * NUMBERLENGTH;
  return 0;
}

// Fx33 - LD B, Vx

int Chip8::Ld_Bcd(int opcode)
{
  int x_reg = XMASK(opcode);

  /* BCD (123) -> 1 2 3 */

  m_memory[m_I] = m_register[x_reg] / 100;
  m_memory[m_I + 1] = (m_register[x_reg] / 10) % 10;
  m_memory[m_I + 2] = (m_register[x_reg] % 10);
  return 0;
}

// Fx55 - LD [I], Vx

int Chip8::Ld_Reg_Mem(int opcode)
{
  int x_reg = XMASK(opcode);

  for (int i = 0; i <= x_reg; i++)
    m_memory[m_I + i] = m_register[i];

  m_I += x_reg + 1;
  return 0;
}

// FX65 - LD Vx, [I]

int Chip8::Ld_Reg_Load(int opcode)
{
  int x_reg = XMASK(opcode);

  for (int i = 0; i <= x_reg; i++)
    m_register[i] = m_memory[m_I + i];

  m_I += x_reg + 1;
  return 0;
}

/* End Of ListFunctions */


void Chip8::decreaseTimers()
{
  if(m_DelayTimer > 0)
    --m_DelayTimer;
  
  if(m_SoundTimer > 0)
    --m_SoundTimer;
}

uint16_t Chip8::fetch()
{
  /* end of executing */
  if ((m_memory[m_PC] == 0) && (m_memory[m_PC + 1] == 0))
    return 0;

  uint16_t result = m_memory[m_PC] << BYTESIZE;
  result |= m_memory[m_PC + 1];
  return result;
}

// have to add operator switch

uint16_t Chip8::decode(uint16_t cmd)
{
  uint8_t first  = NIBBLE((cmd >> 12));
  uint8_t second = NIBBLE((cmd >> 8));
  uint8_t third  = NIBBLE((cmd >> 4));
  uint8_t fourth = NIBBLE((cmd));

  int  result = first;

  if (first == 0x0)
    result = cmd;

  if (first == 0x8)
    result = (result << 4) + fourth;

  if (first == 0xE || first == 0xF)
    result = (result << 8) + (third << 4) + fourth;

  return result;
}

void Chip8::execute(uint16_t decodedCmd, uint16_t cmd)
{
  transaction_callBack cw = NULL;
  for (int i = 0; i < 34; i++)
    if (decodedCmd == FSM[i].code)
    {
      /* call system function */
      cw = FSM[i].worker;
      int goNext = (this->*cw)(cmd);
      if(goNext == 0)
        m_PC += NEXT;
      break;
    }
  if (cw == NULL)
    error = UNKNOWN;
}


