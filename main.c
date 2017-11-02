// #define F_CPU 480000 // already defined in makefile

#include <avr/interrupt.h>
#include <avr/sleep.h>
/*#include "TinyTouchLib.h"*/

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

register uint8_t occount asm("r2");
register uint8_t s asm("r3"); // lcg var
register uint8_t a asm("r4"); // lcg var

static uint8_t rand(void) {
    s ^= s << 3;
    s ^= s >> 5;
    s ^= a++ >> 2;
    return s;
}

ISR(TIM0_OVF_vect) {
    ++occount;
}

int main(void) {
    uint8_t power = 0xff; // inital state of pwm leds = dark
    uint8_t cap = 40; // initial updimming speed = moderate
    /*uint8_t tmp = 0;*/
    int8_t direction = 1; // inital dimming direction = up
    int8_t hold = -1; // (weird math) skip two sequential /\ phases, invert to skip \/
    /*int8_t flicker = 0;*/
    occount = 0;

    set_sleep_mode(SLEEP_MODE_IDLE);  // only disable cpu and flash clk while sleeping
    PORTB = 0xff;  // enable pullup resistors for powersaving
    DDRB |= (1 << PB1);  // set pwm port as output
    TCCR0B |= (1 << CS01);  // timer prescaler == 8
    TCCR0A |= (1 << WGM01) | (1 << WGM00); // set pwm to fast_pwm mode
    TCCR0A |= (1 << COM0B1) | (1 << COM0B0); // set pwm to inverting mode
    TIMSK0 |= (1 << TOIE0); // enable timer interrupt

    /*tinytouch_init();*/
    
    /*while (tinytouch_sense() != tt_push) {*/
        /*++s;  // increment s until button is pressed()*/
    /*}*/
    s=17;

    ADCSRA &= ~(1 << ADEN);  //disable ADC
    
    sei();
    while(1) {
        if (occount >= cap) { // cap sets the delay for updating pwm value
            cli(); // we don't need interrupts while updating pwm
            if (power == 0xff || power <= 0x70) { // change fade direction after reaching max\min values
                direction = (int8_t)(direction * -1);
                hold = (int8_t)(hold * direction); // skip every two dimming phases
                cap = (uint8_t)(MAX(rand() % 200, 40));
            }
            /*flicker = (rand() % 35) * ((rand() & 0x1) ? -1 : 1);*/
            power += direction;
            /*cap = MIN(cap+flicker, 128);*/
            /*tmp = power + flicker;*/
            /*if ((flicker > 0 && tmp > power) || (flicker < 0 && tmp < power))*/
                /*power = tmp;*/
            /*if (hold == 1)*/
                OCR0B = power; // do not update pwm every two continious phases
            occount = 0;
            sei();
        }
        sleep_mode();
    }
}
