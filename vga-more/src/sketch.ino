/*
 VGA video generation
 
 Author:   Nick Gammon
 Date:     20th April 2012
 
 Example code without I2C communications.

 Connections:
 
 D1 : Pixel output (180 ohms in series) (connect to R, G, B tied together)   --> Pins 1, 2, 3 on DB15 socket
 D3 : Horizontal Sync (68 ohms in series) --> Pin 13 on DB15 socket
 D10 : Vertical Sync (68 ohms in series) --> Pin 14 on DB15 socket
 
 Gnd : --> Pins 5, 6, 7, 8, 10 on DB15 socket

*/

#include "TimerHelpers.h"
#include <avr/pgmspace.h>
#include "screenFont.h"
#include <avr/sleep.h>

#define BETA_ARDUINO ARDUINO < 100

const byte pixelPin = 1;     // <------- Pixel data
const byte hSyncPin = 3;     // <------- HSYNC
const byte MSPIM_SCK = 4;    // <-- we aren't using it directly
const byte vSyncPin = 10;    // <------- VSYNC

const int horizontalBytes = 20;  // 160 pixels wide
const int verticalPixels = 480;  // 480 pixels high

const byte i2cAddress = 42;

// Timer 1 - Vertical sync

// output    OC1B   pin 16  (D10) <------- VSYNC

//   Period: 16.64 mS (60 Hz)
//      1/60 * 1e6 = 16666.66 uS
//   Pulse for 64 uS  (2 x HSync width of 32 uS)
//    Sync pulse: 2 lines
//    Back porch: 33 lines
//    Active video: 480 lines
//    Front porch: 10 lines
//       Total: 525 lines

// Timer 2 - Horizontal sync

// output    OC2B   pin 5  (D3)   <------- HSYNC

//   Period: 32 uS (31.25 kHz)
//      (1/60) / 525 * 1e6 = 31.74 uS
//   Pulse for 4 uS (96 times 39.68 nS)
//    Sync pulse: 96 pixels
//    Back porch: 48 pixels
//    Active video: 640 pixels
//    Front porch: 16 pixels
//       Total: 800 pixels

// Pixel time =  ((1/60) / 525 * 1e9) / 800 = 39.68  nS
//  frequency =  1 / (((1/60) / 525 * 1e6) / 800) = 25.2 MHz

// However in practice, it is the SPI speed, namely a period of 125 nS
//     (that is 2 x system clock speed)
//   giving an 8 MHz pixel frequency. Thus the characters are about 3 times too wide.
// Thus we fit 160 of "our" pixels on the screen in what usually takes 3 x 160 = 480

const byte screenFontHeight = 8;
const byte screenFontWidth = 8;

const int verticalLines = verticalPixels / screenFontHeight / 2;  // double-height characters
const int horizontalPixels = horizontalBytes * screenFontWidth;

const byte verticalBackPorchLines = 35;  // includes sync pulse?
const int verticalFrontPorchLines = 525 - verticalBackPorchLines;

volatile int vLine;
volatile int messageLine;
volatile byte backPorchLinesToGo;

char message [verticalLines]  [horizontalBytes];

// ISR: Vsync pulse
ISR (TIMER1_OVF_vect)
  {
  vLine = 0;
  messageLine = 0;
  backPorchLinesToGo = verticalBackPorchLines;
  } // end of TIMER1_OVF_vect
  
// ISR: Hsync pulse ... this interrupt merely wakes us up
ISR (TIMER2_OVF_vect)
  {
  } // end of TIMER2_OVF_vect
    
void setup()
  {
  
  // initial message ... change to suit
  for (int i = 0; i < verticalLines; i++)
    sprintf (message [i], "Line %03i - hello!", i);
   
  // disable Timer 0
  TIMSK0 = 0;  // no interrupts on Timer 0
  OCR0A = 0;   // and turn it off
  OCR0B = 0;
  
  // Timer 1 - vertical sync pulses
  pinMode (vSyncPin, OUTPUT); 
  Timer1::setMode (15, Timer1::PRESCALE_1024, Timer1::CLEAR_B_ON_COMPARE);
  OCR1A = 259;  // 16666 / 64 uS = 260 (less one)
  OCR1B = 0;    // 64 / 64 uS = 1 (less one)
  TIFR1 = _BV (TOV1);   // clear overflow flag
  TIMSK1 = _BV (TOIE1);  // interrupt on overflow on timer 1

  // Timer 2 - horizontal sync pulses
  pinMode (hSyncPin, OUTPUT); 
  Timer2::setMode (7, Timer2::PRESCALE_8, Timer2::CLEAR_B_ON_COMPARE);
  OCR2A = 63;   // 32 / 0.5 uS = 64 (less one)
  OCR2B = 7;    // 4 / 0.5 uS = 8 (less one)
  TIFR2 = _BV (TOV2);   // clear overflow flag
  TIMSK2 = _BV (TOIE2);  // interrupt on overflow on timer 2
 
  // Set up USART in SPI mode (MSPIM)
  
  // baud rate must be zero before enabling the transmitter
  UBRR0 = 0;  // USART Baud Rate Register
  pinMode (MSPIM_SCK, OUTPUT);   // set XCK pin as output to enable master mode
  UCSR0B = 0; 
  UCSR0C = _BV (UMSEL00) | _BV (UMSEL01) | _BV (UCPHA0) | _BV (UCPOL0);  // Master SPI mode

  // prepare to sleep between horizontal sync pulses  
  set_sleep_mode (SLEEP_MODE_IDLE);  
 
}  // end of setup

// draw a single scan line
void doOneScanLine ()
  {
    
  // after vsync we do the back porch
  if (backPorchLinesToGo)
    {
    backPorchLinesToGo--;
    return;   
    }  // end still doing back porch
    
  // if all lines done, do the front porch
  if (vLine >= verticalPixels)
    return;
    
  // pre-load pointer for speed
  const register byte * linePtr = &screen_font [ (vLine >> 1) & 0x07 ] [0];
  register char * messagePtr =  & (message [messageLine] [0] );

  // how many pixels to send
  register byte i = horizontalBytes;

  // turn transmitter on 
  UCSR0B = _BV (TXEN0);  // transmit enable (starts transmitting white)

  // blit pixel data to screen    
  while (i--)
    UDR0 = pgm_read_byte (linePtr + (* messagePtr++));

  // wait till done    
  while (!(UCSR0A & _BV(TXC0))) 
    {}
  
  // disable transmit
  UCSR0B = 0;   // drop back to black
  
  // this line done
  vLine++;  
  
  // every 16 pixels it is time to move to a new line in our text
  //  (because we double up the characters vertically)
  if ((vLine & 0xF) == 0)
    messageLine++;
    
  }  // end of doOneScanLine

void loop() 
  {
  // sleep to ensure we start up in a predictable way
  sleep_mode ();
  doOneScanLine ();
 }  // end of loop
