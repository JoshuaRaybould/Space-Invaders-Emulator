# Space-Invaders-Emulator

## Description
This is C code for an emulator for the Space Invaders arcade game.

For reference, an emulator is hardware or software that enables one computer system to behave like another. In this case, it will specifically allow any computer to act as a retro arcade machine that ran the game Space Invaders.

### Purpose
The main goal of this project is to develop my skills in C while gaining a deeper understanding of how the CPU and other hardware interact with software. Ultimately, I aim to create a fully functional, well-optimized emulator that provides an authentic playing experience.

The choice to emulate Space Invaders was due to it being a good beginner project with many useful resources available online. Beyond that, it is an iconic game that I have never experienced in its original form, making it an interesting project to explore and understand the technology of the time.

## Current State
- **`emulator.c`**: A working emulator of the Intel 8080 processor. It successfully implements all required instructions for Space Invaders and attempts to implement all others (though these still require testing). This code makes use of much of the code from `decompiler.c`.
- **`decompiler.c`**: Created first to build my understanding of the Space Invaders code and ensure I understood how to handle it in the actual emulator. Unlikely to be further updated.
- **`invaders_hex.txt`**: Contains the hexdump of the Space Invaders code, which is read by `emulator.c`.

## Learning Goals
### My aims:
- Develop my skills in C (this is my first large project using the language).
- Gain a deeper understanding of CPU architecture and how hardware interacts with software.
- Overcome challenges in debugging and optimizing code.

### Roadblocks:
- Initially, I was not well-versed in C, which became painfully aparant after starting so I dedicated a large amount of time to learning C in greater detail before continuing.
- The next major step was getting the decompiler to work correctly, requiring some familiarity with assembly code, which proved manageable.
- Implementing the 8080 CPU emulator in `emulator.c` required extensive research into assembly and the processor itself. I frequently consulted the Intel 8080 manual and engaged in extensive debugging by comparing it to a known working emulator. This experience also helped me become more comfortable debugging through the terminal.

## Current To-Do List
- Learn about the rest of the arcade machine's hardware in detail.
- Implement graphics.
- Implement sound.
- Implement I/O (buttons and ports).


