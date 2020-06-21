/*
  (C) Tomas Kovacik
  https://github.com/tomaskovacik/
  GNU GPL3

  output example for V.A.G. radio remote control protocol library for arduino

  this example controls radio based on data send to serial port of arduino

  usage:
  connect ground of arduino to ground of radio
  connect pin 2 of arduino to remote pin of radio
  open serial monitor at 115200b
  send "h" for printing help

  this library use hardcoded timer2! it is not dependant on any of it features, any timer can be used
  input pit has to have hardware interrupt (INTx)
  
  more info:
  http://kovo-blog.blogspot.sk/2013/10/remote-control-audi-radio.html

  5V logic
  start bit:
  9ms LOW
  4.55ms HIGH

  logic 1:
  ~600us LOW ~1700us HIGH

  logic 0:
  ~600us LOW ~600us HIGH

  stop bit:
  ~600us LOW

  I have MFSW without telephone option, so I read codes just for radio:

  UP:       0x41E8D02F
  DOWN: 0x41E850AF
  LEFT:   0x41E840BF
  RIGHT: 0x41E8C03F
  VOL+:  0x41E8807F
  VOL-:    0x41E800FF

  code is always 0x41 0xE8 X 0xFF-X

  discovered codes on my audi concert I unit:

  0x00 	Volume down
  0x01 	mem/cd1
  0x03 	mem/cd3
  0x05 	mem/cd5
  0x07 	search up
  0x09 	reg on/off
  0x0B 	tp
  0x11 	AM
  0x15 	AS-STORE
  0x17 	FM
  0x1D 	search down
  0x20 	AM->AM+AS->FM1->FM2->FM1+AS->FM2->AS/in CD mode “RD”
  0x22 	AM->AM+AS->FM1->FM2->FM1+AS->FM2->AS/in CD mode “RD”
  0x24 	AM->AM+AS->FM1->FM2->FM1+AS->FM2->AS/in CD mode “RD”
  0x26 	AM->AM+AS->FM1->FM2->FM1+AS->FM2->AS/in CD mode “RD”
  0x29 	TP
  0x2B 	search up
  0x3C 	seek up
  0x40 	LEFT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
  0x42 	LEFT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
  0x44 	LEFT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
  0x46 	LEFT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
  0x50 	Seek down/FR
  0x52 	Seek down/FR
  0x54 	Seek down/FR
  0x56 	Seek down/FR
  0x60 	Seek down/FR
  0x62 	seek down
  0x64 	seek down
  0x80 	Volume up
  0x81 	Volume up
  0x82 	Volume up
  0x83 	Volume up
  0x84 	Volume up
  0x85 	Volume up
  0x86 	Volume up
  0x89 	-2 Volume down bas/treble down/fade rear/bal left
  0x8B 	-4 Volume down bas/treble down/fade rear/bal left
  0x8D 	-6 Volume down bas/treble down/fade rear/bal left
  0x8F 	-8 Volume down/fade rear/bal left
  0x97 	TP
  0x9B 	SCAN
  0xA0 	MODE
  0xA2 	MODE
  0xA4 	MODE
  0xA6 	MODE
  0xC0 	RIGHT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
  0xC2 	RIGHT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
  0xC4 	RIGHT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
  0xC6 	RIGHT FM1 6-5-4-3-2-1-FM2 6-5-4-3-2-1
  0xD0 	Seek up/FF
  0xD2 	Seek up/FF
  0xD4 	Seek up/FF
  0xD6 	Seek up/FF
  0xE0 	seek up
  0xE2 	seek up
  0xE4 	seek up
  0xE6 	seek up
*/


#include <VAGRadioRemote.h>

#define REMOTE_PIN 2

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
        Serial.println("mode");
        break;
      case 'r': //right
        remote.right();
        Serial.println("right");
        break;
      case 'l': //left
        remote.left();
        Serial.println("left");
        break;
      case 'u': //up
        remote.up();
        Serial.println("up");
        break;
      case 'd': //down
        remote.down();
        Serial.println("down");
        break;
        case 'R'://random
        remote.random();
        Serial.println("random");
        break;
      case '1': //mem 1
        remote.send(BTN1);
        Serial.println("mem 1");
        break;
      case '2': //mem 2
        remote.send(BTN2);
        Serial.println("mem 2");
        break;
      case '3': //mem 3
        remote.send(BTN3);
        Serial.println("mem 3");
        break;
      case '4': //mem 4
        remote.send(BTN4);
        Serial.println("mem 4");
        break;
      case '5': //mem 5
        remote.send(BTN5);
        Serial.println("mem 5");
        break;
      case '6': //mem 6
        remote.send(BTN6);
        Serial.println("mem 6");
        break;
      case '-': //volume down
        remote.volumeDown();
        Serial.println("volume down");
        break;
      case '+': //volume up
        remote.volumeUp();
        Serial.println("volume up");
        break;
      case 'h': //help
        remote.volumeDown();
        Serial.println("m = mode");
        Serial.println("R = random");
        Serial.println("r = right");
        Serial.println("l = left");
        Serial.println("u = up");
        Serial.println("d = down");
        Serial.println("1 = mem 1");
        Serial.println("2 = mem 2");
        Serial.println("3 = mem 3");
        Serial.println("4 = mem 4");
        Serial.println("5 = mem 5");
        Serial.println("6 = mem 6");
        Serial.println("- = volume down");
        Serial.println("+ = volume up");
        break;
    }
  }
 
}
