# VW Radio Remote Control library for arduino

Moved from this repo (https://github.com/tomaskovacik/arduino/tree/master/VW_radio_remote_control)  to separate repository when it became library.

later I realized that this protocol is inverted standard NEC IR remote protocol with 16bit address

https://www.sbprojects.net/knowledge/ir/nec.php

this library should also works, but I did not test it, do not forget you need invert it (standard NEC start with 9ms HIGH pulse followed with 4.5ms low pulse, while VAG start with 9ms low folowed by 4.5ms high etc ...)

also NEC protocol use LSB first while my implementation use MSF first, maybe I will change it in future


## Instalation of library

Refer to oficial arduino guide here: https://www.arduino.cc/en/Guide/Libraries
Download content of repository and extract it into arduinoSketchFolder/library directory.

## How to use it

Example directory has 5 examples, take a look at them.

sketch must start with 

```
#include "VAGRadioRemote.h"
```

define arduino pin which will be connected to "REM" pin of radio and/or pin for input from remote control to read pressed button
input pin must have hardware input capability

```
#define REMOTEOUT 2
#define REMOTEIN 3
```

then initialize library with this line:

```
VAGRadioRemote remote(REMOTEOUT); //only output
VAGRadioRemote remote(REMOTEOUT,-1); //only output
VAGRadioRemote remote(REMOTEOUT,REMOTEIN);  //for both input and output
VAGRadioRemote remote(-1,REMOTEIN);  //only input
```

in setup() function include this line for set pin as output:

```
remote.begin();
```

and in loop() this functions can be used:

```
void send(uint8_t _byte); //this send command "_byte" to radio
void up(); //send predefined code for UP button
void down(); //send predefined code for DOWN button
void left(); //send predefined code for LEFT button
void right(); //send predefined code for RIGHT button
void volumeUp(); //send predefined code for VOLUME UP  button
void volumeDown(); //send predefined code for VOLUME DOWN button
void button(uint8_t buttonNumber); //send predefined code for button defined as "buttonNumber" = 1,2,3,4,5,6.... (memory,cd buttons on front panel of radio)
void reg(); //send predefined code for REG button - switch between regional and DX radio erceiving mode
void am(); //send predefined code for AM button
void as(); //send predefined code for AS button
void fm(); //send predefined code for FM button
void random(); //send predefined code for RD button (random)
//void down_custom(uint8_t _custom_down); //send predefined code for volume button with increment of "_custom_down, probably custom_up exists ... 
void tp(); //send predefined code for TP button
void scan(); //send predefined code for SCAN button
void mode(); //send predefined code for MODE button
uint8_t gotNewCode(); //return true if we have new code read from input
uint8_t newCode(); //return new code
void clearGotNewCode(); //clear flag that we have new code red
String decodeRemote(uint8_t code); //get meaning of code in human readable format
String decodeRemote(); //get meaning of newCode in human readable format
```

## Examples

Library includes 5 examples, code_finder and emulator are using ebay LCD shield(2x16) with 5 buttons on A0. Examples use  pin 2 of arduino as output to radio (remote signal `REM`).

- code_finder is sketchup for finding all codes understended by my audi concert/chorus 1 radios
- emulator is sketchup emulating audi MFSW
- Radio_remote_over_serial - serial to remote control protocol example
- Radio_remote_read_and_write - read remote control and send it to another pin of arduino 
- Radio_remote_read_to_serial - read remote control and write to serial what button was pressed in human readable form

More info: http://kovo-blog.blogspot.sk/2013/10/remote-control-audi-radio.html

## Protocol

On vehicles where the radio does not use CAN bus, the MFSW (Multifunction
Steering Wheel) controller talks to the radio using only 1 wire, which is
called `REM` (remote).

`REM` line is 5V logic, idle state is HIGH (5V)

- start bit:	9ms LOW 4.55ms HIGH
- logic 1:	~600us LOW ~1700us HIGH
- logic 0: 	~600us LOW ~600us HIGH
- stop bit:	~600us LOW

The MFSW controller always sends a packet of 4 bytes to the radio.  It
consists of 2 unknown header bytes, followed by a code byte, and finally
a checksum byte:

    0x41 0xE8 <code> <checksum>

The checksum is `0xFF - code`.  Example:

    0x41 0xE8 0xD0 0x2F

Code byte: `0xD0`, Checksum: `0xFF - 0xD0 = 0x2F`.

## MFSW

Using a logic analyzer, these codes were captured from an MFSW without
telephone option:

| Steering Wheel Button | Code      | Complete Packet       |
| --------------------- | --------- | --------------------- |
| UP                    | `0xD0`    | `0x41 0xE8 0xD0 0x2F` |
| DOWN                  | `0x50`    | `0x41 0xE8 0x50 0xAF` |
| LEFT                  | `0x40`    | `0x41 0xE8 0x40 0xBF` |
| RIGHT                 | `0xC0`    | `0x41 0xE8 0xC0 0x3F` |
| VOL+                  | `0x80`    | `0x41 0xE8 0x80 0x7F` |
| VOL-                  | `0x00`    | `0x41 0xE8 0x00 0xFF` |

## Audi Concert I Radio

Using the `code_finder` program above, an Audi Concert I radio responded to
these codes:

| Code      | Function                                                     |
| --------- | ------------------------------------------------------------ |
| `0x00`    | Volume down                                                  |
| `0x01`    | mem/cd1                                                      |
| `0x03`    | mem/cd3                                                      |
| `0x05`    | mem/cd5                                                      |
| `0x07`    | search up                                                    |
| `0x09`    | reg on/off                                                   |
| `0x0B`    | tp                                                           |
| `0x11`    | AM                                                           |
| `0x15`    | AS-STORE                                                     |
| `0x17`    | FM                                                           |
| `0x1D`    | search down                                                  |
| `0x20`    | AM->AM+AS->FM1->FM2->FM1+AS->FM2->AS/in CD mode “RD”         |
| `0x22`    | AM->AM+AS->FM1->FM2->FM1+AS->FM2->AS/in CD mode “RD”         |
| `0x24`    | AM->AM+AS->FM1->FM2->FM1+AS->FM2->AS/in CD mode “RD”         |
| `0x26`    | AM->AM+AS->FM1->FM2->FM1+AS->FM2->AS/in CD mode “RD”         |
| `0x27`    | MEM 1->2->3->4->-5->6                                        |
| `0x29`    | TP                                                           |
| `0x2B`    | search up                                                    |
| `0x3C`    | seek up                                                      |
| `0x40`    | LEFT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1                         |
| `0x42`    | LEFT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1                         |
| `0x44`    | LEFT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1                         |
| `0x46`    | LEFT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1                         |
| `0x50`    | Seek down/FR                                                 |
| `0x52`    | Seek down/FR                                                 |
| `0x54`    | Seek down/FR                                                 |
| `0x56`    | Seek down/FR                                                 |
| `0x60`    | Seek down/FR                                                 |
| `0x62`    | seek down                                                    |
| `0x64`    | seek down                                                    |
| `0x80`    | Volume up                                                    |
| `0x81`    | Volume up                                                    |
| `0x82`    | Volume up                                                    |
| `0x83`    | Volume up                                                    |
| `0x84`    | Volume up                                                    |
| `0x85`    | Volume up                                                    |
| `0x86`    | Volume up                                                    |
| `0x89`    | -2 Volume down bas/treble down/fade rear/bal left            |
| `0x8B`    | -4 Volume down bas/treble down/fade rear/bal left            |
| `0x8D`    | -6 Volume down bas/treble down/fade rear/bal left            |
| `0x8F`    | -8 Volume down/fade rear/bal left                            |
| `0x97`    | TP                                                           |
| `0x9B`    | SCAN                                                         |
| `0xA0`    | MODE                                                         |
| `0xA2`    | MODE                                                         |
| `0xA4`    | MODE                                                         |
| `0xA6`    | MODE                                                         |
| `0xC0`    | RIGHT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1                        |
| `0xC2`    | RIGHT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1                        |
| `0xC4`    | RIGHT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1                        |
| `0xC6`    | RIGHT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1                        |
| `0xD0`    | Seek up/FF                                                   |
| `0xD2`    | Seek up/FF                                                   |
| `0xD4`    | Seek up/FF                                                   |
| `0xD6`    | Seek up/FF                                                   |
| `0xE0`    | seek up                                                      |
| `0xE2`    | seek up                                                      |
| `0xE4`    | seek up                                                      |
| `0xE6`    | seek up                                                      |
