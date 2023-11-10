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
  DDRB = 0x10; // buzzer output
  DDRE = 0xef; // switch input
  EICRB = 0x02; // falling edge
  EIMSK = 0x10;
  TCCR0 = 0x03; // 32 scale
  TIMSK = 0x01;
  sei();
  
  while(1) {}
}
