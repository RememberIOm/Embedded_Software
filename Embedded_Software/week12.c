#include "includes.h"

#define F_CPU 16000000UL // CPU frequency = 16 Mhz
#include <avr/io.h>
#include <util/delay.h>

#define UCHAR unsigned char   // define UCHAR
#define USHORT unsigned short // define USHORT
#define ATS75_CONFIG_REG 1
#define ATS75_TEMP_REG 0
#define ATS75_ADDR 0x98 // Shift left by 1 bit 0b00110000

#define TASK_STK_SIZE OS_TASK_DEF_STK_SIZE
#define N_TASKS 3
#define SEND_QUEUE_SIZE 4

// 1. Please provide declarations related to a mailbox or a message queue.
OS_EVENT *TxMbox;
OS_EVENT *SendQueue;

OS_STK TaskStk[N_TASKS][TASK_STK_SIZE];

void *SendQueueTbl[SEND_QUEUE_SIZE];

void write_twi_1byte_nopreset(UCHAR reg, UCHAR data);
void write_twi_0byte_nopreset(UCHAR reg);
void TemperatureTask(void *data);
void FndTask(void *data);
void FndDisplayTask(void *data);

int main(void) {
  OSInit();
  OS_ENTER_CRITICAL();
  TCCR0 = 0x07;
  TIMSK = _BV(TOIE0);
  TCNT0 = 256 - (CPU_CLOCK_HZ / OS_TICKS_PER_SEC / 1024);
  OS_EXIT_CRITICAL();

  TxMbox = OSMboxCreate((void *)0);
  SendQueue = OSQCreate(&SendQueueTbl[0], SEND_QUEUE_SIZE);

  OSTaskCreate(TemperatureTask, (void *)0,
               (void *)&TaskStk[0][TASK_STK_SIZE - 1], 0);
  OSTaskCreate(FndTask, (void *)0, (void *)&TaskStk[1][TASK_STK_SIZE - 1], 1);
  OSTaskCreate(FndDisplayTask, (void *)0,
               (void *)&TaskStk[2][TASK_STK_SIZE - 1], 2);
  OSStart();

  return 0;
}

void InitI2C() {
  PORTD = 3;                    // For Pull-up override value
  SFIOR &= ~(1 << PUD);         // PUD
  TWSR = 0;                     // TWPS0 = 0, TWPS1 = 0
  TWBR = 32;                    // for 100  K Hz bus clock
  TWCR = _BV(TWEA) | _BV(TWEN); // TWEA = Ack pulse is generated
                                // TWEN enables TWI operation
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

  // Since the temperature sensor provides values in 16-bit format, we need to
  // receive it in two 8-bit parts.
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
    // 2.Please write mailbox code."
    OSMboxPost(TxMbox, (void *)&value);

    OSTimeDlyHMSM(0, 0, 1, 0);
  }
}

void FndTask(void *data) {
  INT8U err;
  data = data;
  UCHAR value_int, value_deci, num[4];
  USHORT now_value;

  while (1) {
    // 3. Please provide code for a mailbox, message queue, and temperature data
    // extraction.
    now_value = *(USHORT *)OSMboxPend(TxMbox, 0, &err);

    if ((now_value & 0x8000) != 0x8000) {
      num[3] = 11;
    } else {
      num[3] = 10;
      now_value = ~now_value - 1;
    }

    value_int = (UCHAR)((now_value & 0x7f00) >> 8);
    value_deci = (UCHAR)(now_value & 0x00ff);

    num[2] = (value_int / 10) % 10;
    num[1] = value_int % 10;
    num[0] = ((value_deci & 0x80) == 0x80) * 5;

    OSQPost(SendQueue, (void *)&num[0]);

    OSTimeDlyHMSM(0, 0, 1, 0);
  }
}

void FndDisplayTask(void *data) {
  UCHAR FND_DATA[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d,
                      0x7c, 0x07, 0x7f, 0x67, 0x40, 0x00};
  UCHAR fnd_sel[4] = {0x01, 0x02, 0x04, 0x08};
  UCHAR *display;
  INT8U err;
  int i;

  DDRC = 0xff;
  DDRG = 0x0f;

  // 4. Please write code for a message queue and screen output."
  display = (UCHAR *)OSQPend(SendQueue, 0, &err);

  while (1) {
    for (i = 0; i < 4; i++) {
      PORTC = FND_DATA[display[i]];
      PORTG = fnd_sel[i];
      if (i == 1)
        PORTC |= 0x80;
      OSTimeDlyHMSM(0, 0, 0, 1);
    }
  }
}

void write_twi_1byte_nopreset(UCHAR reg, UCHAR data) {
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // transmitting START
  while (((TWCR & (1 << TWINT)) == 0x00) ||
         ((TWSR & 0xf8) != 0x08 && (TWSR & 0xf8) != 0x10))
    ;                                // wait for ACK
  TWDR = ATS75_ADDR | 0;             // SLA+W ready, W=0
  TWCR = (1 << TWINT) | (1 << TWEN); // transmitting SLA+W
  while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x18)
    ;
  TWDR = reg;                        // aTS75 Reg value ready
  TWCR = (1 << TWINT) | (1 << TWEN); // Transmitting aTS75 Register Value
  while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xF8) != 0x28)
    ;
  TWDR = data;                       // DATA ready
  TWCR = (1 << TWINT) | (1 << TWEN); // transmitting DATA
  while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xF8) != 0x28)
    ;
  TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); // transmitting STOP
}
void write_twi_0byte_nopreset(UCHAR reg) {
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // transmitting START
  while (((TWCR & (1 << TWINT)) == 0x00) ||
         ((TWSR & 0xf8) != 0x08 && (TWSR & 0xf8) != 0x10))
    ;                                // wait for ACK
  TWDR = ATS75_ADDR | 0;             // SLA+W ready, W=0
  TWCR = (1 << TWINT) | (1 << TWEN); // transmitting SLA+W
  while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x18)
    ;
  TWDR = reg;                        // aTS75 Reg value ready
  TWCR = (1 << TWINT) | (1 << TWEN); // transmitting aTS75 Reg value
  while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xF8) != 0x28)
    ;
  TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); // transmitting STOP
}
