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
- logic 0:      ~600us LOW ~1700us HIGH
- logic 1:      ~600us LOW ~600us HIGH
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
#include "VAGRadioRemote.h"
#include <Arduino.h>

/**
   Constructor
*/
VAGRadioRemote::VAGRadioRemote(uint8_t pin){
	_pin=pin;	
}

/**
   Destructor
*/
VAGRadioRemote::~VAGRadioRemote(){}

void VAGRadioRemote::begin()
{
	digitalWrite(_pin,HIGH);
	pinMode(_pin,OUTPUT);
	digitalWrite(_pin,HIGH);
}


void VAGRadioRemote::send(uint8_t _byte){ //send whole packet
	write_start();
	write(FIRST_BYTE);
	write(SECOND_BYTE);
	write(_byte);
	write(calc_crc(_byte));
}

void VAGRadioRemote::write_start(){ //send initial pulses
//- start bit:    9ms LOW 4.55ms HIGH
//pin is at high already

	digitalWrite(_pin,LOW);
	delay(9);
	digitalWrite(_pin,HIGH);
	delay(4);
	delayMicroseconds(550);
}

void VAGRadioRemote::write(uint8_t _byte){ //send single byte
//- logic 0:      ~600us LOW ~1700us HIGH
//- logic 1:      ~600us LOW ~600us HIGH

        uint8_t i;

        for (i = 0; i < 8; i++)  {
	    digitalWrite(_pin,LOW);
	    delayMicroseconds(600);
	    digitalWrite(_pin,HIGH);
	    if (!!(_byte & (1 << (7 - i))))
	      delayMicroseconds(600);
	    else
	      delayMicroseconds(1700);
  	}
	digitalWrite(_pin,LOW);
	delayMicroseconds(600);
	digitalWrite(_pin,HIGH);

}

uint8_t VAGRadioRemote::calc_crc(uint8_t _byte){ //return calculated checksum
return (0xFF-_byte);
}

void VAGRadioRemote::up(){
	VAGRadioRemote::send(UP);
}

void VAGRadioRemote::down(){
	VAGRadioRemote::send(DOWN);
}

void VAGRadioRemote::left(){
	VAGRadioRemote::send(LEFT);
}

void VAGRadioRemote::right(){
	VAGRadioRemote::send(RIGHT);
}

void VAGRadioRemote::volumeUp(){
        VAGRadioRemote::send(VOLUMEUP);
}

void VAGRadioRemote::volumeDown(){
        VAGRadioRemote::send(VOLUMEDOWN);
}

void VAGRadioRemote::button(uint8_t buttonNumber){
	VAGRadioRemote::send(buttonNumber);
}

void VAGRadioRemote::reg(){
        VAGRadioRemote::send(REG);
}

void VAGRadioRemote::am(){
        VAGRadioRemote::send(AM);
}

void VAGRadioRemote::as(){
        VAGRadioRemote::send(AS);
}

void VAGRadioRemote::fm(){
        VAGRadioRemote::send(FM);
}

void VAGRadioRemote::random(){
        VAGRadioRemote::send(RANDOM);
}
/*
void VAGRadioRemote::down_custom(uint8_t _custom_down){
        VAGRadioRemote::send(0x80+_custom_down);
}
*/

void VAGRadioRemote::tp(){
	VAGRadioRemote::send(TP);
}

void VAGRadioRemote::scan(){
	VAGRadioRemote::send(SCAN);
}

void VAGRadioRemote::mode(){
	VAGRadioRemote::send(MODE);
}


