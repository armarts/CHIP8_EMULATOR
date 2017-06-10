/********************************************
 *                                          *
 ********************************************/

#include <stdlib.h>
#include <time.h>

#define MEMORYSIZE 0x1000
#define VIDEOMEMORY 0xF00
#define NEXT 0x2
#define ENTRYPOINT 0x200
#define STACKSIZE 0x10
#define SCREENSIZE 0x800
#define HEIGHT 0x40
#define WEIGHT 0x20
#define REGNUM 0x10
#define TIMERSNUM 0x2
#define FONTSIZE 0x50
#define ADDRESSMASK(arg) (arg & 0x0FFF)
#define XMASK(arg) ((arg & 0x0F00) >> 8)
#define YMASK(arg) ((arg & 0x00F0) >> 4)
#define NIBBLE(arg) (arg & 0x000F)
#define CONSTMASK(arg) (arg & 0x00FF)

#define ind(x, y) ( ((y + WEIGHT) % WEIGHT) * HEIGHT + ((x + HEIGHT) % HEIGHT) )

Chip8::Chip8() : BaseCPU(REGNUM, TIMERSNUM),
                 m_PC(ENTRYPOINT),
                 m_SP(0),
                 m_I(0),
                 m_DelayTimer(0),
                 m_SoundTimer(0)
{
    m_okConstruct = true;

    m_memory   = (uint8_t*)  calloc(MEMORYSIZE, sizeof(uint8_t));
    m_stack    = (uint16_t*) calloc(STACKSIZE, sizeof(uint16_t));
    m_register = (uint8_t*)  calloc(m_RegCount, sizeof(uint8_t));

    m_gfx = (int*) calloc(SCREENSIZE, sizeof(int));

    m_PC = ENTRYPOINT;
    m_SP = 0;

    if(m_memory == NULL || m_stack == NULL || m_register == NULL || m_gfx == NULL)
      m_okConstruct = false;

    if (m_okConstruct)
    {
      for(int i = 0; i < FONTSIZE; i++)
        m_memory[i] = Chip8_fontset[i];
    }

}

Chip8::~Chip8()
{
    if (m_okConstruct)
    {
      free(m_memory);
      free(m_stack);
      free(m_register);
      m_memory = NULL;
      m_stack = NULL;
      m_register = NULL;
    }
}

void Chip8::dump()
{

    printf("DUMP\n");

    printf("RegCount = %d\n", m_RegCount);
    printf("TimerCount = %d\n", m_TimerCount);
    printf("I = %d\n", m_I);
    printf("okConstruct = %d\n", m_okConstruct);
    printf("PC = %d\n", m_PC);
    printf("SP = %d\n", m_SP);

  /*
    for(int i = 0; i < MEMORYSIZE; i++)
        printf("[%d] = %02x \n", i, m_memory[i]);
  */
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

    /* Open file */

    FILE *rom = fopen(path, "rb");

    if (!rom)
        return BADOPEN;

    size_t romSize = FileSize(rom);

    uint8_t *romBuffer = (uint8_t*) calloc(romSize, sizeof(uint8_t));
    if (romBuffer == NULL)
        return BADALLOC;

    /* Load Chip8 file to buffer */

    size_t result = fread(romBuffer, sizeof(uint8_t), romSize, rom);

    /* Close File */

    fclose(rom);

    if (result != romSize)
        return BADREAD;

    for (size_t i = 0; i < romSize; i++)
        m_memory[m_PC + i] = romBuffer[i]; //TODO

    free(romBuffer);

    return OK;

}

/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
/* /---------------------------------------------------------------- */
/*             List of function chip-8                               */

/* 00E0 - CLS */

void Chip8::Cls(int opcode)
{
    //memset(m_gfx, 0, SCREENSIZE);

    for (int i = 0; i < SCREENSIZE; i++)
      m_gfx[i] = 0;

    //drawFlag = true;

    m_PC += NEXT;
}

// 00EE - RET

