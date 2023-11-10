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

const int scale[8] = {DO, RE, MI, FA, SOL, LA, TI, HDO};
int state = ON;
int music_index = 0;
int timer_state = 0;

ISR(TIMER0_OVF_vect) {
  if (state == ON) {
    PORTB = 0x00;
    state = OFF;
  } else {
    PORTB = 0x10;
    state = ON;
  }
    
  TCNT0 = scale[music_index];
  timer_state = 1;
}

ISR(INT4_vect) {
  if (timer_state == 1) {
    music_index = (music_index + 1) % 8;
    timer_state = 0;
    _delay_ms(100);
  }
}

int main() {
  DDRB = 0x10; // ?? ??
  DDRE = 0xef; // ??? ??
  EICRB = 0x02; // ?? ??
  EIMSK = 0x10;
  TCCR0 = 0x03; // 32??
  TIMSK = 0x01;
  sei();
  
  while(1) {}
}
