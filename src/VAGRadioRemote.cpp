/*
  (C) Tomas Kovacik
  https://github.com/tomaskovacik/
  GNU GPL3

arduino library fo V.A.G. radio remote control protocol, for reading and writing.

this library use hardcoded timer2! it is not dependant on any of it features, any timer can be used
input pit has to have hardware interrupt (INTx)

====================== reading example:
#include <VAGRadioRemote.h>

#define REMOTE_PIN 2

VAGRadioRemote remote(NULL, REMOTE_PIN);

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

VAGRadioRemote remote(REMOTE_PIN,NULL);


int h = 0x00;

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


This information is for vehicles where the radio does not use CAN bus.

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

#include "VAGRadioRemote.h"
#include <Arduino.h>

//#define USE_TIMER0
//#define USE_TIMER1
//#define USE_TIMER2
//#define USE_TIMER3
//#define USE_TIMER4
#define USE_TIMER5

volatile static uint8_t data[2];
volatile static uint8_t sendPtr=0;// 67 is stop => 9000ms,4500ms,4x8x2 + stopbit
volatile static uint8_t counter=0;
volatile static uint8_t _outpin;
volatile static uint8_t _inpin;

static volatile uint16_t captime = 0;
static volatile uint8_t captureEnabled = 0;
static volatile uint8_t capbit = 0;
static volatile uint8_t capbyte[4];
static volatile uint8_t capptr = 0;
static volatile uint8_t _gotNewCode = 0;
static volatile uint8_t _newCode = 0;

/**
   Constructor
*/

VAGRadioRemote::VAGRadioRemote(uint8_t outpin,uint8_t inpin){
if (outpin != NULL){
		#define _OUTPUT
        	_outpin = outpin;
}

if (inpin != NULL){
		#define _INPUT
		_inpin = inpin;
}

}

/**
   Destructor
*/
VAGRadioRemote::~VAGRadioRemote(){}

#ifdef USE_TIMER0
void VAGRadioRemote::setTimer(void){
  // @ 0.5us ticks
  TCCR0A = 0x00; // Normal port operation, OC0 disconnected
  TCCR0B = 0x00; // Normal port operation, OC0 disconnected
  TCCR0A |= _BV(WGM01); // CTC mode
  TCCR0B |= _BV(CS01);// prescaler = 8 -> 1 timer clock tick is 0.5us long @ 16Mhz
  OCR0A = 50;//run compare rutine every 5us, 0.5x10
  TCNT0 = 0;
  TIMSK0 |= _BV(OCIE0A); // enable output compare interrupt A on timer0
}
#define __TIMERX_COMPA_vect TIMER0_COMPA_vect
#endif

#ifdef USE_TIMER1
void VAGRadioRemote::setTimer(void){
  // @ 0.5us ticks
  TCCR1A = 0x00; // Normal port operation, OC0 disconnected
  TCCR1B = 0x00; // Normal port operation, OC0 disconnected
  TCCR1B |= _BV(WGM12); // CTC mode
  TCCR1B |= _BV(CS11);// prescaler = 8 -> 1 timer clock tick is 0.5us long @ 16Mhz
  OCR1A = 50;//run compare rutine every 5us, 0.5x10
  TCNT1 = 0;
  TIMSK1 |= _BV(OCIE1A); // enable output compare interrupt A on timer0
}
#define __TIMERX_COMPA_vect TIMER1_COMPA_vect
#endif

#ifdef USE_TIMER2
void VAGRadioRemote::setTimer(void){
  // @ 0.5us ticks
  TCCR2A = 0x00; // Normal port operation, OC0 disconnected
  TCCR2B = 0x00; // Normal port operation, OC0 disconnected
  TCCR2A |= _BV(WGM21); // CTC mode
  TCCR2B |= _BV(CS21);// prescaler = 8 -> 1 timer clock tick is 0.5us long @ 16Mhz
  OCR2A = 50;//run compare rutine every 5us, 0.5x10
  TCNT2 = 0;
  TIMSK2 |= _BV(OCIE2A); // enable output compare interrupt A on timer0
}
#define __TIMERX_COMPA_vect TIMER2_COMPA_vect
#endif

#ifdef USE_TIMER3
void VAGRadioRemote::setTimer(void){
  // @ 0.5us ticks
  TCCR3A = 0x00; // Normal port operation, OC0 disconnected
  TCCR3B = 0x00; // Normal port operation, OC0 disconnected
  TCCR3B |= _BV(WGM32); // CTC mode
  TCCR3B |= _BV(CS31);// prescaler = 8 -> 1 timer clock tick is 0.5us long @ 16Mhz
  OCR3A = 50;//run compare rutine every 5us, 0.5x10
  TCNT3 = 0;
  TIMSK3 |= _BV(OCIE3A); // enable output compare interrupt A on timer0
}
#define __TIMERX_COMPA_vect TIMER3_COMPA_vect
#endif