void Chip8::Ret(int opcode)
{

  if (m_SP <= 0 || m_SP >= STACKSIZE)
  {
    error = STACKERROR;
    return ;
  }

  m_PC = m_stack[--m_SP];
  m_PC += NEXT;
}

// 1nnn - JP addr

void Chip8::Jp(int opcode)
{
  uint16_t address = ADDRESSMASK(opcode);

  if (address < ENTRYPOINT || address >= MEMORYSIZE)
  {
    error = ADDRESSERR;
    return ;
  }

  m_PC = address;
}

// 2nnn - CALL addr

void Chip8::Call(int opcode)
{
  uint16_t address = ADDRESSMASK(opcode);

  if (address < ENTRYPOINT || address >= MEMORYSIZE)
  {
    error = ADDRESSERR;
    return ;
  }
  m_stack[m_SP++] = m_PC;
  m_PC = address;
}

// 3xkk - SE Vx, byte

void Chip8::Se_Const(int opcode)
{
  int x_reg = XMASK(opcode);
  uint8_t kk = CONSTMASK(opcode);

  if (m_register[x_reg] == kk)
    m_PC += 2 * NEXT;
  else
    m_PC += NEXT;
}

// 4xkk - SNE Vx, byte

void Chip8::Sne_Const(int opcode)
{
  int x_reg = XMASK(opcode);
  uint8_t kk = CONSTMASK(opcode);

  if (m_register[x_reg] != kk)
    m_PC += 2 * NEXT;
  else
    m_PC += NEXT;
}

// 5xy0 - SE Vx, Vy

void Chip8::Se_Reg(int opcode)
{
  int x_reg = XMASK(opcode);
  int y_reg = YMASK(opcode);

  if (m_register[x_reg] == m_register[y_reg])
   m_PC += 2 * NEXT;
  else
   m_PC += NEXT;
}

// 6xkk - LD Vx, byte

void Chip8::Ld_Const(int opcode)
{
  int x_reg = XMASK(opcode);
  uint8_t kk = CONSTMASK(opcode);

  m_register[x_reg] = kk;
  m_PC += NEXT;
}

// 7xkk - ADD Vx, byte

void Chip8::Add_Const(int opcode)
{
  int x_reg = XMASK(opcode);
  uint8_t kk = CONSTMASK(opcode);

  m_register[x_reg] += kk;
  m_PC += NEXT;
}

// 8xy0 - LD Vx, Vy

void Chip8::Ld_Reg(int opcode)
{
  int x_reg = XMASK(opcode);
  int y_reg = YMASK(opcode);

  m_register[x_reg] = m_register[y_reg];
  m_PC += NEXT;
}

// 8xy1 - OR Vx, Vy

void Chip8::Or(int opcode)
{
  int x_reg = XMASK(opcode);
  int y_reg = YMASK(opcode);

  m_register[x_reg] |= m_register[y_reg];
  m_PC += NEXT;
}

// 8xy2 - AND Vx, Vy

void Chip8::And(int opcode)
{
  int x_reg = XMASK(opcode);
  int y_reg = YMASK(opcode);

  m_register[x_reg] &= m_register[y_reg];
  m_PC += NEXT;
}

// 8xy3 - XOR Vx, Vy

void Chip8::Xor(int opcode)
{
  int x_reg = XMASK(opcode);
  int y_reg = YMASK(opcode);

  m_register[x_reg] ^= m_register[y_reg];
  m_PC += NEXT;
}

// 8xy4 - ADD Vx, Vy

void Chip8::Add_Reg(int opcode)
{
  int x_reg = XMASK(opcode);
  int y_reg = YMASK(opcode);

  if (int(m_register[x_reg]) + int(m_register[y_reg]) < 256)
    m_register[VF] = 0;
  else
    m_register[VF] = 1;

  m_register[x_reg] += m_register[y_reg];
  m_PC += NEXT;
}

// 8xy5 - SUB Vx, Vy

