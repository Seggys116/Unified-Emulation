
# Project: Unified-Emulation

[![MIT License](https://img.shields.io/badge/License-MIT-green.svg)](https://github.com/Unified-Projects/Unified-Emulation/blob/main/LICENCE) ![GitHub release (with filter)](https://img.shields.io/github/v/release/Unified-Projects/Unified-Emulation)

The aim of this projects is to build an understanding of how hardwares function and interact with each other. Meanwhile, being able to build a enjoyable applicatiom im the process.

## Implementations

Currently the only emulation available is the `NES` emulator.

## TODO:

Shrink the engine so that it only has the code needed for the emulation and not for whole 3D games.

## Installation and usage

This will compile the whole feature-set:

```bash
  mkdir Build
  cmake ..
  make
```

You can download the compiled version from the **release page**.

**Fun Fact:** You can pass `--debug` in to the executable to get a nice debug screen! (However you will need to add `rsc/Fonts/Font.ttf` of your desired debug font! Retro.ttf works great)

### To play roms:

You need to have a `rsc/ROMS/NES` folder with the roms in in the same directory as the executable.

To iterate over the open ROM press N

***We do not encourage pirating of the ROMS***

## NES:

Key layouts:

```
Up = Key_UP;
Down = Key_DOWN;
Left = Key_LEFT;
Right = Key_RIGHT;
Select = Key_RIGHT_SHIFT;
Start = Key_ENTER;
A = Key_A;
B = Key_S;
X = Key_Z;
Y = Key_X;
```

Extra Emulator Keys:

- N : Next ROM
- P : Change Pallete (DEBUG ONLY)
- R : Reset
- PG_UP : Scale Up (DEBUG ONLY)
- PG_DOWN : Scale Down (DEBUG ONLY)

![SMB With Debug On](https://github.com/Unified-Projects/Unified-Emulation/blob/main/images/SMB.png)
![DT Without Debug](https://github.com/Unified-Projects/Unified-Emulation/blob/main/images/DT.png)

## Authors

- [@Seggys116](https://www.github.com/Seggys116)
