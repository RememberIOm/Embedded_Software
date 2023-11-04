#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define START 1
#define STOP 0

typedef unsigned char uc;

const uc digit[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x27, 0x7f, 0x6f};
const uc fnd_sel[4] = {0x08, 0x04, 0x02, 0x01};
const uc dot = 0x80;

int count = 0, signal = STOP;

void display_fnd(const int* const count) {
	const uc fnd[] = {
		digit[*count / 1000],
		digit[*count % 1000 / 100],
		digit[*count % 100 / 10] + dot,
		digit[*count % 10]
	};

	for (int i = 0; i < 4; ++i) {
		PORTC = fnd[i];
		PORTG = fnd_sel[i];
		_delay_us(250);
	}
}

ISR(INT4_vect) {
	signal ^= 1;
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
		
		if (signal == START) {
			count = (count + 1) % 10000;
		}
	}
}
