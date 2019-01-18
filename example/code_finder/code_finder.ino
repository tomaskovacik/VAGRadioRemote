/*

license: GPL 

code finder for audi radios (probably also for vw, skoda, seat)
using ebay LCD shield(2x16) with 5 buttons on A0 
output (remote signal) is on pin 2

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

*/
#include "VAGRadioRemote.h"
#include <LiquidCrystal.h>
#define REMOTE_PIN 2

//lcd pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
VAGRadioRemote remote(REMOTE_PIN);

int adc_key_val[5] ={
  30, 150, 360, 535, 760 };
int NUM_KEYS = 5;
int adc_key_in;
int key=-1;
int oldkey=-1;

uint8_t h=0x00;

void setup() {
  remote.begin();
  lcd.begin(16,2);
  lcd.home();
  lcd.clear();
  Serial.begin(9600);
}

void loop() {

  adc_key_in = analogRead(0);    // read the value from the sensor
  key = get_key(adc_key_in);     // convert into key press
  if (key != oldkey)             // if keypress is detected
  {
    delay(50);          // wait for debounce time
    adc_key_in = analogRead(0);    // read the value from the sensor
    key = get_key(adc_key_in);                  // convert into key press
    if (key != oldkey)
    {
      oldkey = key;
      switch (key)
      {
      case 0:
        // righ - increment and send
        h++;
        remote.send(h);
        break;
      case 1:
        //increment but dont send
        h++;
        break;
      case 2:
        //decrement but dont send
        h--; 
        break;
      case 3:
        // left - decrement and send
        if(h>0)
        h--; 
        remote.send(h);
        break;
      case 4:
        // select - repeat sending of last number
        remote.send(h);
        break;
      }
  lcd.clear();
  lcd.home();
  lcd.print(h,HEX);

     }
    }
  }
 

// Convert ADC value to key number
int get_key(unsigned int input)
{
  int k;
  for (k = 0; k < NUM_KEYS; k++)
  {
    if (input < adc_key_val[k])
    {
      return k;
    }
  }
  if (k >= NUM_KEYS)
    k = -1;     // No valid key pressed
  return k;
}
