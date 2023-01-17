#ifndef __GPIO_H
#define __GPIO_H

#include <stdint.h>

#define BIT(n)  (1 << n)
#define REDLED      1,0
#define S1          2,1
#define GREENLED    4,7
#define S2          1,1
#define SDA         2,5
#define SCL         2,4

typedef enum {input, output, inPullUp, inPullDown} pinMode_t;
typedef enum {us, ms, sec} timeUnit_t;

//GPIO-----------------------------------
void pinMode(uint8_t port, uint8_t bit, pinMode_t mode);
uint8_t pinRead(uint8_t port, uint8_t bit);
void pinWrite(uint8_t port, uint8_t bit, uint8_t value);
void pinInit();

//TIMER-----------------------------------
void wait(uint16_t time,timeUnit_t unit);

//I2C-----------------------------------
void release(uint8_t port, uint8_t bit);
void pullow(uint8_t port, uint8_t bit);

void writeBit(uint8_t value);
uint8_t readBit();

uint8_t writeByte(uint8_t value);
uint8_t readByte(uint8_t nack);

void start();
void stop();

uint8_t i2cWrite(uint8_t addr, uint8_t * data, uint8_t size);
uint8_t i2cRead(uint8_t addr, uint8_t * data, uint8_t size);


#endif //__GPIO_H
