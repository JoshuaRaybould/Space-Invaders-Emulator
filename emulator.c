#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct ConditionCodes {
    uint8_t    z:1;
    uint8_t    s:1;
    uint8_t    p:1;
    uint8_t    cy:1;
    uint8_t    ac:1;
    uint8_t    pad:3;
} ConditionCodes;

typedef struct State8080 {
    uint8_t    a;
    uint8_t    b;
    uint8_t    c;
    uint8_t    d;
    uint8_t    e;
    uint8_t    h;
    uint8_t    l;
    uint16_t    sp;
    uint16_t    pc;
    uint8_t     *memory;
    struct      ConditionCodes      cc;
    uint8_t     int_enable;
} State8080;

char fullText[65536];
int codeNum = 0;
int counter = 0;

void Disassemble8080p(uint8_t *codebuffer, int pc);

int Parity(uint8_t answer)
{
    int counter = 0;
    for (int i = 0; i < 8; i++)
    {
        if (answer & 0x01)
        {
            counter++;
        }
        answer = answer >> 1;
    }
    if (counter % 2 == 0)
    {
        return 1;
    }
    return 0;
}

unsigned char HexToUnsigned(char* curByte)
{
    unsigned char byte = 0;
    for (int i = 0; i < 2; i++) {
        char c = curByte[i];
        byte <<= 4;
        if (c >= '0' && c <= '9') {
            byte |= (c - '0');
        } else if (c >= 'a' && c <= 'f') {
            byte |= (c - 'a' + 10);
        }
    }
    free((void*)curByte);
    return byte;
}

unsigned char NextByte(char curLine[], int pos)
{
    char* curByte = malloc(3 * sizeof(char));
    curByte[0] = curLine[pos++];
    curByte[1] = curLine[pos++];
    curByte[2] = '\0';
    return HexToUnsigned(curByte);
}


void UnimplenetedInstruction(State8080* state)
{
    printf("Error: UnimplenetedInstruction");
    exit(1);
}

