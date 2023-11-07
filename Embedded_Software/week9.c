#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

int main() {
	const int music[] = {
    3, 2, 1, 2, 3, 3, 3, 0, 2, 2, 2, 0, 3, 5, 5, 0,
    3, 2, 1, 2, 3, 3, 3, 0, 2, 2, 3, 2, 1, 0, 0, 0
    };
					
	const int music_size = sizeof(music) / sizeof(int);
					
	DDRA = 0xff;
	
	unsigned char value = 0;
	
	for (;;) {
		for (int i = 0; i < music_size; ++i) {
			if (music[i] != 0) {
				value = 1 << (8 - music[i]);
			}
			
			PORTA = value;
			
			_delay_ms(200);
			
			if (music[(i + 1) % music_size] != 0) {
				PORTA = 0;
			}
			
			_delay_ms(100);
		}
	}
}
