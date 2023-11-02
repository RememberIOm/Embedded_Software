#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

int main() {
	unsigned char value = 0;
	int music[] = {3, 2, 1, 2, 3, 3, 3, 0, 2, 2, 2, 0, 3, 5, 5, 0,
	3, 2, 1, 2, 3, 3, 3, 0, 2, 2, 3, 2, 1, 0, 0, 0};
	DDRA= 0xff;
	
	for (;;) {
		for (int i = 0; i < 32; ++i) {
			PORTA = value;
			_delay_ms(200);
			
			if (music[i] == 0) {
				// pass
				} else {
				value = 1 << (8 - music[i]);
				PORTA = 0;
			}
			
			_delay_ms(200);
		}
	}
}
