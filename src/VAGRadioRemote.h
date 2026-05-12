/*
  (C) Tomas Kovacik
  https://github.com/tomaskovacik/
  GNU GPL3

arduino library for V.A.G. radio remote control protocol, for reading and writing.

this library use hardcoded timer2! it is not dependant on any of it features, any timer can be used
input pin must have hardware interrupt (INTx), , output pin must not be input only of corse

====================== reading example:
#include <VAGRadioRemote.h>

#define REMOTE_PIN 2

VAGRadioRemote remote(-1, REMOTE_PIN);

void setup() {
  remote.begin();
  Serial.begin(115200);
}

void loop() {
 if (remote.gotNewCode()) {
    Serial.println(remote.decodeRemote(remote.newCode()));
    remote.clearGotNewCode();
  }
}
=====================

===================== writing example on diferent device of corse
#include <VAGRadioRemote.h>

#define REMOTE_PIN 2 // this has to have hardware interrupt!

long last_update = 0;

VAGRadioRemote remote(REMOTE_PIN,-1);

void setup() {
  remote.begin();
  Serial.begin(115200);
}

void loop() {
  if (Serial.available() > 0)
  {
    // read the incoming byte:
    char c = Serial.read();
    switch (c)
    {
      case 'm': //mode
        remote.mode();
.
.
.
.

===================


## Protocol

On vehicles where the radio does not use CAN bus, the MFSW (Multifunction
Steering Wheel) controller talks to the radio using only 1 wire(sort of , second one is ground)
, which is called `REM` (remote).

`REM` line is 5V logic, idle state is HIGH (5V)

- start bit:    ~9000us LOW ~4500us HIGH
- logic 1:       ~560us LOW ~1690us HIGH (total 2.25ms, NEC inverted)
- logic 0:       ~560us LOW ~560us HIGH  (total 1.125ms, NEC inverted)
- stop bit:      ~560us LOW

The MFSW controller always sends a packet of 4 bytes to the radio.  It
consists of 2 unknown header bytes, followed by a code byte, and finally
a checksum byte:

    0x82 0x17 <code> <checksum>

The checksum is `0xFF - code`.  Example:

    0x82 0x17 0x0B 0xF4

Code byte: `0x0B`, Checksum: `0xFF - 0x0B = 0xF4`.

## MFSW

Using a logic analyzer, these codes were captured from an MFSW without
telephone option (bytes shown in NEC LSB-first representation):

| Steering Wheel Button | Code      | Complete Packet       |
| --------------------- | --------- | --------------------- |
| UP                    | `0x0B`    | `0x82 0x17 0x0B 0xF4` |
| DOWN                  | `0x0A`    | `0x82 0x17 0x0A 0xF5` |
| LEFT                  | `0x02`    | `0x82 0x17 0x02 0xFD` |
| RIGHT                 | `0x03`    | `0x82 0x17 0x03 0xFC` |
| VOL+                  | `0x01`    | `0x82 0x17 0x01 0xFE` |
| VOL-                  | `0x00`    | `0x82 0x17 0x00 0xFF` |


*/
#ifndef VAGRadioRemote_h
#define VAGRadioRemote_h
#include <inttypes.h>
#include <Arduino.h>

#define VOLUMEDOWN 0x00 //	Volume down
#define BTN1 0x80    //	mem/cd1
#define BTN2 0x40    //	I just made this up: mem/cd2
#define BTN3 0xC0 //	mem/cd3
#define BTN4 0x20 //	madeup: mem/cd4
#define BTN5 0xA0 //	mm/cd5
#define BTN6 0x60 //	madeup: mem/cd6
//#define UP 0x0B //	search up
#define VW_TOGGLE_MODE 0x10 //Toggle Mode, user submitted
#define REG 0x90 //	reg on/off
//#define TP 0xE9 //	tp
#define AM 0x88 //	AM
#define AS 0xA8 //	AS-STORE
#define FM 0xE8 //	FM
//#define 0x20  // 	search down
#define RANDOM 0x04 //	AM->AM+AS->FM1->FM2->FM1+AS->FM2->AS/in CD mode "RD"
//#define 0x44 	AM->AM+AS->FM1->FM2->FM1+AS->FM2->AS/in CD mode "RD"
//#define 0x24 	AM->AM+AS->FM1->FM2->FM1+AS->FM2->AS/in CD mode "RD"
//#define 0x64 	AM->AM+AS->FM1->FM2->FM1+AS->FM2->AS/in CD mode "RD"
//#define TP 0x94 // 	TP
//#define SEARCH_UP 0xD4// 	search up
//#define SEEK_UP 0x3C// 	seek up
#define LEFT 0x02// 	LEFT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
//#define 0x42 	LEFT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
//#define 0x22 	LEFT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
#define VW_LEFT 0xA2 // Left FM1/FM2/AM 6-5-4-3-2-1 | CD/SD - Folder Down, user submitted
//#define 0x62 	LEFT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
#define VW_RIGHT 0xB2 // Left FM1/FM2/AM 1-2-3-4-5-5 | CD/SD - Folder Up
#define DOWN 0x0A //	Seek down/FR
//#define 0x4A 	Seek down/FR
//#define 0x2A 	Seek down/FR
//#define 0x6A 	Seek down/FR
#define CDDOWN 0x06 // 	Seek down // down in CD mode
//#define 0x46 	seek down
//#define 0x26 	seek down
#define VOLUMEUP 0x01// 	Volume up
//#define 0x81 	Volume up
//#define 0x41 	Volume up
//#define 0xC1 	Volume up
//#define 0x21 	Volume up
//#define 0xA1 	Volume up
//#define 0x61 	Volume up
#define VW_VOICE_DIAL 0x11 // Voice Dail
#define DOWN_2 0x91// 	-2 Volume down bas/treble down/fade rear/bal left
#define DOWN_4 0xD1// 	-4 Volume down bas/treble down/fade rear/bal left
#define DOWN_6 0xB1// 	-6 Volume down bas/treble down/fade rear/bal left
#define DOWN_8 0xF1// 	-8 Volume down/fade rear/bal left
#define TP 0xE9// 	TP
#define SCAN 0xD9// 	SCAN
#define MODE 0x05 //	MODE
//#define 0x45 	MODE
//#define 0x25 	MODE
//#define 0x65 	MODE
#define RIGHT 0x03 //	RIGHT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
//#define 0x43 	RIGHT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
//#define 0x23 	RIGHT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
//#define 0x63 	RIGHT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
#define UP 0x0B// 	Seek up/FF
//#define 0x4B 	Seek up/FF
//#define 0x2B 	Seek up/FF
//#define 0x6B 	Seek up/FF
#define CDUP 0x07 //	seek up //up in CD mode
//#define 0x47 	seek up
//#define 0x27 	seek up
//#define 0x67 	seek up

#define FIRST_BYTE 0x82
#define SECOND_BYTE 0x17
#define CRC(x) (0xFF-x)
#define SENDPOINTERTOP 67

class VAGRadioRemote
{
public:

VAGRadioRemote(int8_t outpin, int8_t inpin = -1);
~VAGRadioRemote();
void begin(); //setup port

void send(uint8_t _byte); //send whole packet
void up();
void cdUp();
void down();
void cdDown();
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
uint8_t gotNewCode();
uint8_t newCode();
void clearGotNewCode();
static uint8_t bitLenght(uint8_t _byte,uint8_t _bit);

String decodeRemote(uint8_t code);
String decodeRemote();
static void remoteInGoingHigh();
static void remoteInGoingLow();
private:
void setTimer(void);
};

#endif
