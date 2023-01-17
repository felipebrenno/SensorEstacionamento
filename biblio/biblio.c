#include <msp430.h> 
#include <stdint.h>
#include "biblio.h"

volatile  uint8_t  *PIN[]   =  {  &P1IN,  &P2IN,  &P3IN,  &P4IN,  &P5IN,  &P6IN,  &P7IN};
volatile  uint8_t  *PDIR[]  =  {  &P1DIR,  &P2DIR,  &P3DIR,  &P4DIR,  &P5DIR,  &P6DIR,  &P7DIR}; //ponteiros para pos da memorias
volatile  uint8_t  *PREN[]  =  {  &P1REN,  &P2REN,  &P3REN,  &P4REN,  &P5REN,  &P6REN,  &P7REN};
volatile  uint8_t  *POUT[]  =  {  &P1OUT,  &P2OUT,  &P3OUT,  &P4OUT,  &P5OUT,  &P6OUT,  &P7OUT};

void pinMode(uint8_t port, uint8_t bit, pinMode_t mode) //Vamos configurar o pino dependendo em como o usuario vai pedir
{

    //bit = [0,1,2,... 7]
    //  -->  0x01  (0000.0001)
    //  -->  0x02  (0000.0010)
    //  -->  0x04  (0000.1000)
    //  -->  ...
    //  -->  0x80  (1000.0001)

    uint8_t mask = (1 << bit);

    if(mode == input)
    {
       *(  PDIR[port - 1]  )  &=  ~mask;

    }

    if(mode == output)
    {
       *(  PDIR[port - 1]  )  |=  mask;

    }

    if(mode == inPullUp)
    {
        *(  PDIR[port - 1]  )  &=  ~mask;
        *(  PREN[port - 1]  )  |=   mask;
        *(  POUT[port - 1]  )  |=   mask;
    }

    if(mode == inPullDown)
    {
        *(  PDIR[port - 1]  )  &=  ~mask;
        *(  PREN[port - 1]  )  |=   mask;
        *(  POUT[port - 1]  )  &=  ~mask;
    }
}

uint8_t pinRead(uint8_t port, uint8_t bit)
{
    return (    *PIN[port - 1] & BIT(bit) );
}

void pinWrite(uint8_t port, uint8_t bit, uint8_t value)
{
    if(value)
        *(  POUT[port - 1]  ) |=  BIT(bit);

    else
        *(  POUT[port - 1]  ) &= ~BIT(bit);

}

void pinInit()
{
    pinMode(REDLED, output);
    pinMode(GREENLED, output);
    pinMode(S2, inPullUp);
    pinMode(S1, inPullUp);
}

void wait(uint16_t time,timeUnit_t unit){

    if(unit == sec){
        TA0CTL = TASSEL__ACLK | MC__UP | TACLR;

        TA0CCR0 = ( time * (32768) ) -1;

        while (!(TA0CCTL0 & CCIFG));

        TA0CCTL0 &= ~CCIFG;
    }

    else if(unit == ms){
            TA0CTL = TASSEL__SMCLK | MC__UP | TACLR;
            TA0CCR0 = ( time * (32768)/1000 ) -1;

            while (!(TA0CCTL0 & CCIFG));

            TA0CCTL0 &= ~CCIFG;
        }

    else{
            TA0CTL = TASSEL__SMCLK | MC__UP | TACLR;
            TA0CCR0 = ( time * (32768)/1000000 ) - 1;

            while (!(TA0CCTL0 & CCIFG));

            TA0CCTL0 &= ~CCIFG;
        }
}

// I2C ---------------------------------------------------
void release(uint8_t port, uint8_t bit)
{
    pinMode(port, bit, inPullUp);
}

void pullow(uint8_t port, uint8_t bit)
{
    pinWrite(port, bit, 0);
    pinMode(port, bit, output);
}

void writeBit(uint8_t value)
{
    if(value)
        release(SDA);
    else
        pullow(SDA);

    pullow(SCL);
    wait(10,ms);

    release(SCL);
    wait(20,ms);

    pullow(SCL);
    wait(10,ms);
}

uint8_t readBit()
{
    release(SDA);
    pullow(SCL);
    wait(10,ms);

    release(SCL);
    wait(10,ms);

    uint8_t bit = pinRead(SDA);
    wait(10,ms);

    pullow(SCL);
    wait(10,ms);

    return bit;
}

uint8_t writeByte(uint8_t value)
{
    uint8_t mask = 0x80;

    while(mask)
    {
        writeBit(value & mask);
        mask = mask >> 1;
    }

    return readBit();
}

uint8_t readByte(uint8_t nack)
{
    uint8_t mask    = 0x80;
    uint8_t value   = 0x00;

    while(mask)
    {
        if(readBit())
            value |= mask;

        mask >> 1;
    }

    writeBit(nack);
    return value;
}

void start()
{
    release(SDA);
    release(SCL);

    wait(10,ms);

    pullow(SDA);
    wait(10,ms);

    pullow(SCL);
    wait(20,ms);
}

void stop()
{
    pullow(SDA);
    wait(10,ms);

    release(SCL);
    wait(10,ms);

    release(SDA);
    wait(20,ms);
}

uint8_t i2cWrite(uint8_t addr, uint8_t * data, uint8_t size)
{
    start();
    writeByte(addr << 1 | 0);

    while(size--)
    {
        writeByte(*data++);
    }

    stop();

    //return nack;
}

uint8_t i2cRead(uint8_t addr, uint8_t * data, uint8_t size)
{
    start();

    uint8_t nack = writeByte(addr << 1 | 1);

    if(nack)
        return nack;

    while(size--)
        {
            *data++ = readByte(!size);
        }

    stop();
}