void Emulate8080p(State8080* state)
{
    uint16_t offset;
    unsigned char* opcode = &state->memory[state->pc];

    Disassemble8080p(state->memory, state->pc);

    switch(*opcode)
    {
        case 0x00: break;
        case 0x01:
        {
            state->b = opcode[2];
            state->c = opcode[1];
            state->pc += 2;
            break;
        }
        case 0x02:
        {
            offset = (state->b << 8) | state->c;
            state->memory[offset] = state->a;
            break;
        }
        case 0x03:
        {
            uint16_t bc = (state->b << 8) | (state->c);
            bc += 1;
            state->c = bc & 0xff;
            state->b = (bc >> 8) & 0xff;
            break;
        }
        case 0x04:
        {
            uint16_t answer = (uint16_t) state->b + 1;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.ac = (((uint16_t) state->b & 0x0f) + 1) > 0x0f;
            state->b = answer;
            break;
        }
        case 0x05:
        {
            uint16_t answer = (uint16_t) state->b - 1;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);

            state->b = answer;
            break;
        }
        case 0x06:
        {
            state->b = opcode[1];
            state->pc++;
            break;
        }
        case 0x07:
        {
            uint8_t x = state->a;
            state->a = (x << 1) | (x >> 7) & 0x01;
            state->cc.cy = (x >> 7) & 0x01;
            break;
        }
        case 0x08:
            break;
        case 0x09:
        {
            uint16_t bc = (state->b << 8) | state->c;
            uint16_t hl = (state->h << 8) | state->l;
            uint16_t total = bc + hl;
            state->cc.cy = (total > 0xffff);
            state->l = total & 0xff;
            state->h = (total >> 8) & 0xff;
            break;
        }
        case 0x0a:
        {
            offset = (state->b << 8) |state->c;
            state->a = state->memory[offset];
            break;
        }
        case 0x0b:
        {
            uint16_t bc = (state->b << 8) | (state->c);
            bc -= 1;
            state->c = bc & 0xff;
            state->b = (bc >> 8) & 0xff;
            break;
        }
        case 0x0c:
        {
            uint16_t answer = (uint16_t) state->c + 1;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.ac = (((uint16_t) state->c & 0x0f) + 1) > 0x0f;
            state->c = answer;
            break;
        }
        case 0x0d:
        {
            uint16_t answer = (uint16_t) state->c - 1;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);

            state->c = answer;
            break;
        }
        case 0x0e:
        {
            state->c = opcode[1];
            state->pc++;
            break;
        }
        case 0x0f:
        {
            uint8_t x = state->a;
            state->a = ((x & 1) << 7) | (x >> 1);
            state->cc.cy = (1 == (x & 1));
            break;
        }
        case 0x10:
            break;
        case 0x11:
        {
            state->d = opcode[2];
            state->e = opcode[1];
            state->pc += 2;
            break;
        }
        case 0x12:
        {
            offset = (state->d << 8) | state->e;
            state->memory[offset] = state->a;
            break;
        }
        case 0x13:
        {
            uint16_t de = (state->d << 8) | (state->e);
            de += 1;
            state->e = de & 0xff;
            state->d = (de >> 8) & 0xff;
            break;
        }
        case 0x14:
        {
            uint16_t answer = (uint16_t) state->d + 1;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.ac = (((uint16_t) state->d & 0x0f) + 1) > 0x0f;
            state->d = answer;
            break;
        }
        case 0x15:
        {
            uint16_t answer = (uint16_t) state->d - 1;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);

            state->d = answer;
            break;
        }
        case 0x16:
        {
            state->d = opcode[1];
            state->pc++;
            break;
        }
        case 0x17:
        {
            uint8_t x = state->a;
            state->a = (x << 1) | (state->cc.cy);
            state->cc.cy = (x >> 7) & 0x01;
            break;
        }
        case 0x18:
            break;
        case 0x19:
        {
            uint16_t de = (state->d << 8) | state->e;
            uint16_t hl = (state->h << 8) | state->l;
            uint16_t total = de + hl;
            state->cc.cy = (total > 0xffff);
            state->l = total & 0xff;
            state->h = (total >> 8) & 0xff;
            break;
        }
        case 0x1a:
        {
            offset = (state->d << 8) | state->e;
            state->a = state->memory[offset];
            break;
        }
        case 0x1b:
        {
            uint16_t de = (state->d << 8) | (state->e);
            de -= 1;
            state->e = de & 0xff;
            state->d = (de >> 8) & 0xff;
            break;
        }
        case 0x1c:
        {
            uint16_t answer = (uint16_t) state->e + 1;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.ac = (((uint16_t) state->e & 0x0f) + 1) > 0x0f;
            state->e = answer;
            break;
        }
        case 0x1d:
        {
            uint16_t answer = (uint16_t) state->e - 1;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);

            state->e = answer;
            break;
        }
        case 0x1e:
        {
            state->e = opcode[1];
            state->pc++;
            break;
        }
        case 0x1f:
        {
            uint8_t x = state->a;
            state->a = (state->cc.cy << 7) |(x >> 1);
            state->cc.cy = (x & 0x01);
            break;
        }
        case 0x20:
            break;
        case 0x21:
        {
            state->h = opcode[2];
            state->l = opcode[1];
            state->pc += 2;
            break;
        }
        case 0x22:
        {
            offset = (opcode[2] << 8) | opcode[1];
            state->memory[offset] = state->l;
            state->memory[offset + 1] = state->h;
            state->pc += 2;
            break;
        }
        case 0x23:
        {
            uint16_t hl = (state->h << 8) | (state->l);
            hl += 1;
            state->l = hl & 0xff;
            state->h = (hl >> 8) & 0xff;
            break;
        }
        case 0x24:
        {
            uint16_t answer = (uint16_t) state->h + 1;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.ac = (((uint16_t) state->h & 0x0f) + 1) > 0x0f;
            state->h = answer;
            break;
        }
        case 0x25:
        {
            uint16_t answer = (uint16_t) state->h - 1;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->h = answer;
            break;
        }
        case 0x26:
        {
            state->h = opcode[1];
            state->pc++;
            break;
        }
        case 0x27: UnimplenetedInstruction(state);
        break;
        case 0x28:
            break;
        case 0x29:
        {
            uint16_t hi = (state->h << 8) | state->l;
            uint16_t hl = (state->h << 8) | state->l;
            uint16_t total = hi + hl;
            state->cc.cy = (total > 0xffff);
            state->l = total & 0xff;
            state->h = (total >> 8) & 0xff;
            break;
        }
        case 0x2a:
        {
            offset = (opcode[2] << 8) | opcode[1];
            state->l = state->memory[offset];
            state->h = state->memory[offset + 1];
            state->pc += 2;
            break;
        }
        case 0x2b:
        {
            uint16_t hl = (state->h << 8) | (state->l);
            hl -= 1;
            state->l = hl & 0xff;
            state->h = (hl >> 8) & 0xff;
            break;
        }
        case 0x2c:
        {
            uint16_t answer = (uint16_t) state->l + 1;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.ac = (((uint16_t) state->l & 0x0f) + 1) > 0x0f;
            state->l = answer;
            break;
        }
        case 0x2d:
        {
            uint16_t answer = (uint16_t) state->l - 1;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);

            state->l = answer;
            break;
        }
        case 0x2e:
        {
            state->l = opcode[1];
            state->pc++;
            break;
        }
        case 0x2f:
            state->a = ~state->a;
            break;
        case 0x30:
            break;
        case 0x31:
        {
            state->sp = (opcode[2] << 8) | opcode[1];
            state->pc += 2;
            break;
        }
        case 0x32:
        {
            offset = (opcode[2] << 8) | opcode[1];
            state->memory[offset] = state->a;
            state->pc += 2;
            break;
        }
        case 0x33:
            state->sp += 1;
            break;
        case 0x34:
        {
            offset = (state->h << 8) | (state->l);
            uint16_t answer = state->memory[offset] + 1;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.ac = ((state->memory[offset] & 0x0f) + 1) > 0x0f;
            state->memory[offset] = answer;
            break;
        }
        case 0x35:
        {
            offset = (state->h << 8) | state->l;
            uint16_t answer = state->memory[offset] - 1;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);

            state->memory[offset] = answer;
            break;
        }
        case 0x36:
        {
            offset = (state->h << 8) | state->l;
            state->memory[offset] = opcode[1];
            state->pc++;
            break;
        }
        case 0x37:
        {
            state->cc.cy = 1;
            break;
        }
        case 0x38:
            break;
        case 0x39:
        {
            uint16_t hl = (state->h << 8) | state->l;
            uint16_t total = state->sp + hl;
            state->cc.cy = (total > 0xffff);
            state->l = total & 0xff;
            state->h = (total >> 8) & 0xff;
            break;
        }
        case 0x3a:
        {
            offset = (opcode[2] << 8) | opcode[1];
            state->a = state->memory[offset];
            state->pc += 2;
            break;
        }
        case 0x3b:
            state->sp -= 1;
            break;
        case 0x3c:
        {
            uint16_t answer = (uint16_t) state->a + 1;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.ac = (((uint16_t) state->a & 0x0f) + 1) > 0x0f;
            state->a = answer;
            break;
        }
        case 0x3d:
        {
            uint16_t answer = (uint16_t) state->a - 1;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);

            state->a = answer;
            break;
        }
        case 0x3e:
        {
            state->a = opcode[1];
            state->pc++;
            break;
        }
        case 0x3f:
        {
            state->cc.cy = !(state->cc.cy);
            break;
        }
        case 0x40:
            state->b = state->b;
            break;
        case 0x41:
            state->b = state->c;
            break;
        case 0x42:
            state->b = state->d;
            break;
        case 0x43:
            state->b = state->e;
            break;
        case 0x44:
            state->b = state->h;
            break;
        case 0x45:
            state->b = state->l;
            break;
        case 0x46:
            offset = (state->h << 8) | state->l;
            state->b = state->memory[offset];
            break;
        case 0x47:
            state->b = state->a;
            break;
        case 0x48:
            state->c = state->b;
            break;
        case 0x49:
            state->c = state->c;
            break;
        case 0x4a:
            state->c = state->d;
            break;
        case 0x4b:
            state->c = state->e;
        break;
        case 0x4c:
            state->c = state->h;
            break;
        case 0x4d:
            state->c = state->l;
            break;
        case 0x4e:
            offset = (state->h << 8) | state->l;
            state->c = state->memory[offset];
            break;
        case 0x4f:
            state->c = state->a;
            break;
        case 0x50:
            state->d = state->b;
            break;
        case 0x51:
            state->d = state->c;
            break;
        case 0x52:
            state->d = state->d;
            break;
        case 0x53:
            state->d = state->e;
            break;
        case 0x54:
            state->d = state->h;
            break;
        case 0x55:
            state->d = state->l;
            break;
        case 0x56:
            offset = (state->h << 8) | state->l;
            state->d = state->memory[offset];
            break;
        case 0x57:
            state->d = state->a;
            break;
        case 0x58:
            state->e = state->b;
            break;
        case 0x59:
            state->e = state->c;
            break;
        case 0x5a:
            state->e = state->d;
            break;
        case 0x5b:
            state->e = state->e;
            break;
        case 0x5c:
            state->e = state->h;
            break;
        case 0x5d:
            state->e = state->l;
            break;
        case 0x5e:
            offset = (state->h << 8) | state->l;
            state->e = state->memory[offset];
            break;
        case 0x5f:
            state->e = state->a;
            break;
        case 0x60:
            state->h = state->b;
            break;
        case 0x61:
            state->h = state->c;
            break;
        case 0x62:
            state->h = state->d;
            break;
        case 0x63:
            state->h = state->e;
            break;
        case 0x64:
            state->h = state->h;
            break;
        case 0x65:
            state->h = state->l;
            break;
        case 0x66:
            offset = (state->h << 8) | state->l;
            state->h = state->memory[offset];
            break;
        case 0x67:
            state->h = state->a;
            break;
        case 0x68:
            state->l = state->b;
            break;
        case 0x69:
            state->l = state->c;
            break;
        case 0x6a:
            state->l = state->d;
            break;
        case 0x6b:
            state->l = state->e;
            break;
        case 0x6c:
            state->l = state->h;
            break;
        case 0x6d:
            state->l = state->l;
            break;
        case 0x6e:
            offset = (state->h << 8) | state->l;
            state->l = state->memory[offset];
            break;
        case 0x6f:
            state->l = state->a;
            break;
        case 0x70:
            offset = (state->h << 8) | state->l;
            state->memory[offset] = state->b;
            break;
        case 0x71:
            offset = (state->h << 8) | state->l;
            state->memory[offset] = state->c;
            break;
        case 0x72:
            offset = (state->h << 8) | state->l;
            state->memory[offset] = state->d;
            break;
        case 0x73:
            offset = (state->h << 8) | state->l;
            state->memory[offset] = state->e;
            break;
        case 0x74:
            offset = (state->h << 8) | state->l;
            state->memory[offset] = state->h;
            break;
        case 0x75:
            offset = (state->h << 8) | state->l;
            state->memory[offset] = state->l;
            break;
        case 0x76:
            exit(0);
            break;
        case 0x77:
            offset = (state->h << 8) | state->l;
            state->memory[offset] = state->a;
            break;
        case 0x78:
            state->a = state->b;
            break;
        case 0x79:
            state->a = state->c;
            break;
        case 0x7a:
            state->a = state->d;
            break;
        case 0x7b:
            state->a = state->e;
            break;
        case 0x7c:
            state->a = state->h;
            break;
        case 0x7d:
            state->a = state->l;
            break;
        case 0x7e:
            offset = (state->h << 8) | state->l;
            state->a = state->memory[offset];
            break;
        case 0x7f:
            state->a = state->a;
        case 0x80:
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->b;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (answer > 0xff);
            state->cc.ac = ((state->a & 0xf) + (state->b & 0xf)) > 0xf;
            state->a = answer & 0xff;
        }
        case 0x81:
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->c;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (answer > 0xff);
            state->cc.ac = ((state->a & 0xf) + (state->c & 0xf)) > 0xf;
            state->a = (answer & 0xff);
        }
        case 0x82:
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->d;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (answer > 0xff);
            state->cc.ac = ((state->a & 0xf) + (state->d & 0xf)) > 0xf;
            state->a = (answer & 0xff);
        }
        case 0x83:
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->e;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (answer > 0xff);
            state->cc.ac = ((state->a & 0xf) + (state->e & 0xf)) > 0xf;
            state->a = (answer & 0xff);
        }
        case 0x84:
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->h;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (answer > 0xff);
            state->cc.ac = ((state->a & 0xf) + (state->h & 0xf)) > 0xf;
            state->a = (answer & 0xff);
        }
        case 0x85:
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->l;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (answer > 0xff);
            state->cc.ac = ((state->a & 0xf) + (state->l & 0xf)) > 0xf;
            state->a = (answer & 0xff);
        }
        case 0x86:
        {
            offset = (state->h << 8) | (state->l);
            uint16_t answer = (uint16_t) state->a + state->memory[offset];
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (answer > 0xff);
            state->cc.ac = ((state->a & 0xf) + (state->memory[offset] & 0xf)) > 0xf;
            state->a = (answer & 0xff);
        }
        case 0x87:
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->a;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (answer > 0xff);
            state->cc.ac = ((state->a & 0xf) + (state->a & 0xf)) > 0xf;
            state->a = (answer & 0xff);
        }
        case 0x88:
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->b + (uint16_t) state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (answer > 0xff);
            state->cc.ac = ((state->a & 0xf) + (state->b & 0xf) + state->cc.cy) > 0xf;
            state->a = (answer & 0xff);
        }
        case 0x89:
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->c + (uint16_t) state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (answer > 0xff);
            state->cc.ac = ((state->a & 0xf) + (state->c & 0xf) + state->cc.cy) > 0xf;
            state->a = (answer & 0xff);
        }
        case 0x8a:
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->d + (uint16_t) state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (answer > 0xff);
            state->cc.ac = ((state->a & 0xf) + (state->d & 0xf) + state->cc.cy) > 0xf;
            state->a = (answer & 0xff);
        }
        case 0x8b:
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->e + (uint16_t) state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (answer > 0xff);
            state->cc.ac = ((state->a & 0xf) + (state->e & 0xf) + state->cc.cy) > 0xf;
            state->a = (answer & 0xff);
        }
        case 0x8c:
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->h + (uint16_t) state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (answer > 0xff);
            state->cc.ac = ((state->a & 0xf) + (state->h & 0xf) + state->cc.cy) > 0xf;
            state->a = (answer & 0xff);
        }
        case 0x8d:
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->l + (uint16_t) state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (answer > 0xff);
            state->cc.ac = ((state->a & 0xf) + (state->l & 0xf) + state->cc.cy) > 0xf;
            state->a = (answer & 0xff);
        }
        case 0x8e:
        {
            offset = (state->h << 8) | (state->l);
            uint16_t answer = (uint16_t) state->a + state->memory[offset] + (uint16_t) state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (answer > 0xff);
            state->cc.ac = ((state->a & 0xf) + (state->memory[offset] & 0xf) + state->cc.cy) > 0xf;
            state->a = (answer & 0xff);
        }
        case 0x8f:
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->a + (uint16_t) state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (answer > 0xff);
            state->cc.ac = ((state->a & 0x0f) + (state->a & 0x0f) + state->cc.cy) > 0x0f;
            state->a = (answer & 0xff);
        }
        case 0x90:
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->b;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (state->a < state->b);
            state->a = (answer & 0xff);
        }
        case 0x91:
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->c;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (state->a < state->c);
            state->a = (answer & 0xff);
        }
        case 0x92:
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->d;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (state->a < state->d);
            state->a = (answer & 0xff);
        }
        case 0x93:
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->e;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (state->a < state->e);
            state->a = (answer & 0xff);
        }
        case 0x94:
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->h;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (state->a < state->h);
            state->a = (answer & 0xff);
        }
        case 0x95:
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->l;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (state->a < state->l);
            state->a = (answer & 0xff);
        }
        case 0x96:
        {
            offset = (state->h << 8) | (state->l);
            uint16_t answer = (uint16_t) state->a - state->memory[offset];
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (state->a < state->memory[offset]);
            state->a = (answer & 0xff);
        }
        case 0x97:
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->a;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (state->a < state->a);
            state->a = (answer & 0xff);
        }
        case 0x98:
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->b - (uint16_t) state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (state->a < (state->b + state->cc.cy));
            state->a = (answer & 0xff);
        }
        case 0x99:
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->c - (uint16_t) state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (state->a < (state->c + state->cc.cy));
            state->a = (answer & 0xff);
        }
        case 0x9a:
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->d - (uint16_t) state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (state->a < (state->d + state->cc.cy));
            state->a = (answer & 0xff);
        }
        case 0x9b:
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->e - (uint16_t) state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (state->a < (state->e + state->cc.cy));
            state->a = (answer & 0xff);
        }
        case 0x9c:
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->h - (uint16_t) state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (state->a < (state->h + state->cc.cy));
            state->a = (answer & 0xff);
        }
        case 0x9d:
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->l - (uint16_t) state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (state->a < (state->l + state->cc.cy));
            state->a = (answer & 0xff);
        }
        case 0x9e:
        {
            offset = (state->h << 8) | (state->l);
            uint16_t answer = (uint16_t) state->a - state->memory[offset] - (uint16_t) state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (state->a < (state->h + state->cc.cy));
            state->a = (answer & 0xff);
        }
        case 0x9f:
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->a - (uint16_t) state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (state->a < (state->a + state->cc.cy));
            state->a = (answer & 0xff);
        }
        case 0xa0:
        {
            uint8_t ans = state->a & state->b;
            state->cc.z = (ans == 0);
            state->cc.s = (0x80 == (ans & 0x80));
            state->cc.p = Parity(ans);
            state->cc.cy = 0;
            state->a = ans;
            break;
        }
        case 0xa1:
        {
            uint8_t ans = state->a & state->c;
            state->cc.z = (ans == 0);
            state->cc.s = (0x80 == (ans & 0x80));
            state->cc.p = Parity(ans);
            state->cc.cy = 0;
            state->a = ans;
            break;
        }
        case 0xa2:
        {
            uint8_t ans = state->a & state->d;
            state->cc.z = (ans == 0);
            state->cc.s = (0x80 == (ans & 0x80));
            state->cc.p = Parity(ans);
            state->cc.cy = 0;
            state->a = ans;
            break;
        }
        case 0xa3:
        {
            uint8_t ans = state->a & state->e;
            state->cc.z = (ans == 0);
            state->cc.s = (0x80 == (ans & 0x80));
            state->cc.p = Parity(ans);
            state->cc.cy = 0;
            state->a = ans;
            break;
        }
        case 0xa4:
        {
            uint8_t ans = state->a & state->h;
            state->cc.z = (ans == 0);
            state->cc.s = (0x80 == (ans & 0x80));
            state->cc.p = Parity(ans);
            state->cc.cy = 0;
            state->a = ans;
            break;
        }
        case 0xa5:
        {
            uint8_t ans = state->a & state->l;
            state->cc.z = (ans == 0);
            state->cc.s = (0x80 == (ans & 0x80));
            state->cc.p = Parity(ans);
            state->cc.cy = 0;
            state->a = ans;
            break;
        }
        case 0xa6:
        {
            offset = (state->h << 8) | state->l;
            uint8_t ans = state->a & state->memory[offset];
            state->cc.z = (ans == 0);
            state->cc.s = (0x80 == (ans & 0x80));
            state->cc.p = Parity(ans);
            state->cc.cy = 0;
            state->a = ans;
            break;
        }
        case 0xa7:
        {
            uint8_t ans = state->a & state->a;
            state->cc.z = (ans == 0);
            state->cc.s = (0x80 == (ans & 0x80));
            state->cc.p = Parity(ans);
            state->cc.cy = 0;
            state->a = ans;
            break;
        }
        case 0xa8:
        {
            uint8_t x = state->a ^ state->b;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = 0;
            state->cc.ac = 0;
            state->a = x;
            break;
        }
        case 0xa9:
        {
            uint8_t x = state->a ^ state->c;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = 0;
            state->cc.ac = 0;
            state->a = x;
            break;
        }
        case 0xaa:
        {
            uint8_t x = state->a ^ state->d;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = 0;
            state->cc.ac = 0;
            state->a = x;
            break;
        }
        case 0xab:
        {
            uint8_t x = state->a ^ state->e;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = 0;
            state->cc.ac = 0;
            state->a = x;
            break;
        }
        case 0xac:
        {
            uint8_t x = state->a ^ state->h;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = 0;
            state->cc.ac = 0;
            state->a = x;
            break;
        }
        case 0xad:
        {
            uint8_t x = state->a ^ state->l;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = 0;
            state->cc.ac = 0;
            state->a = x;
            break;
        }
        case 0xae:
        {
            offset = (state->h << 8) | state->l;
            uint8_t x = state->a ^ state->memory[offset];
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = 0;
            state->cc.ac = 0;
            state->a = x;
            break;
        }
        case 0xaf:
        {
            uint8_t x = state->a ^ state->a;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = 0;
            state->cc.ac = 0;
            state->a = x;
            break;
        }
        case 0xb0:
        {
            uint8_t x = state->a | state->b;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = 0;
            state->cc.ac = 0;
            state->a = x;
            break;
        }
        case 0xb1:
        {
            uint8_t x = state->a | state->c;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = 0;
            state->cc.ac = 0;
            state->a = x;
            break;
        }
        case 0xb2:
        {
            uint8_t x = state->a | state->d;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = 0;
            state->cc.ac = 0;
            state->a = x;
            break;
        }
        case 0xb3:
        {
            uint8_t x = state->a | state->e;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = 0;
            state->cc.ac = 0;
            state->a = x;
            break;
        }
        case 0xb4:
        {
            uint8_t x = state->a | state->h;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = 0;
            state->cc.ac = 0;
            state->a = x;
            break;
        }
        case 0xb5:
        {
            uint8_t x = state->a | state->l;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = 0;
            state->cc.ac = 0;
            state->a = x;
            break;
        }
        case 0xb6:
        {
            offset = (state-> h << 8) | state->l;
            uint8_t x = state->a | state->memory[offset];
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = 0;
            state->cc.ac = 0;
            state->a = x;
            break;
        }
        case 0xb7:
        {
            uint8_t x = state->a | state->a;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = 0;
            state->cc.ac = 0;
            state->a = x;
            break;
        }
        case 0xb8:
        {
            uint16_t x = (uint16_t) state->a - (uint16_t) state->b;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = (state->b > state->a);
            break;
        }
        case 0xb9:
        {
            uint16_t x = (uint16_t) state->a - (uint16_t) state->c;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = (state->c > state->a);
            break;
        }
        case 0xba:
        {
            uint16_t x = (uint16_t) state->a - (uint16_t) state->d;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = (state->d > state->a);
            break;
        }
        case 0xbb:
        {
            uint16_t x = (uint16_t) state->a - (uint16_t) state->e;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = (state->e > state->a);
            break;
        }
        case 0xbc:
        {
            uint16_t x = (uint16_t) state->a - (uint16_t) state->h;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = (state->h > state->a);
            break;
        }
        case 0xbd:
        {
            uint16_t x = (uint16_t) state->a - (uint16_t) state->l;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = (state->l > state->a);
            break;
        }
        case 0xbe:
        {
            offset = (state->h << 8) | state->l;
            uint16_t x = (uint16_t) state->a - (uint16_t) state->memory[offset];
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = (state->memory[offset] > state->a);
            break;
        }
        case 0xbf:
        {
            uint16_t x = (uint16_t) state->a - (uint16_t) state->a;
            state->cc.z = (x == 0);
            state->cc.s = (x & 0x80) != 0;
            state->cc.p = Parity(x & 0xff);
            state->cc.cy = (state->a > state->a);
            break;
        }
        case 0xc0:
        {
            if (state->cc.z == 0)
            {
                state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
                state->pc--;
                state->sp += 2;
            }
            break;
        }
        case 0xc1:
        {
            state->c = state->memory[state->sp];
            state->b = state->memory[state->sp + 1];
            state->sp += 2;
            break;
        }
        case 0xc2:
            if (state->cc.z == 0)
            {
                state->pc = (opcode[2] << 8) | opcode[1];
                state->pc--;
            }
            else
                state->pc += 2;
            break;
        case 0xc3:
            state->pc = (opcode[2] << 8) | opcode[1];
            state->pc--;
            break;
        case 0xc4:
        {
            if (state->cc.z == 0) {
                uint16_t ret = state->pc + 2;
                state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                state->memory[state->sp - 2] = (ret & 0xff);
                state->sp = state->sp - 2;
                state->pc = (opcode[2] << 8) | opcode[1];
                state->pc--;
            }
            else
            {
                state->pc += 2;
            }
        }
            break;
        case 0xc5:
        {
            state->memory[state->sp - 1] = state->b;
            state->memory[state->sp - 2] = state->c;
            state->sp -= 2;
            break;
        }
        case 0xc6:
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) opcode[1];
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (answer > 0xff);
            state->a = answer & 0xff;
            state->pc += 1;
            break;
        }
        case 0xc7:
        {
            uint16_t ret = state->pc;
            state->memory[state->sp - 1] = (ret >> 8) &  0xff;
            state->memory[state->sp - 2] = (ret &  0xff);
            state->sp = state->sp - 2;
            state->pc = 0x00;
            state->pc--;
            break;
        }
        case 0xc8:
        {
            if (state->cc.z == 1)
            {
                state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
                state->sp += 2;
            }
            break;
        }

        case 0xc9:
        {
            state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
            state->pc--;
            state->sp += 2;
            break;
        }
        case 0xca:
            if (state->cc.z == 1)
            {
                state->pc = (opcode[2] << 8) | opcode[1];
                state->pc--;
            }
            else
                state->pc += 2;
            break;
        case 0xcb:
            break;
        case 0xcc:
        {
            if (state->cc.z == 1)
            {
                uint16_t ret = state->pc + 2;
                state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                state->memory[state->sp - 2] = (ret & 0xff);
                state->sp = state->sp - 2;
                state->pc = (opcode[2] << 8) | opcode[1];
                state->pc--;
            }
            else
            {
                state->pc += 2;
            }
            break;
        }
        case 0xcd:
        {
            uint16_t ret = state->pc+3;
            state->memory[state->sp - 1] = (ret >> 8) & 0xff;
            state->memory[state->sp - 2] = (ret & 0xff);
            state->sp = state->sp - 2;
            state->pc = (opcode[2] << 8) | opcode[1];
            state->pc--;
            break;
        }
        case 0xce:
        {
            uint16_t answer = state->a + opcode[1] + state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (answer > 0xff);
            state->cc.ac = ((state->a & 0x0f) + opcode[1] + (state->cc.cy)) > 0x0f;
            state->a = answer;
            state->pc += 1;
            break;
        }
        case 0xcf:
        {
            uint16_t ret = state->pc;
            state->memory[state->sp - 1] = (ret >> 8) &  0xff;
            state->memory[state->sp - 2] = (ret &  0xff);
            state->sp = state->sp - 2;
            state->pc = 0x08;
            state->pc--;
            break;
        }
        case 0xd0:
        {
            if (state->cc.cy == 0)
            {
                state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
                state->pc--;
                state->sp += 2;
            }
            break;
        }
        case 0xd1:
        {
            state->e = state->memory[state->sp];
            state->d = state->memory[state->sp + 1];
            state->sp += 2;
            break;
        }
        case 0xd2:
            if (state->cc.cy == 0)
            {
                state->pc = (opcode[2] << 8) | opcode[1];
                state->pc--;
            }
            else
                state->pc += 2;
            break;
        case 0xd3:
        {
            state->pc++;
            break;
        }
        case 0xd4:
        {
            if (state->cc.cy == 0)
            {
                uint16_t ret = state->pc + 2;
                state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                state->memory[state->sp - 2] = (ret & 0xff);
                state->sp = state->sp - 2;
                state->pc = (opcode[2] << 8) | opcode[1];
                state->pc--;
            }
            else
            {
                state->pc += 2;
            }
            break;
        }
        break;
        case 0xd5:
        {
            state->memory[state->sp - 1] = state->d;
            state->memory[state->sp - 2] = state->e;
            state->sp -= 2;
            break;
        }
        case 0xd6:
        {
            uint16_t answer = (uint16_t) state->a - opcode[1];
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = (state->a < opcode[1]);
            state->a = answer & 0xff;
            state->pc += 1;
            break;
        }
        case 0xd7:
        {
            uint16_t ret = state->pc;
            state->memory[state->sp - 1] = (ret >> 8) &  0xff;
            state->memory[state->sp - 2] = (ret &  0xff);
            state->sp = state->sp - 2;
            state->pc = 0x10;
            state->pc--;
            break;
        }
        case 0xd8:
        {
            if (state->cc.cy == 1)
            {
                state->pc = (state->memory[state->sp  + 1] << 8) | state->memory[state->sp];
                state->pc--;
                state->sp += 2;
            }
            break;
        }
        case 0xd9:
            break;
        case 0xda:
            if (state->cc.cy == 1)
            {
                state->pc = (opcode[2] << 8) | opcode[1];
                state->pc--;
            }
            else
                state->pc += 2;
            break;
        case 0xdb:
        {
            state->pc++;
            break;
        }
        case 0xdc:
        {
            if (state->cc.cy == 1)
            {
                uint16_t ret = state->pc + 2;
                state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                state->memory[state->sp - 2] = (ret & 0xff);
                state->sp = state->sp - 2;
                state->pc = (opcode[2] << 8) | opcode[1];
                state->pc--;
            }
            else
            {
                state->pc += 2;
            }
            break;
        }
        case 0xdd:
            break;
        case 0xde:
        {
            uint16_t ans = (uint16_t) state->a - opcode[1] - state->cc.cy;
            state->cc.z = (ans == 0);
            state->cc.s = (0x80 == (ans & 0x80));
            state->cc.p = Parity(ans & 0xff);
            state->cc.cy = (state->a < (opcode[1] + state->cc.cy));
            state->pc++;
            state->a = ans;
            break;
        }
        case 0xdf:
        {
            uint16_t ret = state->pc + 1;
            state->memory[state->sp - 1] = (ret >> 8) &  0xff;
            state->memory[state->sp - 2] = (ret &  0xff);
            state->sp = state->sp - 2;
            state->pc = 0x18;
            state->pc--;
            break;
        }
        case 0xe0:
        {
            if (state->cc.p == 0)
            {
                state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
                state->pc--;
                state->sp += 2;
            }
            break;
        }
        case 0xe1:
        {
            state->l = state->memory[state->sp];
            state->h = state->memory[state->sp + 1];
            state->sp += 2;
            break;
        }
        case 0xe2:
            if (state->cc.p == 0)
            {
                state->pc = (opcode[2] << 8) | opcode[1];
                state->pc--;
            }
            else
                state->pc += 2;
            break;
        case 0xe3:
        {
            uint8_t tmpl = state->l;
            uint8_t tmph = state->h;
            state->l = state->memory[state->sp];
            state->h = state->memory[state->sp +  1];
            state->memory[state->sp] = tmpl;
            state->memory[state->sp + 1] = tmph;
            break;
        }
        case 0xe4:
        {
            if (state->cc.p == 0)
            {
                uint16_t ret = state->pc + 2;
                state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                state->memory[state->sp - 2] = (ret & 0xff);
                state->sp = state->sp - 2;
                state->pc = (opcode[2] << 8) | opcode[1];
                state->pc--;
            }
            else
            {
                state->pc += 2;
            }
            break;
        }
        case 0xe5:
        {
            state->memory[state->sp - 1] = state->h;
            state->memory[state->sp - 2] = state->l;
            state->sp -= 2;
            break;
        }
        case 0xe6:
        {
            uint8_t ans = state->a & opcode[1];
            state->cc.z = (ans == 0);
            state->cc.s = (0x80 == (ans & 0x80));
            state->cc.p = Parity(ans);
            state->cc.cy = 0;
            state->a = ans;
            state->pc++;
            break;
        }
        case 0xe7:
        {
            uint16_t ret = state->pc + 1;
            state->memory[state->sp - 1] = (ret >> 8) &  0xff;
            state->memory[state->sp - 2] = (ret &  0xff);
            state->sp = state->sp - 2;
            state->pc = 0x20;
            state->pc--;
            break;
        }
        case 0xe8:
        {
            if (state->cc.p == 1)
            {
                state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
                state->pc--;
                state->sp += 2;
            }
            break;
        }
        case 0xe9:
        {
            state->pc = (state->h << 8) | state->l;
            state->pc--;
            break;
        }
        case 0xea:
        {
            if (state->cc.p == 1)
            {
                state->pc = (opcode[2] << 8) | opcode[1];
                state->pc--;
            }
            else
                state->pc += 2;
            break;
        }
        case 0xeb:
        {
            uint8_t temph = state->h;
            uint8_t templ = state->l;
            state->h = state->d;
            state->l = state->e;
            state->d = temph;
            state->e = templ;
            break;
        }
        case 0xec:
        {
            if (state->cc.p == 1)
            {
                uint16_t ret = state->pc + 2;
                state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                state->memory[state->sp - 2] = (ret & 0xff);
                state->sp = state->sp - 2;
                state->pc = (opcode[2] << 8) | opcode[1];
                state->pc--;
            }
            else
            {
                state->pc += 2;
            }
            break;
        }
        case 0xed:
            break;
        case 0xee:
        {
            uint8_t answer = state->a ^ opcode[1];
            state->cc.z = (answer == 0);
            state->cc.s = (0x80 == (answer & 0x80));
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = 0;
            state->cc.ac = 0;
            state->pc++;
            state->a = answer;
            break;
        }
        case 0xef:
        {
            uint16_t ret = state->pc + 1;
            state->memory[state->sp - 1] = (ret >> 8) &  0xff;
            state->memory[state->sp - 2] = (ret &  0xff);
            state->sp = state->sp - 2;
            state->pc = 0x28;
            state->pc--;
            break;
        }
        case 0xf0:
        {
            if (state->cc.s == 0)
            {
                state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
                state->pc--;
                state->sp += 2;
            }
            break;
        }
        case 0xf1:
        {
            uint8_t psw = state->memory[state->sp];
            state->cc.z = (0x01 == (psw & 0x01));
            state->cc.s = (0x02 == (psw & 0x02));
            state->cc.p = (0x04 == (psw & 0x04));
            state->cc.cy = (0x08 == (psw & 0x08));
            state->cc.ac = (0x10 == (psw & 0x10));
            state->a = state->memory[state->sp + 1];
            state->sp += 2;
            break;
        }
        case 0xf2:
            if (state->cc.s == 0)
            {
                state->pc = (opcode[2] << 8) | opcode[1];
                state->pc--;
            }
            else
                state->pc += 2;
            break;
        case 0xf3:
            state->int_enable = 0;
            break;
        case 0xf4:
        {
            if (state->cc.s == 0)
            {
                uint16_t ret = state->pc + 2;
                state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                state->memory[state->sp - 2] = (ret & 0xff);
                state->sp = state->sp - 2;
                state->pc = (opcode[2] << 8) | opcode[1];
                state->pc--;
            }
            else
            {
                state->pc += 2;
            }
            break;
        }
        case 0xf5:
        {
            state->memory[state->sp - 1] = state->a;
            uint8_t psw = ((state->cc.z) | (state->cc.s << 1) |
                            (state->cc.p << 2) | (state->cc.cy << 3) |
                            (state->cc.ac << 4));
            state->memory[state->sp - 2] = psw;
            state->sp -= 2;
            break;
        }
        case 0xf6:
        {
            uint8_t answer = state->a | opcode[1];
            state->cc.z = (answer == 0);
            state->cc.s = (0x80 == (answer & 0x80));
            state->cc.p = Parity(answer & 0xff);
            state->cc.cy = 0;
            state->cc.ac = 0;
            state->pc++;
            state->a = answer;
            break;
        }
        case 0xf7:
        {
            uint16_t ret = state->pc + 1;
            state->memory[state->sp - 1] = (ret >> 8) &  0xff;
            state->memory[state->sp - 2] = (ret &  0xff);
            state->sp = state->sp - 2;
            state->pc = 0x30;
            state->pc--;
            break;
        }
        case 0xf8:
        {
            if (state->cc.s == 1)
            {
                state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
                state->pc--;
                state->sp += 2;
            }
            break;
        }
        case 0xf9:
            state->sp = (state->h << 8) | state->l;
            break;
        case 0xfa:
            if (state->cc.s == 1)
            {
                state->pc = (opcode[2] << 8) | opcode[1];
                state->pc--;
            }
            else
                state->pc += 2;
            break;
        case 0xfb:
            state->int_enable = 1;
            break;
        case 0xfc:
        {
            if (state->cc.s == 1)
            {
                uint16_t ret = state->pc + 2;
                state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                state->memory[state->sp - 2] = (ret & 0xff);
                state->sp = state->sp - 2;
                state->pc = (opcode[2] << 8) | opcode[1];
                state->pc--;
            }
            else
            {
                state->pc += 2;
            }
            break;
        }
        case 0xfd:
            break;
        case 0xfe:
        {
            {
                uint8_t x = state->a - opcode[1];
                state->cc.z = (x == 0);
                state->cc.s = (x & 0x80) != 0;
                state->cc.p = Parity(x & 0xff);
                state->cc.cy = (opcode[1] > state->a);
                state->pc++;
                break;
            }
        }
        break;
        case 0xff:
        {
            uint16_t ret = state->pc + 1;
            state->memory[state->sp - 1] = (ret >> 8) &  0xff;
            state->memory[state->sp - 2] = (ret &  0xff);
            state->sp = state->sp - 2;
            state->pc = 0x38;
            state->pc--;
            break;
        }
        default:
            break;
    }
    state->pc++;

    /* print out processor state */
    printf("\tC=%d,P=%d,S=%d,Z=%d\n", state->cc.cy, state->cc.p,
           state->cc.s, state->cc.z);
    printf("\tA $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x\n",
           state->a, state->b, state->c, state->d,
           state->e, state->h, state->l, state->sp);
    printf("\tNum: %i\n\n", ++counter);
    if (counter == 37410 || counter == 40000 || counter == 42000 || counter == 44000 || counter == 46000) {
        printf("Halleluha");
    }
}

