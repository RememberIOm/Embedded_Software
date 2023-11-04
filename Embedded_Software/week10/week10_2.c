#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>

typedef unsigned char uc;

const uc digit[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x27, 0x7f, 0x6f};

void printNum(const uc* const num) {
	PORTC = *num;
	PORTG = 0x0f;
}

void printLED(const int* const num) {
	PORTA = 1 << (7 - *num);
}

int main() {
	DDRA = 0xff;
	DDRC = 0xff;
	DDRG = 0x0f;
	
	while (1) {
		const int rand_num = rand() % 8;
		const uc num = digit[rand_num];
		
		printNum(&num);
		printLED(&rand_num);
		
		_delay_ms(500);
	}
}
