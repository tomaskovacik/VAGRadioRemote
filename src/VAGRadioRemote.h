/*
  (C) Tomas Kovacik
  https://github.com/tomaskovacik/
  GNU GPL3

arduino library fo V.A.G. radio remote control protocol.  This information is for vehicles where the radio
does not use CAN bus.

## Protocol

On vehicles where the radio does not use CAN bus, the MFSW (Multifunction
Steering Wheel) controller talks to the radio using only 1 wire, which is
called `REM` (remote).

`REM` line is 5V logic, idle state is HIGH (5V)

- start bit:    9ms LOW 4.55ms HIGH
- logic 1:      ~600us LOW ~1700us HIGH
- logic 0:      ~600us LOW ~600us HIGH
- stop bit:     ~600us LOW

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


*/
#ifndef VAGFISReader_h
#define VAGFISReader_h
#include <inttypes.h>
#include <Arduino.h>

#define VOLUMEDOWN 0x00 //	Volume down
#define BTN1 0x01    //	mem/cd1
#define BTN2 0x02    //	I just made this up: mem/cd2
#define BTN3 0x03 //	mem/cd3
#define BTN3 0x04 //	madeup: mem/cd4
#define BTN5 0x05 //	mm/cd5
#define BTN5 0x06 //	madeup: mem/cd6
#define UP 0x07 //	search up
#define REG 0x09 //	reg on/off
//#define TP 0x0B //	tp
#define AM 0x11 //	AM
#define AS 0x15 //	AS-STORE
#define FM 0x17 //	FM
//#define 0x1D  // 	search down
#define RANDOM 0x20 //	AM->AM+AS->FM1->FM2->FM1+AS->FM2->AS/in CD mode “RD”
//#define 0x22 	AM->AM+AS->FM1->FM2->FM1+AS->FM2->AS/in CD mode “RD”
//#define 0x24 	AM->AM+AS->FM1->FM2->FM1+AS->FM2->AS/in CD mode “RD”
//#define 0x26 	AM->AM+AS->FM1->FM2->FM1+AS->FM2->AS/in CD mode “RD”
//#define TP 0x29 // 	TP
//#define SEARCH_UP 0x2B// 	search up
//#define SEEK_UP 0x3C// 	seek up
#define LEFT 0x40// 	LEFT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
//#define 0x42 	LEFT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
//#define 0x44 	LEFT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
//#define 0x46 	LEFT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
#define DOWN 0x50 //	Seek down/FR
//#define 0x52 	Seek down/FR
//#define 0x54 	Seek down/FR
//#define 0x56 	Seek down/FR
//#define 0x60 	Seek down/FR
//#define 0x62 	seek down
//#define 0x64 	seek down
#define VOLUMEUP 0x80// 	Volume up
//#define 0x81 	Volume up
//#define 0x82 	Volume up
//#define 0x83 	Volume up
//#define 0x84 	Volume up
//#define 0x85 	Volume up
//#define 0x86 	Volume up
#define DOWN_2 0x89// 	-2 Volume down bas/treble down/fade rear/bal left
#define DOWN_4 0x8B// 	-4 Volume down bas/treble down/fade rear/bal left
#define DOWN_6 0x8D// 	-6 Volume down bas/treble down/fade rear/bal left
#define DOWN_8 0x8F// 	-8 Volume down/fade rear/bal left
#define TP 0x97// 	TP
#define SCAN 0x9B// 	SCAN
#define MODE 0xA0 //	MODE
//#define 0xA2 	MODE
//#define 0xA4 	MODE
//#define 0xA6 	MODE
#define RIGHT 0xC0 //	RIGHT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
//#define 0xC2 	RIGHT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
//#define 0xC4 	RIGHT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
//#define 0xC6 	RIGHT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
#define UP 0xD0// 	Seek up/FF
//#define 0xD2 	Seek up/FF
//#define 0xD4 	Seek up/FF
//#define 0xD6 	Seek up/FF
//#define 0xE0 	seek up
//#define 0xE2 	seek up
//#define 0xE4 	seek up
//#define 0xE6 	seek up

#define FIRST_BYTE 0x41
#define SECOND_BYTE 0xE8
#define CRC(x) (0xFF-x)
 
class VAGRadioRemote
{
public:

VAGRadioRemote(uint8_t pin);
~VAGRadioRemote();
void begin(); //setup port
void send(uint8_t _byte); //send whole packet
void up();
void down();
void left();
void right();
void volumeUp();
void volumeDown();
void button(uint8_t buttonNumber);
void reg();
void am();
void as();
void fm();
void random();
//void down_custom(uint8_t _custom_down);
void tp();
void scan();
void mode();

private:
uint8_t _pin;
void writeStart(); //send initial pulses
void writeStop(); //send initial pulses
void write(uint8_t _byte); //send single byte
uint8_t calc_crc(uint8_t byte); //return calculated checksum

};

#endif