void Chip8::Sub(int opcode)
{
  int x_reg = XMASK(opcode);
  int y_reg = YMASK(opcode);

  if (m_register[x_reg] >= m_register[y_reg])
    m_register[VF] = 1;
  else
    m_register[VF] = 0;

  m_register[x_reg] -= m_register[y_reg];
  m_PC += NEXT;
}

// 8xy6 - SHR Vx {, Vy}

void Chip8::Shr(int opcode)
{
  int x_reg = XMASK(opcode);

  m_register[VF] = m_register[x_reg] & 1;
  m_register[x_reg] >>= 1;

  m_PC += NEXT;
}

// 8xy7 - SUBN Vx, Vy

void Chip8::SubN(int opcode)
{
  int x_reg = XMASK(opcode);
  int y_reg = YMASK(opcode);

  if (m_register[y_reg] >= m_register[x_reg])
    m_register[VF] = 1;
  else
    m_register[VF] = 0;

  m_register[x_reg] = m_register[y_reg] - m_register[x_reg];
  m_PC += NEXT; // TODO
}

// 8xyE - SHL Vx, {, Vy}

void Chip8::Shl(int opcode)
{
  int x_reg = XMASK(opcode);

  m_register[VF] = m_register[x_reg] >> 7;
  m_register[x_reg] <<= 1;

  m_PC += NEXT;
}

// 9xy0 - SNE Vx, Vy

void Chip8::Sne_Reg(int opcode)
{
  int x_reg = XMASK(opcode);
  int y_reg = YMASK(opcode);

  if (m_register[x_reg] != m_register[y_reg])
   m_PC += 2 * NEXT;
  else
   m_PC += NEXT;
}

// Annn - LD I, addr

void Chip8::Ld_I(int opcode)
{
  uint16_t address = opcode & 0x0FFF;

  m_I = address;
  m_PC += NEXT;
}

// Bnnn - JP V0, addr

void Chip8::Jp_Reg(int opcode)
{
  uint16_t address = opcode & 0x0FFF;

  if (address < ENTRYPOINT || address >= MEMORYSIZE)
  {
    error = ADDRESSERR;
    return ;
  }

  m_PC = m_register[V0] + address;
}

// Cxkk - RND, Vx, byte

void Chip8::Rnd(int opcode)
{
  int x_reg  = XMASK(opcode);
  int kk = (CONSTMASK(opcode));

  srand( time(NULL) );
  // abs for (rand)!!!!
  m_register[x_reg] = (rand() % 255) & kk;

  m_PC += NEXT;
}

void Chip8::Drw(int opcode)
{
  int x_reg = XMASK(opcode);
  int y_reg = YMASK(opcode);
  int n = (opcode & 0x000F);

  int startX = m_register[x_reg];
  int startY = m_register[y_reg];

  m_register[VF] = 0;
  for (int y = 0; y < n; y++)
  {
    int pixels = m_memory[m_I + y];
    for (int x = 0; x < 8; x++)
    {

      int *pixel = m_gfx + ind(startX + x, startY + y);

      if (pixels & (0x80 >> x))
      {
        if (*pixel)
          m_register[VF] = 1;
        *pixel ^= 1;
      }
    }
  }

  //drawFlag = true;

  /* PrintScreen */

  /*
  printf("VIDEO\n");

  printf("\n----------------------------\n");

  for (int i = 0; i < SCREENSIZE; i++)
  {
      if (i % 64 == 0)
        printf("\n");
      if (m_gfx[i] == 1)
        printf("*");
      else
        printf(".");
  }

  printf("\n\n----------------------------\n");
  */

  /* Test Variable TODO delete

  int check;

  scanf("%d", &check);
  */

  int t = 10000000;

  while (t)
    t--;

  m_PC += NEXT;

}

// Ex9E - SKP Vx

void Chip8::Skp(int opcode)
{
  int x_reg = XMASK(opcode);

  if (keyboard.isKeyPressed(m_register[x_reg]))
    m_PC += NEXT;

  m_PC += NEXT;
}

