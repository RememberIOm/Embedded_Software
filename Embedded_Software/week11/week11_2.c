#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define ON 1
#define OFF 0

enum scale_timer {
  DO = 17,
  RE = 43,
  MI = 66,
  FA = 77,
  SOL = 97,
  LA = 114,
  TI = 129,
  HDO = 137
};

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
