#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define DO 17
#define RE 43
#define MI 66
#define FA 77
#define SOL 97
#define LA 114
#define TI 129
#define HDO 137

#define ON 1
#define OFF 0

const int scale[] = {0, DO, RE, MI, FA, SOL, LA, TI, HDO};
int state = ON;
int value = 0;
int timer_state = ON;

const int music[] = {
  3, 2, 1, 2, 3, 3, 3, 0, 2, 2, 2, 0, 3, 5, 5, 0,
  3, 2, 1, 2, 3, 3, 3, 0, 2, 2, 3, 2, 1, 0, 0, 0
};
const int music_size = sizeof(music) / sizeof(int);

ISR(TIMER0_OVF_vect) {
  if (timer_state == ON) {
    if (state == ON) {
      PORTB = 0x00;
      state = OFF;
      } else {
      PORTB = 0x10;
      state = ON;
    }
  
    TCNT0 = value;
  }  
}

int main() {
  DDRB = 0x10; // ?? ??
  TCCR0 = 0x03; // 32??
  TIMSK = 0x01;
  sei();
  
  while(1) {
    for (int i = 0; i < music_size; ++i) {
      if (music[i] != 0) {
        value = scale[music[i]];
      }
      
      timer_state = ON;
      
      _delay_ms(200);
      
      if (music[(i + 1) % music_size] != 0) {
        timer_state = OFF;
      }
      
      _delay_ms(100);
    }
  }
}