// ExA1 - SKNP Vx

void Chip8::Sknp(int opcode)
{
  int x_reg = XMASK(opcode);

  if (!keyboard.isKeyPressed(m_register[x_reg]))
    m_PC += NEXT;

  m_PC += NEXT;
}

// Fx07 - LD Vx, DT

void Chip8::Ld_Reg_Dt(int opcode)
{
  int x_reg = XMASK(opcode);

  m_register[x_reg] = m_DelayTimer;

  m_PC += NEXT;
}

// Fx0A - LD Vx, K

void Chip8::Ld_Key(int opcode)
{
  int x_reg = XMASK(opcode);

  int key = keyboard.isAnyKeyPressed();
  if (key != -1)
  {
    m_register[x_reg] = key;

    m_PC += NEXT;
  }
}

// Fx15 - LD DT, Vx

void Chip8::Ld_Dt(int opcode)
{
  int x_reg = XMASK(opcode);

  m_DelayTimer = m_register[x_reg];

  m_PC += NEXT;
}

// Fx18 - LD ST, Vx

void Chip8::Ld_St(int opcode)
{
  int x_reg = XMASK(opcode);
  m_SoundTimer = m_register[x_reg];

  m_PC += NEXT;
}

// Fx1E - ADD I, Vx

void Chip8::Add_I(int opcode)
{
  int x_reg = XMASK(opcode);

  m_I += m_register[x_reg];

  m_PC += NEXT;
}

// Fx29 - LD F, Vx

void Chip8::Ld_Spr(int opcode)
{
  int x_reg = XMASK(opcode);

  m_I = m_register[x_reg] * 0x5;

  m_PC += NEXT;
}

// Fx33 - LD B, Vx

void Chip8::Ld_Bcd(int opcode)
{
  int x_reg = XMASK(opcode);

  m_memory[m_I] = m_register[x_reg] / 100;
  m_memory[m_I + 1] = (m_register[x_reg] / 10) % 10;
  m_memory[m_I + 2] = (m_register[x_reg] % 10);

  m_PC += NEXT;
}

// Fx55 - LD [I], Vx

void Chip8::Ld_Reg_Mem(int opcode)
{
  int x_reg = XMASK(opcode);

  for (int i = 0; i <= x_reg; i++)
    m_memory[m_I + i] = m_register[i];

  m_I += x_reg + 1;

  m_PC += NEXT;
}

// FX65 - LD Vx, [I]

void Chip8::Ld_Reg_Load(int opcode)
{
  int x_reg = XMASK(opcode);

  for (int i = 0; i <= x_reg; i++)
    m_register[i] = m_memory[m_I + i];

  m_I += x_reg + 1;

  m_PC += NEXT;
}

/* End Of ListFunctions */

uint16_t Chip8::fetch()
{
  if ((m_memory[m_PC] == 0) && (m_memory[m_PC + 1] == 0))
    return 0;

  uint16_t result = m_memory[m_PC] << 8;
  result |= m_memory[m_PC + 1];
  return result;
}

uint16_t Chip8::decode(uint16_t cmd)
{
  uint8_t first  = (cmd >> 12);
  char second = (cmd & 0x0F00) >> 8;
  char third  = (cmd & 0x00F0) >> 4;
  char fourth = (cmd & 0x000F);

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
  for (int i = 0; i < 34; i++)
    if (decodedCmd == FSM[i].code)
    {
      transaction_callBack cw = FSM[i].worker;
      (this->*cw)(cmd);

      break;
    }
}