int main()
{
    char line[255];
    FILE * fpointer = fopen("invaders_hex.txt", "r");
    fullText[0] = '\0';

    int count;
    while (fgets(line, 255, fpointer)) {
        int pos = 0;
        char relevantLine[33];
        count = 0;
        //printf("%.39s\n", line + 10);
        for (int i = 9; i <= 59; i++) {
            //printf("%c", line[i]);
            if (line[i] != ' ') {
                relevantLine[count++] = line[i];
            }
        }
        relevantLine[count] = '\0';
        //printf("%s\n", relevantLine);
        strcat(fullText, relevantLine);
    }
    printf("%s\n", fullText);

    State8080 state;
    state.a = 0;
    state.b = 0;
    state.c = 0;
    state.d = 0;
    state.e = 0;
    state.h = 0;
    state.l = 0;
    state.sp = 0;
    state.pc = 0;
    state.memory = (uint8_t *)malloc(65536 * sizeof(uint8_t));
    state.cc.z = 0;
    state.cc.s = 0;
    state.cc.p = 0;
    state.cc.cy = 0;
    state.cc.ac = 0;
    state.cc.pad = 0;
    state.int_enable = 0;

    int pos = 0;
    int curMemPos = 0;
    while (fullText[pos] != '\0') {
        state.memory[curMemPos++] = NextByte(fullText, pos);
        pos += 2;
    }

    while(true) {
        // printf("hi\n");
        Emulate8080p(&state);
    }



    free(state.memory);
    return 0;
}

