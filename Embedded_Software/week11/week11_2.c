#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define ON 1
#define OFF 0

enum scale_timer {
  LTI = 3,
  DO = 17, DOS = 30,
  RE = 43, RES = 55,
  MI = 66,
  FA = 77, FAS = 87,
  SOL = 97, SOLS = 105,
  LA = 114, LAS = 122,
  TI = 129,
  HDO = 137, HDOS = 143,
  HRE = 150, HRES = 156,
  HMI = 161,
  HFA = 166, HFAS = 171,
  HSOL = 176, HSOLS = 181,
  HLA = 185, HLAS = 189,
  HTI = 193,
  HHDO = 196
};

const int scale[] = {
  0, 
  LTI,
  DO, DOS, RE, RES, MI, FA, FAS, SOL, SOLS, LA, LAS, TI,
  HDO, HDOS, HRE, HRES, HMI, HFA, HFAS, HSOL, HSOLS, HLA, HLAS, HTI,
  HHDO
};
int state = ON;
int value = 0;
int timer_state = ON;

const int music[] = {
  11, 0, 13, 0, 15, 0, 8, 0, 0, 0, 0, 0, 8, 15, 13, 11, 10, 8, 10, 0, 0, 0, 0, 0,
  6, 0, 0, 0, 11, 0, 8, 0, 0, 0, 3, 1, 3, 0, 0, 0, 6, 0, 3, 0, 0, 0, 0, 0,
  1, 0, 3, 0, 6, 0, 8, 0, 0, 0, 0, 0, 8, 15, 13, 11, 10, 8, 10, 0, 0, 0, 0, 0,
  6, 8, 10, 11, 10, 0, 8, 0, 3, 0, 0, 1, 3, 0, 0, 0, 6, 0, 3, 0, 0, 0, 0, 0,
  0, 0, 0, 6, 8, 15, 20, 0, 0, 0, 0, 0, 13, 0, 0, 0, 13, 11, 13, 0, 0, 0, 0, 0,
  18, 0, 0, 0, 16, 0, 15, 0, 15, 16, 15, 0, 0, 0, 13, 15, 13, 0, 11, 0, 10, 0, 13, 0,
  11, 0, 0, 0, 10, 0, 8, 0, 0, 0, 0, 0, 4, 0, 0, 6, 8, 13, 10, 0, 0, 0, 0, 0,
  6, 0, 0, 0, 3, 0, 8, 0, 15, 16, 15, 0, 0, 0, 13, 15, 13, 0, 11, 0, 13, 0, 10, 0
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
  DDRB = 0x10;
  TCCR0 = 0x05;
  TIMSK = 0x01;
  sei();
  
  while(1) {
    for (int i = 0; i < music_size; ++i) {
      if (music[i] != 0) {
        value = scale[music[i]];
      }
      
      timer_state = ON;
      
      _delay_ms(150);
      
      if (music[(i + 1) % music_size] != 0) {
        timer_state = OFF;
      }
      
      _delay_ms(50);
    }
  }
}