void Chip8::Run()
{
  sf::RenderWindow window(sf::VideoMode(640, 320), "Chip8");

  // clear the window with black color
  window.clear(sf::Color::White);

  //window.setVerticalSyncEnabled(true);

  double time1 = (double) clock() / CLOCKS_PER_SEC;

  // run the program as long as the window is open
  while (window.isOpen())
  {
    // check all the window's events that were triggered since the last iteration of the loop
    sf::Event event;


    while (window.pollEvent(event))
    {
      // "close requested" event: we close the window

      switch(event.type)
      {
        case(sf::Event::Closed):
          window.close();
          break;

        case(sf::Event::KeyPressed):
          switch(event.key.code)
          {
              case sf::Keyboard::Num1: keyboard.pressKey(0x1); break;
              case sf::Keyboard::Num2: keyboard.pressKey(0x2); break;
              case sf::Keyboard::Num3: keyboard.pressKey(0x3); break;
              case sf::Keyboard::Num4: keyboard.pressKey(0xc); break;
              case sf::Keyboard::Q: keyboard.pressKey(0x4); break;
              case sf::Keyboard::W: keyboard.pressKey(0x5); break;
              case sf::Keyboard::E: keyboard.pressKey(0x6); break;
              case sf::Keyboard::R: keyboard.pressKey(0xd); break;
              case sf::Keyboard::A: keyboard.pressKey(0x7); break;
              case sf::Keyboard::S: keyboard.pressKey(0x8); break;
              case sf::Keyboard::D: keyboard.pressKey(0x9); break;
              case sf::Keyboard::F: keyboard.pressKey(0xe); break;
              case sf::Keyboard::Z: keyboard.pressKey(0xa); break;
              case sf::Keyboard::X: keyboard.pressKey(0x0); break;
              case sf::Keyboard::C: keyboard.pressKey(0xb); break;
              case sf::Keyboard::V: keyboard.pressKey(0xf); break;
          }
          break;
        case(sf::Event::KeyReleased):
            switch(event.key.code)
            {
              case sf::Keyboard::Num1: keyboard.releaseKey(0x1); break;
              case sf::Keyboard::Num2: keyboard.releaseKey(0x2); break;
              case sf::Keyboard::Num3: keyboard.releaseKey(0x3); break;
              case sf::Keyboard::Num4: keyboard.releaseKey(0xc); break;
              case sf::Keyboard::Q: keyboard.releaseKey(0x4); break;
              case sf::Keyboard::W: keyboard.releaseKey(0x5); break;
              case sf::Keyboard::E: keyboard.releaseKey(0x6); break;
              case sf::Keyboard::R: keyboard.releaseKey(0xd); break;
              case sf::Keyboard::A: keyboard.releaseKey(0x7); break;
              case sf::Keyboard::S: keyboard.releaseKey(0x8); break;
              case sf::Keyboard::D: keyboard.releaseKey(0x9); break;
              case sf::Keyboard::F: keyboard.releaseKey(0xe); break;
              case sf::Keyboard::Z: keyboard.releaseKey(0xa); break;
              case sf::Keyboard::X: keyboard.releaseKey(0x0); break;
              case sf::Keyboard::C: keyboard.releaseKey(0xb); break;
              case sf::Keyboard::V: keyboard.releaseKey(0xf); break;

            }
            break;
        }
    }

    doCycle();

    double time2 = (double) clock() / CLOCKS_PER_SEC;

    if (time2 - time1 >=  1.0/60.0)
    {

      if (m_DelayTimer > 0)
        --m_DelayTimer;

      if (m_SoundTimer > 0)
        --m_SoundTimer;

        time1 = (double) clock() / CLOCKS_PER_SEC;

    //if (drawFlag)
    //{

      sf::RectangleShape rectangle;
      rectangle.setSize(sf::Vector2f(10, 10));

      for (int i = 0; i < SCREENSIZE; i++)
      {
        rectangle.setPosition((i % 64) * 10, (i / 64) * 10);
        if (m_gfx[i] == 1)
          rectangle.setFillColor(sf::Color::Black);
        else
          rectangle.setFillColor(sf::Color::White);

        window.draw(rectangle);
      }

    }

      // end the current frame
      window.display();
      //usleep(1000000);

  //    drawFlag = false;
    }
  //}

}
