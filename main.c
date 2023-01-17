// Felipe Breno Melo de Azevedo
// 19/0106263
#include <msp430.h>
#include <stdint.h>
#include <math.h>
#include "./biblio/biblio.h"

void wait(uint16_t time,timeUnit_t unit);
void leds(uint16_t dist);
void ta2_prog(uint16_t freq);

volatile uint16_t   freq, dist, t1, t2;
volatile uint16_t   subida;

volatile float echo;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;       // stop watchdog timer

    pinInit();          // Configura os leds como saida

    P1OUT &= ~BIT5;     // Configura o pino 1.5 como saida
    P1DIR |= BIT5;      // Pino do trigger (TA0.4)

    P2SEL |= BIT0;      // Configura o pino 2.0 como entrada
    P2DIR &= ~BIT0;     // Pino do echo (TA1.1)

    pinMode(2, 5, output);      // Configura o pino 2.5 como saida
    P2SEL |= BIT5;              // Pino do buzzer (TA2.2)

    //Configurando o Timer A1 como SMCLOCK no modo Continuo
    TA1CTL = MC__CONTINUOUS | TASSEL__SMCLK;

    TA1CCTL1 = CM_3 | CAP | CCIE;   // Configurando como modo de captura no flanco de subida e descida

    t1      =   0;
    t2      =   0;
    freq    =   0;
    echo    =   0;
    dist    =   0;
    subida  =   1;

    __enable_interrupt();

    while(1){

        pinWrite(1, 5, 1);      // Ativa o trigger por 12 ms

        wait(12,ms);

        pinWrite(1, 5, 0);      // Desativa o trigger após 12ms

        wait(10,us);

        leds(dist);             // Acende o led conforme a distancia

        if(dist > 50)           // Condição apenas para garantir que não terá frequencia nenhuma acima de 50cm
            freq = 0;
        else
            freq = ( 5000 - dist * 100 );   // Calculo da frequencia conforme a equação da reta mostrada no PDF

        wait(15,us);

        ta2_prog(freq);     // Propaga a frequencia pro buzzer
    }

}

void ta2_prog(uint16_t  freq)
{
    TA2CTL = TASSEL__SMCLK | MC__UP | TACLR;

    TA2CCR0 = (1048576 / freq) - 1;     // Formula para transformar a frequencia propagada em batidas do SMCLOCK
                                        // N = (1 / freq) * 2^20
                                        // CCRN = N - 1

    if(freq <= 0 || TA2CCR0 >= 0xFFFF)
            TA2CCR0 = 0;

    TA2CCR2 = TA2CCR0 >> 1;

    TA2CCTL2 = OUTMOD_7;
}

void leds(uint16_t dist)
{
    if(dist > 50)       // Apaga os dois leds se a distancia maior que 50 cm
    {
        pinWrite(REDLED,0);
        pinWrite(GREENLED,0);
    }

    else if(dist >= 30 && dist <= 50)   // Acende o led verde entre 30 e 50 cm
    {
        pinWrite(REDLED,0);
        pinWrite(GREENLED,1);
    }

    else if(dist >= 10 && dist < 30)    // Acende o led vermelho entre 10 e 30 cm
    {
        pinWrite(REDLED,1);
        pinWrite(GREENLED,0);
    }

    else if(dist < 10)                  // Acende os dois leds se a distancia menor que 10 cm
    {
        pinWrite(REDLED,1);
        pinWrite(GREENLED,1);
    }
}

// Interrupção da captura do timer A1
#pragma vector = TIMER1_A1_VECTOR
__interrupt void ta1_cap(void){

 if (subida)    // Verifica o flanco
 {
     t1=TA1CCR1;    // Captura o primeiro valor

     TA1IV;

     subida = 0;    // Flanco de descida
 }

 else
 {
     t2=TA1CCR1;    // Captura o segundo valor

     echo = t2-t1;      // Subtrai (final - inicial) para achar o echo
     echo= echo/1048.0; // Transforma em ms

     dist = (echo * 34)/2;  //Multiplica pela velocidade do som e divide pela metade para encontrar a distancia em cm

     TA1IV;

     subida =  1;   // Flanco de subida
 }
}

