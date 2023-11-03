#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>

#define START 1
#define STOP 0

typedef unsigned char uc;

const uc digit[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x27, 0x7f, 0x6f};
const uc fnd_sel[4] = {0x08, 0x04, 0x02, 0x01};
const uc dot = 0x80;

int count = 0, signal = 0;

void display_fnd(const int* const count) {
	int fnd[4];
	
	fnd[3] = (*count / 1000) % 10;
	fnd[2] = (*count / 100) % 10;
	fnd[1] = (*count / 10) % 10;
	fnd[0] = *count % 10;

	for (int i = 0; i < 4; ++i) {
		PORTC = digit[fnd[i]];
		PORTG = fnd_sel[i];
		_delay_ms(2);
	}
}

ISR(INT4_vect) {
	if (signal == START) {
		signal = STOP;
	} else {
		signal = START;
	}
}

ISR(INT5_vect) {
	if (signal == STOP) {
		count = 0;
	}
}

int main() {
	DDRC = 0xff;
	DDRG = 0x0f;
	DDRE = 0xcf;
	EICRB = 0x0a;
	EIMSK = 0x30;
	SREG |= 1 << 7;
	
	while (1) {
		display_fnd(&count);
	}
}
