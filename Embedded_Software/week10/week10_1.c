#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

typedef unsigned char uc;

const uc digit[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x27, 0x7f, 0x6f};
const uc fnd_sel[4] = {0x08, 0x04, 0x02, 0x01};
const uc dot = 0x80;

void printNum(const uc const num[]) {
	for (int i = 0; i < 4; ++i) {
		PORTC = num[i];
		PORTG = fnd_sel[i];
		_delay_us(250);
	}
}

int main() {
	DDRC = 0xff;
	DDRG = 0x0f;
	
	uc num[4];
	
	while (1) {
		for (int i = 0; i < 10000; ++i) {
			num[0] = digit[i / 1000];
			num[1] = digit[i % 1000 / 100] + dot;
			num[2] = digit[i % 100 / 10];
			num[3] = digit[i % 10];
			
			printNum(num);
		}
	}
}
