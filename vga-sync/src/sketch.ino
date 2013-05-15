/* VGA Sync Generator
 * 11/11/08 - dwan : dwanafite@yahoo.fr
 *
 * Based on :
 *  - RG Matrix Example v.2 8/1/08, by BroHogan, from the Arduino Playground
 *  - Simplest universal VGA/PAL terminal, by Ibragimov Maxim Rafikovich, http://www.vga-avr.narod.ru/main.html
 *  - the very useful Timer/Counter/Prescaler Calculator, http://www.et06.dk/atmega_timers/
 *  - Arduino.cc Port Manipulation Tutorial
 *
 * This program outputs pretty accurate VGA syncronization signals. It's using a timer interrupt on timer2, so hopefully you can make other cool things in loop().
 */

// 640 * 480 @ 60Hz - FvSync = 60.3 Hz / FhSync = 31.3 kHz
// HSync : pin 7 Arduino, pin 13 VGA
// VSync : pin 6 Arduino, pin 14 VGA
// Arduino's pin 5 is HIGH when video can be sent, LOW otherwise. I use it to power a transistor. See this wonderful page : http://www.anatekcorp.com/driving.htm
// 1 NOP = 62,5 ns wasted

#define NOP asm("nop")

#define vga_field_line_count 525         // number of VGA lines
#define ISR_FREQ 0x3F                    // Sets the speed of the ISR - LOWER IS FASTER - 62

volatile unsigned int linecount;

void setup()
{
  //Serial.begin(9600);         // set up Serial library at 9600 bps
  DDRD |= B11100000;            // it sets pins 7, 6 and 5 as output without changing the value of pins 0 & 1, which are RX & TX
  //       76543210 <- pin translation

  PORTD |= B11000000;   // sets pins 7 (hSync) and 6 (vSync) HIGH
  //        76543210;
  
  setISRtimer();                        // setup the timer
  startISR();                           // start the timer to toggle shutdown
}

void loop()
{ 
 
}

///////////////////////////// ISR Timer Functions ///////////////////////////
ISR(TIMER2_COMPA_vect)
{
        // Stop video
        // pin 5 LOW
        PORTD &= ~(1 << 5);
        
        // Front porch
        NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP;
        
        
        // Count number of lines
    if (++linecount == vga_field_line_count)
    {
    linecount = 0;
    }

    // can it be vsync tiem nao ?
    if ((linecount == 10 )||(linecount == 11 ))
    {
        // hsync LOW
        // vsync LOW
        PORTD &= ~(1 << 7);
        PORTD &= ~(1 << 6);
    }
    else    // ,hsync only
    {
        // hsync LOW
        // vsync HIGH
        PORTD &= ~(1 << 7);
        PORTD |= (1 << 6);
    }
    
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP;
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP;
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP;
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP;
       
        // nonetheless,
    // hsync HIGH
    PORTD |= (1 << 7); 
    
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP;
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP;
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP;
    NOP;
    
    // Start video if we are in a visible area
        if (linecount > 45)
        {
        // pin 5 HIGH
        PORTD |= (1 << 5);
        }
}

void setISRtimer(){  // setup ISR timer controling toggleing
  TCCR2A = 0x02;                        // WGM22=0 + WGM21=1 + WGM20=0 = Mode2 (CTC)
  TCCR2B = (1 << CS01);                 // /8 prescaler (2MHz)
  TCNT2 = 0;                            // clear counter
  OCR2A = ISR_FREQ;                     // set TOP (divisor) - see #define
}

void startISR(){  // Starts the ISR
  TCNT2 = 0;                            // clear counter (needed here also)
  TIMSK2|=(1<<OCIE2A);                  // set interrupts=enabled (calls ISR(TIMER2_COMPA_vect)
}

void stopISR(){    // Stops the ISR
  TIMSK2&=~(1<<OCIE2A);                  // disable interrupts
}
