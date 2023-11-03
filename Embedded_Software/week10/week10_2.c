#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>

typedef unsigned char uc;

const uc digit[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x27, 0x7f, 0x6f};
const uc fnd_sel[4] = {0x08, 0x04, 0x02, 0x01};
const uc dot = 0x80;

void printNum(const uc* const arr[4]) {
	for (int i = 0; i < 4; ++i) {
		PORTC = *arr[4 - i];
		PORTG = fnd_sel[i];
		_delay_ms(2);
	}
}

void printLED(const int* const num) {
	PORTA = 1 << (7 - *num);
	_delay_ms(2);
}

int main() {
	DDRA = 0xff;
	DDRC = 0xff;
	DDRG = 0x0f;
	
	uc num[4];
	
	while (1) {
		const int rand_num = rand() & 8;
		
		for (int i = 0; i < 4; ++i) {
			num[i] = digit[rand_num];
		}
		
		printNum(&num);
		printLED(&rand_num);
		
		_delay_ms(500);
	}
}