void Disassemble8080p(uint8_t *codebuffer, int pc) {
    unsigned char *code = &codebuffer[pc];
    printf("\t%04x \n", pc);
    printf("\t");
    switch (*code)
    {
        case 0x00: printf("NOP\n");
        break;
        case 0x01:
            printf("LXI  B,#$%02x%02x\n", code[2], code[1]);
            break;
        case 0x02: printf("STAX B\n");
        break;
        case 0x03: printf("INX  B\n");
        break;
        case 0x04: printf("INR  B\n");
        break;
        case 0x05: printf("DCR  B\n");
        break;
        case 0x06:
            printf("MVI  B,#$%02x\n", code[1]);
            codeNum++;
            break;
        case 0x07: printf("RLC\n");
        break;
        case 0x08: printf("");
        break;
        case 0x09: printf("DAD  B\n");
        break;
        case 0x0a: printf("LDAX B\n");
        break;
        case 0x0b: printf("DCX  B\n");
        break;
        case 0x0c: printf("INR  C\n");
        break;
        case 0x0d: printf("DCR  C\n");
        break;
        case 0x0e:
            printf("MVI  C,#$%02x\n", code[1]);
            codeNum++;
            break;
        case 0x0f: printf("RRC\n");
        break;
        case 0x10: printf("");
        break;
        case 0x11:
            printf("LXI  D,#$%02x%02x\n", code[2], code[1]);
            break;
        case 0x12: printf("STAX D\n");
        break;
        case 0x13: printf("INX  D\n");
        break;
        case 0x14: printf("INR  D\n");
        break;
        case 0x15: printf("DCR  D\n");
        break;
        case 0x16:
            printf("MVI  D,#$%02x\n", code[1]);
            codeNum++;
            break;
        case 0x17: printf("RAL\n");
        break;
        case 0x18: printf("");
        break;
        case 0x19: printf("DAD  D\n");
        break;
        case 0x1a: printf("LDAX D\n");
        break;
        case 0x1b: printf("DCX  D\n");
        break;
        case 0x1c: printf("INR  E\n");
        break;
        case 0x1d: printf("DCR  E\n");
        break;
        case 0x1e:
            printf("MVI  E,#$%02x\n", code[1]);
            codeNum++;
            break;
        case 0x1f: printf("RAR\n");
        break;
        case 0x20: printf("");
        break;
        case 0x21:
            printf("LXI  H,#$%02x%02x\n", code[2], code[1]);
            break;
        case 0x22:
            printf("SHLD $%02x%02x\n", code[2], code[1]);
            break;
        case 0x23: printf("INX  H\n");
        break;
        case 0x24: printf("INR  H\n");
        break;
        case 0x25: printf("DCR  H\n");
        break;
        case 0x26:
            printf("MVI  H, #$%02x\n", code[1]);
            break;
        case 0x27: printf("DAA\n");
        break;
        case 0x28: printf("");
        break;
        case 0x29: printf("DAD  H\n");
        break;
        case 0x2a:
            printf("LHLD #$%02x%02x\n", code[2], code[1]);
            codeNum += 2;
            break;
        case 0x2b: printf("DCX  H\n");
        break;
        case 0x2c: printf("INR  L\n");
        break;
        case 0x2d: printf("DCR  L\n");
        break;
        case 0x2e:
            printf("MVI  L,#$%02x\n", code[1]);
            codeNum++;
            break;
        case 0x2f: printf("CMA\n");
        break;
        case 0x30: printf("");
        break;
        case 0x31:
            printf("LXI  SP,#$%02x%02x\n", code[2], code[1]);
            break;
        case 0x32:
            printf("STA  $%02x%02x\n", code[2], code[1]);
            break;
        case 0x33: printf("INX  SP\n");
        break;
        case 0x34: printf("INR  M\n");
        break;
        case 0x35: printf("DCR  M\n");
        break;
        case 0x36:
            printf("MVI  M,#$%02x\n", code[1]);
            break;
        case 0x37: printf("STC\n");
        break;
        case 0x38: printf("");
        break;
        case 0x39: printf("DAD  SP\n");
        break;
        case 0x3a:
            printf("LDA  $%02x%02x\n", code[2], code[1]);
            break;
        case 0x3b: printf("DCX  SP\n");
        break;
        case 0x3c: printf("INR  A\n");
        break;
        case 0x3d: printf("DCR  A\n");
        break;
        case 0x3e:
            printf("MVI  A,#$%02x\n", code[1]);
            break;
        case 0x3f: printf("CMC\n");
        break;
        case 0x40: printf("MOV  B,B\n");
        break;
        case 0x41: printf("MOV  B,C\n");
        break;
        case 0x42: printf("MOV  B,D\n");
        break;
        case 0x43: printf("MOV  B,E\n");
        break;
        case 0x44: printf("MOV  B,H\n");
        break;
        case 0x45: printf("MOV  B,L\n");
        break;
        case 0x46: printf("MOV  B,M\n");
        break;
        case 0x47: printf("MOV  B,A\n");
        break;
        case 0x48: printf("MOV  C,B\n");
        break;
        case 0x49: printf("MOV  C,C\n");
        break;
        case 0x4a: printf("MOV  C,D\n");
        break;
        case 0x4b: printf("MOV  C,E\n");
        break;
        case 0x4c: printf("MOV  C,H\n");
        break;
        case 0x4d: printf("MOV  C,L\n");
        break;
        case 0x4e: printf("MOV  C,M\n");
        break;
        case 0x4f: printf("MOV  C,A\n");
        break;
        case 0x50: printf("MOV  D,B\n");
        break;
        case 0x51: printf("MOV  D,C\n");
        break;
        case 0x52: printf("MOV  D,D\n");
        break;
        case 0x53: printf("MOV  D,E\n");
        break;
        case 0x54: printf("MOV  D,H\n");
        break;
        case 0x55: printf("MOV  D,L\n");
        break;
        case 0x56: printf("MOV  D,M\n");
        break;
        case 0x57: printf("MOV  D,A\n");
        break;
        case 0x58: printf("MOV  E,B\n");
        break;
        case 0x59: printf("MOV  E,C\n");
        break;
        case 0x5a: printf("MOV  E,D\n");
        break;
        case 0x5b: printf("MOV  E,E\n");
        break;
        case 0x5c: printf("MOV  E,H\n");
        break;
        case 0x5d: printf("MOV  E,L\n");
        break;
        case 0x5e: printf("MOV  E,M\n");
        break;
        case 0x5f: printf("MOV  E,A\n");
        break;
        case 0x60: printf("MOV  H,B\n");
        break;
        case 0x61: printf("MOV  H,C\n");
        break;
        case 0x62: printf("MOV  H,D\n");
        break;
        case 0x63: printf("MOV  H,E\n");
        break;
        case 0x64: printf("MOV  H,H\n");
        break;
        case 0x65: printf("MOV  H,L\n");
        break;
        case 0x66: printf("MOV  H,M\n");
        break;
        case 0x67: printf("MOV  H,A\n");
        break;
        case 0x68: printf("MOV  L,B");
        break;
        case 0x69: printf("MOV  L,C\n");
        break;
        case 0x6a: printf("MOV  L,D\n");
        break;
        case 0x6b: printf("MOV  L,E\n");
        break;
        case 0x6c: printf("MOV  L,H\n");
        break;
        case 0x6d: printf("MOV  L,L\n");
        break;
        case 0x6e: printf("MOV  L,M\n");
        break;
        case 0x6f: printf("MOV  L,A\n");
        break;
        case 0x70: printf("MOV  M,B\n");
        break;
        case 0x71: printf("MOV  M,C\n");
        break;
        case 0x72: printf("MOV  M,D\n");
        break;
        case 0x73: printf("MOV  M,E\n");
        break;
        case 0x74: printf("MOV  M,H\n");
        break;
        case 0x75: printf("MOV  M,L\n");
        break;
        case 0x76: printf("HLT\n");
        break;
        case 0x77: printf("MOV  M,A\n");
        break;
        case 0x78: printf("MOV  A,B\n");
        break;
        case 0x79: printf("MOV  A,C\n");
        break;
        case 0x7a: printf("MOV  A,D\n");
        break;
        case 0x7b: printf("MOV  A,E\n");
        break;
        case 0x7c: printf("MOV  A,H\n");
        break;
        case 0x7d: printf("MOV  A,L\n");
        break;
        case 0x7e: printf("MOV  A,M\n");
        break;
        case 0x7f: printf("MOV  A,A\n");
        break;
        case 0x80: printf("ADD  B\n");
        break;
        case 0x81: printf("ADD  C\n");
        break;
        case 0x82: printf("ADD  D\n");
        break;
        case 0x83: printf("ADD  E\n");
        break;
        case 0x84: printf("ADD  H\n");
        break;
        case 0x85: printf("ADD  L\n");
        break;
        case 0x86: printf("ADD  M\n");
        break;
        case 0x87: printf("ADD  A\n");
        break;
        case 0x88: printf("ADC  B\n");
        break;
        case 0x89: printf("ADC  C\n");
        break;
        case 0x8a: printf("ADC  D\n");
        break;
        case 0x8b: printf("ADC  E\n");
        break;
        case 0x8c: printf("ADC  H\n");
        break;
        case 0x8d: printf("ADC  L\n");
        break;
        case 0x8e: printf("ADC  M\n");
        break;
        case 0x8f: printf("ADC  A\n");
        break;
        case 0x90: printf("SUB  B\n");
        break;
        case 0x91: printf("SUB  C\n");
        break;
        case 0x92: printf("SUB  D\n");
        break;
        case 0x93: printf("SUB  E\n");
        break;
        case 0x94: printf("SUB  H\n");
        break;
        case 0x95: printf("SUB  L\n");
        break;
        case 0x96: printf("SUB  M\n");
        break;
        case 0x97: printf("SUB  A\n");
        break;
        case 0x98: printf("SBB  B\n");
        break;
        case 0x99: printf("SBB  C\n");
        break;
        case 0x9a: printf("SBB  D\n");
        break;
        case 0x9b: printf("SBB  E\n");
        break;
        case 0x9c: printf("SBB  H\n");
        break;
        case 0x9d: printf("SBB  L\n");
        break;
        case 0x9e: printf("SBB  M\n");
        break;
        case 0x9f: printf("SBB  A\n");
        break;
        case 0xa0: printf("ANA  B\n");
        break;
        case 0xa1: printf("ANA  C\n");
        break;
        case 0xa2: printf("ANA  D\n");
        break;
        case 0xa3: printf("ANA  E\n");
        break;
        case 0xa4: printf("ANA  H\n");
        break;
        case 0xa5: printf("ANA  L\n");
        break;
        case 0xa6: printf("ANA  M\n");
        break;
        case 0xa7: printf("ANA  A\n");
        break;
        case 0xa8: printf("XRA  B\n");
        break;
        case 0xa9: printf("XRA  C\n");
        break;
        case 0xaa: printf("XRA  D\n");
        break;
        case 0xab: printf("XRA  E\n");
        break;
        case 0xac: printf("XRA  H\n");
        break;
        case 0xad: printf("XRA  L\n");
        break;
        case 0xae: printf("XRA  M\n");
        break;
        case 0xaf: printf("XRA  A\n");
        break;
        case 0xb0: printf("ORA  B\n");
        break;
        case 0xb1: printf("ORA  C\n");
        break;
        case 0xb2: printf("ORA  D\n");
        break;
        case 0xb3: printf("ORA  E\n");
        break;
        case 0xb4: printf("ORA  H\n");
        break;
        case 0xb5: printf("ORA  L\n");
        break;
        case 0xb6: printf("ORA  M\n");
        break;
        case 0xb7: printf("ORA  A\n");
        break;
        case 0xb8: printf("CMP  B\n");
        break;
        case 0xb9: printf("CMP  C\n");
        break;
        case 0xba: printf("CMP  D\n");
        break;
        case 0xbb: printf("CMP  E\n");
        break;
        case 0xbc: printf("CMP  H\n");
        break;
        case 0xbd: printf("CMP  L\n");
        break;
        case 0xbe: printf("CMP  M\n");
        break;
        case 0xbf: printf("CMP  A\n");
        break;
        case 0xc0: printf("RNZ\n");
        break;
        case 0xc1: printf("POP  B\n");
        break;
        case 0xc2:
            printf("JNZ  $%02x%02x\n", code[2], code[1]);
            break;
        case 0xc3:
            printf("JMP  $%02x%02x\n", code[2], code[1]);
            break;
        case 0xc4:
            printf("CNZ  $%02x%02x\n", code[2], code[1]);
            break;
        case 0xc5: printf("PUSH B\n");
            break;
        case 0xc6:
            printf("ADI  #$%02x\n", code[1]);
            break;
        case 0xc7: printf("RST  0\n");
        break;
        case 0xc8: printf("RZ\n");
        break;
        case 0xc9: printf("RET\n");
        break;
        case 0xca:
            printf("JZ   $%02x%02x\n", code[2], code[1]);
            break;
        case 0xcb: printf("");
        break;
        case 0xcc:
            printf("CZ   $%02x%02x\n", code[2], code[1]);
            break;
        case 0xcd:
            printf("CALL $%02x%02x\n", code[2], code[1]);
            break;
        case 0xce:
            printf("ACI  $%02x\n", code[1]);
            break;
        case 0xcf: printf("RST  1\n");
        break;
        case 0xd0: printf("RNC\n");
        break;
        case 0xd1: printf("POP  D\n");
        break;
        case 0xd2:
            printf("JNC  $%02x%02x\n", code[2], code[1]);
            break;
        case 0xd3:
            printf("OUT  $%02x\n", code[1]);
            break;
        case 0xd4:
            printf("CNC  $%02x\n", code[1]);
            break;
        case 0xd5: printf("PUSH D\n");
        break;
        case 0xd6:
            printf("SUI  $%02x\n", code[1]);
            break;
        case 0xd7: printf("RST 2\n");
        break;
        case 0xd8: printf("RC\n");
        break;
        case 0xd9: printf("");
        break;
        case 0xda:
            printf("JC   $%02x%02x\n", code[2], code[1]);
            break;
        case 0xdb:
            printf("IN   #$%02x\n", code[1]);
            break;
        case 0xdc:
            printf("CC   $%02x%02x\n", code[2], code[1]);
            break;
        case 0xdd: printf("");
        break;
        case 0xde:
            printf("SBI  #$%02x\n", code[1]);
            break;
        case 0xdf: printf("RST  3\n");
        break;
        case 0xe0: printf("RPO\n");
        break;
        case 0xe1: printf("POP  H\n");
        break;
        case 0xe2:
            printf("JPO  $%02x%02x\n", code[2], code[1]);
            break;
        case 0xe3: printf("XTHL\n");
        break;
        case 0xe4:
            printf("CPO  $%02x%02x\n", code[2], code[1]);
            break;
        case 0xe5: printf("PUSH H\n");
        break;
        case 0xe6:
            printf("ANI  #$%02x\n", code[1]);
            break;
        case 0xe7: printf("RST  4\n");
        break;
        case 0xe8: printf("RPE\n");
        break;
        case 0xe9: printf("PCHL\n");
        break;
        case 0xea:
            printf("JPE  $%02x%02x\n", code[2], code[1]);
            break;
        case 0xeb: printf("XCHG\n");
        break;
        case 0xec:
            printf("CPE  $%02x%02x\n", code[2], code[1]);
            break;
        case 0xed: printf("");
        break;
        case 0xee:
            printf("XRI  #$%02x\n", code[1]);
            break;
        case 0xef: printf("RST  5\n");
        break;
        case 0xf0: printf("RP\n");
        break;
        case 0xf1: printf("POP  PSW\n");
        break;
        case 0xf2:
            printf("JP   $%02x%02x\n", code[2], code[1]);
            break;
        case 0xf3: printf("DI\n");
        break;
        case 0xf4:
            printf("CP   $%02x%02x\n", code[2], code[1]);
            break;
        case 0xf5: printf("PUSH PSW\n");
        break;
        case 0xf6:
            printf("ORI  #$%02x\n", code[1]);
            break;
        case 0xf7: printf("RST  6\n");
        break;
        case 0xf8: printf("RM\n");
        break;
        case 0xf9: printf("SPHL\n");
        break;
        case 0xfa:
            printf("JM   $%02x%02x\n", code[2], code[1]);
            break;
        case 0xfb: printf("EI\n");
        break;
        case 0xfc:
            printf("CM   $%02x%02x\n", code[2], code[1]);
            break;
        case 0xfd: printf("");
        break;
        case 0xfe:
            printf("CPI  #$%02x\n", code[1]);
            break;
        case 0xff: printf("RST  7\n");
        default:
            break;
    }
}

