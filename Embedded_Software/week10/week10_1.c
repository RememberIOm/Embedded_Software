#define F_CPU 16000000UL
#include <avr/io.h>
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

int main() {
	DDRC = 0xff;
	DDRG = 0x0f;
	
	uc num[4];
	
	while (1) {
		for (int i = 0; i < 10; ++i) {
			num[0] = digit[i];
		
			for (int j = 0; j < 10; ++j) {
				num[1] = digit[j] + dot;
			
				for (int k = 0; k < 10; ++k) {
					num[2] = digit[k];
				
					for (int l = 0; l < 10; ++l) {
						num[3] = digit[l];
					
						printNum(&num);
					}
				}
			}
		}
	}
}