#ifdef USE_TIMER4
void VAGRadioRemote::setTimer(void){
  // @ 0.5us ticks
  TCCR4A = 0x00; // Normal port operation, OC0 disconnected
  TCCR4B = 0x00; // Normal port operation, OC0 disconnected
  TCCR4B |= _BV(WGM42); // CTC mode
  TCCR4B |= _BV(CS41);// prescaler = 8 -> 1 timer clock tick is 0.5us long @ 16Mhz
  OCR4A = 50;//run compare rutine every 5us, 0.5x10
  TCNT4 = 0;
  TIMSK4 |= _BV(OCIE4A); // enable output compare interrupt A on timer0
}
#define __TIMERX_COMPA_vect TIMER4_COMPA_vect
#endif

#ifdef USE_TIMER5
void VAGRadioRemote::setTimer(void){
  // @ 0.5us ticks
  TCCR5A = 0x00; // Normal port operation, OC0 disconnected
  TCCR5B = 0x00; // Normal port operation, OC0 disconnected
  TCCR5B |= _BV(WGM52); // CTC mode
  TCCR5B |= _BV(CS51);// prescaler = 8 -> 1 timer clock tick is 0.5us long @ 16Mhz
  OCR5A = 50;//run compare rutine every 5us, 0.5x10
  TCNT5 = 0;
  TIMSK5 |= _BV(OCIE5A); // enable output compare interrupt A on timer0
}
#define __TIMERX_COMPA_vect TIMER5_COMPA_vect
#endif

void VAGRadioRemote::begin()
{

#ifdef _OUTPUT
	pinMode(_outpin,OUTPUT);
	digitalWrite(_outpin,HIGH);
#endif

#ifdef _INPUT
	pinMode(_inpin, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(_inpin), &VAGRadioRemote::remoteInGoingLow, FALLING);
#endif
	setTimer();
}

ISR(__TIMERX_COMPA_vect)
{
#ifdef _INPUT

  if (captureEnabled) {
    captime++;
  }

  if (captime == 600 ) { //300x20us =>  6ms high pulse,
    captime = capptr = captureEnabled = 0;
    attachInterrupt(digitalPinToInterrupt(_inpin), &VAGRadioRemote::remoteInGoingLow, FALLING);
  }
#endif

#ifdef _OUTPUT
if (sendPtr>0 && counter == 0)
{
	switch (sendPtr)
	{
		case 67: //start bit, 9000us low
			digitalWrite(_outpin,LOW); //made pin LOW
			counter=90;//count form 90 to 0 for 9000us
		break;
		case 66: //start bit, 4500us high
			digitalWrite(_outpin,HIGH);
			counter=45;
		break;
		//regular start of bit, 600us low
		case 65:
		case 63:
		case 61:
		case 59:
		case 57:
		case 55:
		case 53:
		case 51:
		case 49:
		case 47:
		case 45:
		case 43:
		case 41:
		case 39:
		case 37:
		case 35:
        	case 33:
	        case 31:
        	case 29:
	        case 27:
        	case 25:
	        case 23:
        	case 21:
	        case 19:
        	case 17:
	        case 15:
		case 13:
		case 11:
		case 9:
		case 7:
		case 5:
		case 3:
		case 1:
			digitalWrite(_outpin,LOW); //made pin LOW
			counter=6; //each bit start with 600us low
		break;
		//0x41 and 0xE8 data
		case 64://zeroes
		case 60:
		case 58:
		case 56:
		case 54:
		case 52:
		case 42:
		case 38:
		case 36:
		case 34:
			digitalWrite(_outpin,HIGH);
			counter=6;
		break;
		case 62://ones
		case 50:
		case 48:
		case 46:
		case 44:
		case 40:
			digitalWrite(_outpin,HIGH);
			counter=17;
		break;
		case 32:
			counter=VAGRadioRemote::bitLenght(data[0],7);
		break;
		case 30:
			counter=VAGRadioRemote::bitLenght(data[0],6);
		break;
        	case 28:
	                counter=VAGRadioRemote::bitLenght(data[0],5);
        	break;
		case 26:
                	counter=VAGRadioRemote::bitLenght(data[0],4);
	        break;
        	case 24:
                	counter=VAGRadioRemote::bitLenght(data[0],3);
	        break;
        	case 22:
                	counter=VAGRadioRemote::bitLenght(data[0],2);
	        break;
        	case 20:
                	counter=VAGRadioRemote::bitLenght(data[0],1);
	        break;
        	case 18:
                	counter=VAGRadioRemote::bitLenght(data[0],0);
	        break;
        	case 16:
                	counter=VAGRadioRemote::bitLenght(data[1],7);
	        break;
        	case 14:
                	counter=VAGRadioRemote::bitLenght(data[1],6);
	        break;
        	case 12:
                	counter=VAGRadioRemote::bitLenght(data[1],5);
	        break;
        	case 10:
                	counter=VAGRadioRemote::bitLenght(data[1],4);
	        break;
        	case 8:
                	counter=VAGRadioRemote::bitLenght(data[1],3);
	        break;
        	case 6:
                	counter=VAGRadioRemote::bitLenght(data[1],2);
	        break;
        	case 4:
                	counter=VAGRadioRemote::bitLenght(data[1],1);
	        break;
        	case 2:
                	counter=VAGRadioRemote::bitLenght(data[1],0);
	        break;
	}
		
}
else 
{
if (counter>0)
	counter--;
if (counter == 0)
	sendPtr--;
}
if (sendPtr == 0 && counter == 0)
	digitalWrite(_outpin,HIGH);	
#endif
}

