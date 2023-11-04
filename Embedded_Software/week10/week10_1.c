#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

typedef unsigned char uc;

const uc digit[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x27, 0x7f, 0x6f};
const uc fnd_sel[4] = {0x08, 0x04, 0x02, 0x01};
const uc dot = 0x80;

void printNum(const int* const num) {
	const uc fnd[] = {
		digit[*num / 1000],
		digit[*num % 1000 / 100],
		digit[*num % 100 / 10] + dot,
		digit[*num % 10]
	};

	for (int i = 0; i < 4; ++i) {
		PORTC = fnd[i];
		PORTG = fnd_sel[i];
		_delay_us(250);
	}
}

int main() {
	DDRC = 0xff;
	DDRG = 0x0f;
	
	int num = 0;
	
	while (1) {
		num = (num + 1) % 10000;
			
		printNum(&num);
	}
}
