/*
  pollserial.h Heavily modified version of:

  HardwareSerial.h - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  Modified July 2010 by Myles D. Metzler
*/

#ifndef PSERIAL_H
#define PSERIAL_H

#if defined(__AVR_ATmega32U4__)
// Serial 0 on leonardo is used for USB, thus we need to use Serial 1.
#define UDR0 UDR1
#define UCSR0A UCSR1A
#define RXC0 RXC1
#define U2X0 U2X1
#define UBRR0 UBRR1
#define UCSR0B UCSR1B
#define TXEN0 TXEN1
#define RXEN0 RXEN1
#define UDRE0 UDRE1
#endif

#include <inttypes.h>
#include "Print.h"

typedef struct {
	uint8_t head;
	uint8_t tail;
	unsigned char * buffer;
} rbuffer;

//define a void function() return type.
typedef void (*pt2Funct)();

class pollserial : public Print {
	public:
		pt2Funct begin(long);
		void end();
		uint8_t available(void);
		int read(void);
		void flush(void);
		virtual size_t write(uint8_t);
		using Print::write; // pull in write(str) and write(buf, size) from Print
};

void USART_recieve();
#endif