#ifdef _OUTPUT
uint8_t VAGRadioRemote::bitLenght(uint8_t _byte,uint8_t _bit){
	digitalWrite(_outpin,HIGH);
	if (!!(_byte & (1 << _bit)))
		return 17;
	else
		return 6;
}

void VAGRadioRemote::send(uint8_t _byte){ //send whole packet
	data[0]=_byte;
	data[1]=CRC(_byte);
	sendPtr=SENDPOINTERTOP;
}
#endif

#ifdef _INPUT
void VAGRadioRemote::remoteInGoingHigh() {
  captime = 0; //reset timer
  if (capptr < 4) {
    captureEnabled = 1; //we capturing only when capture pointer is less then 4th byte.
  }
  attachInterrupt(digitalPinToInterrupt(_inpin), &VAGRadioRemote::remoteInGoingLow, FALLING);
}

void VAGRadioRemote::remoteInGoingLow() {
  if (captureEnabled) {
    //we have ticked in some data, lets calculate what we capture
    captureEnabled = 0; //disable future counting in timer2
    if (captime > 35) {//start bit 4.55ms 200x20

      capbit = capptr = 0; //start of packet, lets reset it all to begining
      capbyte[0] = capbyte[1] = capbyte[2] = capbyte[3] = 0;

    } else {

      if (captime > 2) { //logic 0 is 600us pulse
        capbyte[capptr] <<= 1;
      }
      if (captime > 7) {
        capbyte[capptr] |= 1;
      }
      capbit++;

    }

    if (capbit == 8) { //we got whole byte, lets
      capptr++;
      capbit = 0;
    }

    if (capptr == 4) {
      if (capbyte[0] == 0x41 && capbyte[1] == 0xE8 && capbyte[2] == 0xFF - capbyte[3]) {
        _newCode = capbyte[2];
        _gotNewCode = 1;
        capptr = 0;
      }
    }

  }
  attachInterrupt(digitalPinToInterrupt(_inpin), &VAGRadioRemote::remoteInGoingHigh, RISING);
}

String VAGRadioRemote::decodeRemote(uint8_t code) {
  /*
       UP:       0x41E8D02F
    DOWN: 0x41E850AF
    LEFT:   0x41E840BF
    RIGHT: 0x41E8C03F
    VOL+:  0x41E8807F
    VOL-:    0x41E800FF
  */
  switch (code) {
    case VOLUMEDOWN: return F("Volume down");
    case BTN1: return F("mem/cd1");
    case BTN2: return F("mem/cd2");//Did not work on my audi concert ...
    case BTN3: return F("mem/cd3");
    case BTN4: return F("mem/cd4");//Did not work on my audi concert ...
    case BTN5: return F("mem/cd5");
    case BTN6: return F("mem/cd6");//Did not work on my audi concert ...
    case REG: return F("radio regional on/off");
    case AM: return F("AM");
    case AS: return F("AS-STORE");
    case FM: return F("FM");
    case RANDOM: return F("random");
    case LEFT:
    case 0x42:
    case 0x44:
    case 0x46: return F("LEFT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1");
    case DOWN:
    case 0x52:
    case 0x54:
    case 0x56:
    case 0x60:
    case 0x62:
    case 0x64: return F("Seek down/FR");
    case VOLUMEUP:
    case 0x81:
    case 0x82:
    case 0x83:
    case 0x84:
    case 0x85:
    case 0x86: return F("Volume up");
    case DOWN_2: return F("-2 Volume down bas/treble down/fade rear/bal left");
    case DOWN_4: return F("-4 Volume down bas/treble down/fade rear/bal left");
    case DOWN_6: return F("-6 Volume down bas/treble down/fade rear/bal left");
    case DOWN_8: return F("-8 Volume down/fade rear/bal left");
    case TP: return F("TP");
    case SCAN: return F("SCAN");
    case MODE:
    case 0xA2:
    case 0xA4:
    case 0xA6: return F("MODE");
    case RIGHT:
    case 0xC2:
    case 0xC4:
    case 0xC6: return F("RIGHT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1");
    case UP:
    case 0x07: return F("Seek up/FF");
  }
  return "unknown";
}

String VAGRadioRemote::decodeRemote(){
	return decodeRemote(_newCode);
}

uint8_t VAGRadioRemote::gotNewCode(){
	return _gotNewCode;
}

uint8_t VAGRadioRemote::newCode(){
	return _newCode;
}

void VAGRadioRemote::clearGotNewCode(){
	_gotNewCode=0;
}
#endif

#ifdef _OUTPUT
void VAGRadioRemote::up(){
	VAGRadioRemote::send(UP);
}

void VAGRadioRemote::cdUp(){
        VAGRadioRemote::send(CDUP);
}


void VAGRadioRemote::down(){
	VAGRadioRemote::send(DOWN);
}

void VAGRadioRemote::cdDown(){
        VAGRadioRemote::send(CDDOWN);
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
#endif
