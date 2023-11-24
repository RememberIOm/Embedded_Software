#include "includes.h"

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#define UCHAR unsigned char
#define USHORT unsigned short
#define ATS75_CONFIG_REG 1
#define ATS75_TEMP_REG 0
#define ATS75_ADDR 0x98

#define TASK_STK_SIZE OS_TASK_DEF_STK_SIZE
#define N_TASKS 4

OS_STK TaskStk[N_TASKS][TASK_STK_SIZE];
OS_EVENT *Mbox;

// define Event flag
OS_FLAG_GRP *EventFlag;

UCHAR send[4];

void write_twi_1byte_nopreset(UCHAR reg, UCHAR data);
void write_twi_0byte_nopreset(UCHAR reg);
void TemperatureTask(void *data);
void FndTask(void *data);
void FndDisplayTask(void *data);
void LEDTask(void *data);

int main(void) {
  OSInit();
  OS_ENTER_CRITICAL();
  TCCR0 = 0x07;
  TIMSK = _BV(TOIE0);
  TCNT0 = 256 - (CPU_CLOCK_HZ / OS_TICKS_PER_SEC / 1024);
  OS_EXIT_CRITICAL();
  INT8U err;

  DDRA = 0xff;

  // Create an event flag,
  EventFlag = OSFlagCreate(0x00, &err);
  Mbox = OSMboxCreate((void *)0);

  OSTaskCreate(TemperatureTask, (void *)0,
               (void *)&TaskStk[0][TASK_STK_SIZE - 1], 0);
  OSTaskCreate(FndTask, (void *)0, (void *)&TaskStk[1][TASK_STK_SIZE - 1], 1);
  OSTaskCreate(LEDTask, (void *)0, (void *)&TaskStk[2][TASK_STK_SIZE - 1], 2);
  OSTaskCreate(FndDisplayTask, (void *)0,
               (void *)&TaskStk[3][TASK_STK_SIZE - 1], 3);
  OSStart();

  return 0;
}

void InitI2C() {
  PORTD = 3;                    // For Pull-up override value
  SFIOR &= ~(1 << PUD);         // PUD
  TWSR = 0;                     // TWPS0 = 0, TWPS1 = 0
  TWBR = 32;                    // for 100  K Hz bus clock
  TWCR = _BV(TWEA) | _BV(TWEN); // TWEA = Ack pulse is generated
                                // TWEN = TWI
}

int ReadTemperature(void) {
  int value;

  TWCR = _BV(TWSTA) | _BV(TWINT) | _BV(TWEN);
  while (!(TWCR & _BV(TWINT)))
    ;

  TWDR = 0x98 + 1; // TEMP_I2C_ADDR + 1
  TWCR = _BV(TWINT) | _BV(TWEN);
  while (!(TWCR & _BV(TWINT)))
    ;

  TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);
  while (!(TWCR & _BV(TWINT)))
    ;

  value = TWDR;
  TWCR = _BV(TWINT) | _BV(TWEN);
  while (!(TWCR & _BV(TWINT)))
    ;

  value = ((value << 8) | TWDR);
  TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);

  // value >>= 8;
  TIMSK = (value >= 33) ? TIMSK | _BV(TOIE2) : TIMSK & ~_BV(TOIE2);

  return value;
}

void TemperatureTask(void *data) {
  data = data;
  USHORT value;
  InitI2C();

  write_twi_1byte_nopreset(ATS75_CONFIG_REG, 0x00); // 9bits, Normal
  write_twi_0byte_nopreset(ATS75_TEMP_REG);
  while (1) {
    value = ReadTemperature();
    OSMboxPost(Mbox, (void *)&value);
    OSTimeDlyHMSM(0, 0, 1, 0);
  }
}

void FndTask(void *data) {
  INT8U err;
  data = data;
  int i;
  UCHAR value_int, value_deci, num[4];
  USHORT now_value;

  while (1) {
    now_value = *(USHORT *)OSMboxPend(Mbox, 0, &err);

    if ((now_value & 0x8000) != 0x8000) // Sign bit check
      num[3] = 11;
    else {
      num[3] = 10;
      now_value = (~now_value) - 1; // 2��s Compliment
    }

    value_int = (UCHAR)((now_value & 0x7f00) >> 8);
    value_deci = (UCHAR)(now_value & 0x00ff);

    num[2] = (value_int / 10) % 10;
    num[1] = value_int % 10;
    num[0] = ((value_deci & 0x80) == 0x80) * 5;

    // Fill the send array and call the OSFlagPost function,
    for (i = 0; i < 4; i++) {
      send[i] = num[i];
      OSFlagPost(EventFlag, 0x01 << i, OS_FLAG_SET, &err);
    }

    OSTimeDlyHMSM(0, 0, 2, 0);
  }
}

void LEDTask(void *data) {
  INT8U err;
  UCHAR value_int;
  UCHAR value_result = 0;
  int i;

  while (1) {
    // Call the OSFlagPend function and print the result
    OSFlagPend(EventFlag, 0x0f, OS_FLAG_WAIT_SET_ALL, 0, &err);

    PORTA = 0x00;

    for (i = 0; i < 4; ++i) {
      value_int = send[i];

      if (value_int == 8 || value_int == 9) {
        value_result == 0x00;
      } else {
        value_result = 1 << value_int;
      }

      PORTA |= value_result;
    }

    OSTimeDlyHMSM(0, 0, 1, 0);
  }
}

void FndDisplayTask(void *data) {
  UCHAR FND_DATA[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d,
                      0x7c, 0x07, 0x7f, 0x67, 0x40, 0x00};
  UCHAR fnd_sel[4] = {0x01, 0x02, 0x04, 0x08};
  INT8U err;
  int i;

  DDRC = 0xff;
  DDRG = 0x0f;

  // Call the OSFlagPend function
  OSFlagPend(EventFlag, 0x0f, OS_FLAG_WAIT_SET_ALL, 0, &err);

  while (1) {
    for (i = 0; i < 4; i++) {
      // print the result
      PORTC = FND_DATA[send[i]];
      PORTG = fnd_sel[i];
      if (i == 1)
        PORTC |= 0x80;
      OSTimeDlyHMSM(0, 0, 0, 1);
    }
  }
}

void write_twi_1byte_nopreset(UCHAR reg, UCHAR data) {
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
  while (((TWCR & (1 << TWINT)) == 0x00) ||
         ((TWSR & 0xf8) != 0x08 && (TWSR & 0xf8) != 0x10))
    ;
  TWDR = ATS75_ADDR | 0;
  TWCR = (1 << TWINT) | (1 << TWEN);
  while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x18)
    ;
  TWDR = reg;
  TWCR = (1 << TWINT) | (1 << TWEN);
  while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xF8) != 0x28)
    ;
  TWDR = data;
  TWCR = (1 << TWINT) | (1 << TWEN);
  while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xF8) != 0x28)
    ;
  TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}
void write_twi_0byte_nopreset(UCHAR reg) {
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
  while (((TWCR & (1 << TWINT)) == 0x00) ||
         ((TWSR & 0xf8) != 0x08 && (TWSR & 0xf8) != 0x10))
    ;
  TWDR = ATS75_ADDR | 0;
  TWCR = (1 << TWINT) | (1 << TWEN);
  while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x18)
    ;
  TWDR = reg;
  TWCR = (1 << TWINT) | (1 << TWEN);
  while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xF8) != 0x28)
    ;
  TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}
